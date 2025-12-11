/**
 * ESP32 Micro Hub - ESP-IDF Main
 * 
 * ESP-NOW 리모컨 수신기 with micro-ROS, LCD, CAN
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

// micro-ROS includes
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rmw_microros/rmw_microros.h>
#include <std_msgs/msg/int32.h>
#include <uxr/client/transport.h>

static const char *TAG = "ESP32_MICRO_HUB";

// 설정 상수
#define WIFI_CHANNEL 1
#define ESP_NOW_QUEUE_SIZE 10

// ROS Serial 설정
#define ROS_SERIAL_PORT UART_NUM_1
#define ROS_SERIAL_TXD 17
#define ROS_SERIAL_RXD 16
#define ROS_SERIAL_BAUD_RATE 921600

// ESP-NOW 데이터 구조
typedef struct {
    uint8_t mac_addr[6];
    uint8_t data[250];
    int data_len;
} espnow_event_t;

static QueueHandle_t espnow_queue;
static bool ros_enabled = true;

// micro-ROS 객체
static rcl_allocator_t allocator;
static rclc_support_t support;
static rcl_node_t node;
static rcl_publisher_t publisher;
static std_msgs__msg__Int32 msg;

// CUSTOM_TRANSPORT 함수들
bool uart_transport_open(struct uxrCustomTransport* transport)
{
    ESP_LOGI(TAG, "UART transport opened");
    return true;
}

bool uart_transport_close(struct uxrCustomTransport* transport)
{
    ESP_LOGI(TAG, "UART transport closed");
    return true;
}

size_t uart_transport_write(struct uxrCustomTransport* transport, const uint8_t* buf, size_t len, uint8_t* err)
{
    int written = uart_write_bytes(ROS_SERIAL_PORT, (const char*)buf, len);
    return written > 0 ? written : 0;
}

size_t uart_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* err)
{
    int read = uart_read_bytes(ROS_SERIAL_PORT, buf, len, pdMS_TO_TICKS(timeout));
    return read > 0 ? read : 0;
}

/**
 * ESP-NOW 수신 콜백
 */
static void espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    espnow_event_t evt;
    
    if (recv_info->src_addr == NULL || data == NULL || len <= 0) {
        ESP_LOGE(TAG, "ESP-NOW receive cb arg error");
        return;
    }
    
    memcpy(evt.mac_addr, recv_info->src_addr, 6);
    evt.data_len = len;
    memcpy(evt.data, data, len);
    
    if (xQueueSend(espnow_queue, &evt, portMAX_DELAY) != pdTRUE) {
        ESP_LOGW(TAG, "ESP-NOW queue full");
    }
}

/**
 * ESP-NOW 송신 콜백
 * ESP-IDF 5.x: wifi_tx_info_t 구조체 사용
 */
static void espnow_send_cb(const wifi_tx_info_t *tx_info, esp_now_send_status_t status)
{
    if (status == ESP_NOW_SEND_SUCCESS) {
        ESP_LOGI(TAG, "ESP-NOW send success");
    } else {
        ESP_LOGW(TAG, "ESP-NOW send fail, status: %d", status);
    }
}

/**
 * ESP-NOW 초기화
 */
static esp_err_t espnow_init(void)
{
    espnow_queue = xQueueCreate(ESP_NOW_QUEUE_SIZE, sizeof(espnow_event_t));
    if (espnow_queue == NULL) {
        ESP_LOGE(TAG, "Create ESP-NOW queue fail");
        return ESP_FAIL;
    }
    
    // ESP-NOW 초기화
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(espnow_send_cb));
    ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_recv_cb));
    
    ESP_LOGI(TAG, "ESP-NOW initialized");
    return ESP_OK;
}

/**
 * WiFi 초기화 (ESP-NOW용)
 */
static void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE));
    
    ESP_LOGI(TAG, "WiFi initialized for ESP-NOW");
}

/**
 * ROS Serial 초기화
 */
