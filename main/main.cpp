/**
 * ESP32 Micro Hub - Main Application
 * C++ Class-Based Architecture with micro-ROS
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_mac.h"
#include "nvs_flash.h"

// 설정 파일
#include "config.h"

// C++ 컴포넌트
#include "CanControl.h"
#include "EspNowControl.h"
#include "RosBridge.h"
#include "ButtonControl.h"
#include "LedControl.h"
#include "LcdControl.h"

static const char *TAG = "MAIN";

// 전역 객체 (config.h의 핀 정의 사용)
static CanControl can_control(CAN_TX_PIN, CAN_RX_PIN);
static EspNowControl espnow_control;
static RosBridge ros_bridge(ROS_UART_NUM, ROS_TX_PIN, ROS_RX_PIN, ROS_UART_BAUD_RATE);
static ButtonControl button_control;
static LedControl led_control(LED1_PIN);
static LcdControl lcd_control;

// 태스크 핸들
static TaskHandle_t ui_task_handle = nullptr;
static TaskHandle_t ros_task_handle = nullptr;
static bool ui_task_running = false;
static bool ros_task_running = false;

// ESP-NOW 수신 콜백
void onEspNowRecv(const uint8_t* mac, const uint8_t* data, int len) {
    ESP_LOGI(TAG, "ESP-NOW received %d bytes from %02X:%02X:%02X:%02X:%02X:%02X", 
             len, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    if (len >= 1) {
        uint8_t button_id = data[0];
        ESP_LOGI(TAG, "Button ID: %d", button_id);
        
        // ROS로 발행
        ros_bridge.publish(button_id);
        
        // CAN 명령 전송
        switch (button_id) {
            case 1: // 전진
                can_control.sendMotorCommand(50, 1);
                break;
            case 2: // 후진
                can_control.sendMotorCommand(50, 2);
                break;
            case 3: // 정지
                can_control.sendMotorCommand(0, 0);
                break;
            case 4: // 리프트 업
                can_control.sendLiftCommand(1);
                break;
            case 5: // 리프트 다운
                can_control.sendLiftCommand(2);
                break;
            case 6: // 캐스터 회전
                can_control.sendCasterCommand(1);
                break;
            default:
                ESP_LOGW(TAG, "Unknown button ID: %d", button_id);
                break;
        }
    }
}

// 버튼 이벤트 콜백
void onButtonEvent(ButtonControl::ButtonID id, ButtonControl::ButtonEvent event) {
    ESP_LOGI(TAG, "Button %d - Event: %d", id, event);
    
    if (event == ButtonControl::EVENT_PRESSED) {
        led_control.blink(100, 1);  // 버튼 누름 피드백
        
        // 버튼에 따른 동작
        switch (id) {
            case ButtonControl::BUTTON_SELECT:
                ESP_LOGI(TAG, "SELECT button pressed");
                break;
            case ButtonControl::BUTTON_UP:
                ESP_LOGI(TAG, "UP button pressed");
                break;
            case ButtonControl::BUTTON_DOWN:
                ESP_LOGI(TAG, "DOWN button pressed");
                break;
            case ButtonControl::BUTTON_LEFT_DIR:
                ESP_LOGI(TAG, "LEFT button pressed");
                break;
            case ButtonControl::BUTTON_RIGHT:
                ESP_LOGI(TAG, "RIGHT button pressed");
                break;
            case ButtonControl::BUTTON_POWER:
                ESP_LOGI(TAG, "POWER button pressed");
                break;
            case ButtonControl::BUTTON_EMERGENCY:
                ESP_LOGI(TAG, "EMERGENCY button pressed");
                can_control.sendMotorCommand(0, 0);
                break;
            case ButtonControl::BUTTON_RUN:
                ESP_LOGI(TAG, "RUN button pressed");
                break;
            default:
                break;
        }
    }
}

// CAN 상태 수신 콜백
void onCanStatus(const CanControl::VehicleStatusData& status) {
    ESP_LOGI(TAG, "CAN Status - VMain:%d DCDC:%d Curr:%d Cons:%d MotorT:%d FetT:%d SOC:%d Err:%d",
             status.volt_main, status.volt_dcdc, status.current_avg, status.consumption,
             status.motor_temp, status.fet_temp, status.soc, status.error_code);
    
    // LCD 업데이트 (volt_main을 배터리 전압으로, current_avg를 전류로 사용)
    lcd_control.updateVehicleData(0, status.soc, 
                                  status.motor_temp, status.current_avg, status.fet_temp);
}

// UI 태스크: Button(100ms) + LCD(1s)
static void ui_task(void* parameter) {
    uint32_t button_counter = 0;
    uint32_t lcd_counter = 0;
    const uint32_t BUTTON_PERIOD_MS = BUTTON_SCAN_INTERVAL_MS;  // config.h에서 정의
    const uint32_t LCD_PERIOD_MS = LCD_UPDATE_INTERVAL_MS;      // config.h에서 정의
    
    ESP_LOGI(TAG, "UI Task started");
    
    while (ui_task_running) {
        // Button 스캔 (100ms마다)
        if (button_counter >= BUTTON_PERIOD_MS) {
            button_control.scanButtons();
            button_control.updateToggleState();
            button_counter = 0;
        }
        
        // LCD 업데이트 (1초마다)
        if (lcd_counter >= LCD_PERIOD_MS) {
            lcd_control.renderUI();
            lcd_counter = 0;
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));  // 10ms 틱
        button_counter += 10;
        lcd_counter += 10;
    }
    
    vTaskDelete(NULL);
}

// micro-ROS 태스크: Pub/Sub 처리
static void microros_task(void* parameter) {
    ESP_LOGI(TAG, "micro-ROS Task started");
    
    while (ros_task_running) {
        // 연결 상태 체크 및 재연결
        if (!ros_bridge.isConnected()) {
            ESP_LOGW(TAG, "micro-ROS disconnected, trying to reconnect...");
            lcd_control.updateConnectionStatus(false);
            
            if (ros_bridge.waitForAgent(5000)) {
                ESP_LOGI(TAG, "Reconnected to micro-ROS agent");
                lcd_control.updateConnectionStatus(true);
            }
        }
        
        // TODO: Subscribe 콜백 처리
        
        vTaskDelay(pdMS_TO_TICKS(100));  // 100ms 주기
    }
    
    vTaskDelete(NULL);
}

// WiFi 초기화
static esp_err_t wifi_init(void) {
    esp_err_t ret = esp_netif_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_netif_init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = esp_event_loop_create_default();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_event_loop_create_default failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_mode failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = esp_wifi_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_start failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "WiFi initialized");
    return ESP_OK;
}

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "=== ESP32 Micro Hub Starting ===");
    
    // NVS 초기화
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS initialized");
    
    // WiFi 초기화
    ESP_ERROR_CHECK(wifi_init());
    
    // ESP-NOW 초기화
    ret = espnow_control.begin(1);
    if (ret == ESP_OK) {
        espnow_control.setRecvCallback(onEspNowRecv);
        espnow_control.startRxTask(4096, 5);
        ESP_LOGI(TAG, "ESP-NOW initialized");
    } else {
        ESP_LOGE(TAG, "ESP-NOW init failed: %s", esp_err_to_name(ret));
    }
    
    // CAN 초기화
    ret = can_control.begin(500000);
    if (ret == ESP_OK) {
        can_control.setStatusCallback(onCanStatus);
        can_control.startRxTask(4096, 4);
        ESP_LOGI(TAG, "CAN initialized (500 Kbps)");
    } else {
        ESP_LOGE(TAG, "CAN init failed: %s", esp_err_to_name(ret));
    }
    
    // micro-ROS 초기화
    ret = ros_bridge.begin("esp32_micro_hub");
    if (ret == ESP_OK) {
        ros_bridge.createPublisher("espnow_button");
        ESP_LOGI(TAG, "micro-ROS initialized");
    } else {
        ESP_LOGE(TAG, "micro-ROS init failed: %s", esp_err_to_name(ret));
    }
    
    // Button 초기화 (PCA9555 GPIO + ESP32 Direct GPIO)
    // 참고: PCA9555는 I2C로 별도 초기화 필요, 여기서는 ESP32 Direct GPIO만 사용
    gpio_num_t button_pins[] = {
        BTN_EMERGENCY_PIN,  // GPIO34
        BTN_RUN_PIN         // GPIO35
    };
    ret = button_control.begin(button_pins, 2);
    if (ret == ESP_OK) {
        button_control.setButtonCallback(onButtonEvent);
        ESP_LOGI(TAG, "Button control initialized (2 direct GPIO buttons)");
    } else {
        ESP_LOGE(TAG, "Button control init failed: %s", esp_err_to_name(ret));
    }
    
    // LED 초기화
    ret = led_control.begin();
    if (ret == ESP_OK) {
        led_control.showSuccess();  // 초기화 성공 표시
        ESP_LOGI(TAG, "LED control initialized");
    } else {
        ESP_LOGE(TAG, "LED init failed: %s", esp_err_to_name(ret));
    }
    
    // LCD 초기화 (UI 태스크에서 업데이트)
    ret = lcd_control.begin();
    if (ret == ESP_OK) {
        lcd_control.drawMainScreen();
        lcd_control.updateControlMode("READY");
        lcd_control.updateConnectionStatus(ros_bridge.isConnected());
        ESP_LOGI(TAG, "LCD control initialized (240x320)");
    } else {
        ESP_LOGE(TAG, "LCD init failed: %s", esp_err_to_name(ret));
    }
    
    // UI 태스크 시작 (config.h 설정 사용)
    ui_task_running = true;
    xTaskCreate(
        ui_task,
        "ui_task",
        LCD_UI_TASK_STACK_SIZE,
        NULL,
        LCD_UI_TASK_PRIORITY,
        &ui_task_handle
    );
    ESP_LOGI(TAG, "UI Task started (Button:%dms, LCD:%dms)", 
             BUTTON_SCAN_INTERVAL_MS, LCD_UPDATE_INTERVAL_MS);
    
    // micro-ROS 태스크 시작 (config.h 설정 사용)
    ros_task_running = true;
    xTaskCreate(
        microros_task,
        "microros_task",
        ROS_TASK_STACK_SIZE,
        NULL,
        ROS_TASK_PRIORITY,
        &ros_task_handle
    );
    ESP_LOGI(TAG, "micro-ROS Task started");
    
    ESP_LOGI(TAG, "=== System Ready ===");
    led_control.showSuccess();
    
    // 메인 루프 (모니터링)
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        ESP_LOGI(TAG, "System running - Free heap: %lu bytes", esp_get_free_heap_size());
    }
}