static void ros_serial_init(void)
{
    const uart_config_t uart_config = {
        .baud_rate = ROS_SERIAL_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    ESP_ERROR_CHECK(uart_driver_install(ROS_SERIAL_PORT, 2048, 2048, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(ROS_SERIAL_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ROS_SERIAL_PORT, ROS_SERIAL_TXD, ROS_SERIAL_RXD, 
                                  UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    
    ESP_LOGI(TAG, "ROS Serial initialized on UART%d @ %d bps", ROS_SERIAL_PORT, ROS_SERIAL_BAUD_RATE);
}

/**
 * micro-ROS 초기화
 */
static esp_err_t microros_init(void)
{
    // CUSTOM_TRANSPORT 설정
    rmw_uros_set_custom_transport(
        true,  // framing enable
        NULL,  // args
        uart_transport_open,
        uart_transport_close,
        uart_transport_write,
        uart_transport_read
    );
    
    // allocator 생성
    allocator = rcl_get_default_allocator();
    
    // support 초기화
    rclc_support_init(&support, 0, NULL, &allocator);
    
    // node 생성
    rclc_node_init_default(&node, "esp32_micro_hub", "", &support);
    
    // publisher 생성
    rclc_publisher_init_default(
        &publisher,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "espnow_button"
    );
    
    msg.data = 0;
    
    ESP_LOGI(TAG, "micro-ROS initialized");
    return ESP_OK;
}

/**
 * ESP-NOW 처리 태스크
 */
static void espnow_task(void *pvParameter)
{
    espnow_event_t evt;
    
    ESP_LOGI(TAG, "ESP-NOW task started");
    
    while (1) {
        if (xQueueReceive(espnow_queue, &evt, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "Received ESP-NOW data: len=%d", evt.data_len);
            
            // 데이터 처리
            if (evt.data_len >= 2) {
                uint8_t button_id = evt.data[0];
                uint8_t button_state = evt.data[1];
                
                ESP_LOGI(TAG, "Button: ID=%d, State=%d", button_id, button_state);
                
                // ROS 활성화 시 메시지 발행
                if (ros_enabled) {
                    msg.data = (button_id << 8) | button_state;
                    rcl_ret_t ret = rcl_publish(&publisher, &msg, NULL);
                    if (ret == RCL_RET_OK) {
                        ESP_LOGI(TAG, "Published to ROS: %d", msg.data);
                    } else {
                        ESP_LOGW(TAG, "Failed to publish: %d", ret);
                    }
                } else {
                    // 직접 제어
                    ESP_LOGI(TAG, "Direct control: button_id=%d, state=%d", 
                            button_id, button_state);
                }
            }
        }
    }
}

/**
 * ROS 통신 태스크
 */
static void ros_task(void *pvParameter)
{
    ESP_LOGI(TAG, "ROS task started");
    
    // Agent 연결 대기
    ESP_LOGI(TAG, "Waiting for micro-ROS agent...");
    while (rmw_uros_ping_agent(1000, 1) != RMW_RET_OK) {
        ESP_LOGW(TAG, "Agent not available, retrying...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    ESP_LOGI(TAG, "micro-ROS agent connected!");
    
    while (1) {
        // Executor 또는 spin 대신 간단히 딜레이
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * LCD UI 태스크
 */
static void lcd_ui_task(void *pvParameter)
{
    ESP_LOGI(TAG, "LCD UI task started");
    
    while (1) {
        // TODO: LCD 업데이트
        // TODO: 버튼 처리
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * 메인 애플리케이션
 */
void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Micro Hub starting...");
    
    // NVS 초기화
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // WiFi 초기화 (ESP-NOW용)
    wifi_init();
    
    // ESP-NOW 초기화
    ESP_ERROR_CHECK(espnow_init());
    
    // ROS Serial 초기화
    ros_serial_init();
    
    // micro-ROS 초기화
    ESP_ERROR_CHECK(microros_init());
    
    // TODO: LCD 초기화
    // TODO: CAN 초기화
    
    // 태스크 생성
    xTaskCreate(espnow_task, "espnow_task", 4096, NULL, 5, NULL);
    xTaskCreate(ros_task, "ros_task", 10240, NULL, 5, NULL);
    xTaskCreate(lcd_ui_task, "lcd_ui_task", 4096, NULL, 3, NULL);
    
    ESP_LOGI(TAG, "ESP32 Micro Hub initialized");
}
