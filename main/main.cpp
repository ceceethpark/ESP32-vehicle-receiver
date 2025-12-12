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
#include "TaskManager.h"
#include "WifiControl.h"

static const char *TAG = "MAIN";

// 전역 객체 (config.h의 핀 정의 사용)
static WifiControl wifi_control;
static CanControl can_control(CAN_TX_PIN, CAN_RX_PIN);
static EspNowControl espnow_control;
static RosBridge ros_bridge(ROS_UART_NUM, ROS_TX_PIN, ROS_RX_PIN, ROS_UART_BAUD_RATE);
static ButtonControl button_control;
static LedControl led_control(LED1_PIN);
static LcdControl lcd_control;
static TaskManager task_manager;

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
    
    // ======== Phase 1: 기본 인프라 초기화 ========
    // WiFi 초기화 (ESP-NOW 필수)
    ESP_ERROR_CHECK(wifi_control.initialize());
    
    // LED 초기화 (상태 표시용)
    ESP_ERROR_CHECK(led_control.initialize());
    
    // LCD 초기화 (초기 상태)
    ret = lcd_control.initialize("BOOTING", false);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "LCD init failed: %s (continuing)", esp_err_to_name(ret));
    }
    
    // ======== Phase 2: 통신 컴포넌트 초기화 ========
    // micro-ROS 초기화
    ret = ros_bridge.initialize("esp32_micro_hub", "espnow_button");
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "micro-ROS init failed: %s (continuing)", esp_err_to_name(ret));
    }
    
    // ButtonControl I2C 초기화 (PCA9555 내장)
    uint8_t pca_buttons[] = {
        BTN_SELECT,  // PCA9555 포트 0
        BTN_DOWN,    // PCA9555 포트 1
        BTN_RIGHT,   // PCA9555 포트 2
        BTN_LEFT,    // PCA9555 포트 3
        BTN_UP,      // PCA9555 포트 4
        BTN_POWER    // PCA9555 포트 5
    };
    
    ret = button_control.initializeI2C(
        I2C_NUM_0, 
        (gpio_num_t)I2C_SDA_PIN, 
        (gpio_num_t)I2C_SCL_PIN,
        PCA9555_I2C_ADDR,
        pca_buttons, 
        6,
        // ButtonCallback - 로컬 버튼 이벤트
        [](ButtonControl::ButtonID id, ButtonControl::ButtonEvent event) {
            if (event == ButtonControl::EVENT_PRESSED) {
                ESP_LOGI(TAG, "Local button %d pressed", id);
                
                // ROS 발행
                ros_bridge.publish(id + 100);  // 100번대: 로컬 버튼
                
                // CAN 명령 (리모컨과 동일한 매핑)
                switch (id) {
                    case ButtonControl::BUTTON_UP:  // 전진
                        can_control.sendMotorCommand(50, 1);
                        break;
                    case ButtonControl::BUTTON_DOWN:  // 후진
                        can_control.sendMotorCommand(50, 2);
                        break;
                    case ButtonControl::BUTTON_SELECT:  // 정지
                        can_control.sendMotorCommand(0, 0);
                        break;
                    case ButtonControl::BUTTON_LEFT_DIR:  // 리프트 업
                        can_control.sendLiftCommand(1);
                        break;
                    case ButtonControl::BUTTON_RIGHT:  // 리프트 다운
                        can_control.sendLiftCommand(2);
                        break;
                    case ButtonControl::BUTTON_POWER:  // 전원/캐스터
                        can_control.sendCasterCommand(1);
                        break;
                    default:
                        break;
                }
            }
        },
        nullptr  // RemoteCommandCallback (ESP-NOW가 처리)
    );
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "ButtonControl I2C init failed: %s (continuing)", esp_err_to_name(ret));
    } else {
        // 버튼 스캔 태스크 시작
        ret = button_control.startScanTask(2048, 4);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Button scan task start failed: %s", esp_err_to_name(ret));
        }
    }
    
    // CAN 초기화 (ROS/LCD 콜백 설정)
    ret = can_control.initialize(500000, 
        // StatusCallback - ROS로 발행
        [](const CanControl::VehicleStatusData& status) {
            ros_bridge.publish(status.soc);
        },
        // LcdUpdateCallback - LCD 업데이트
        [](int16_t volt_main, uint8_t soc, int16_t motor_temp, int16_t current_avg, int16_t fet_temp) {
            lcd_control.updateVehicleData(volt_main, soc, motor_temp, current_avg, fet_temp);
        },
        4096, 4);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "CAN init failed: %s (continuing)", esp_err_to_name(ret));
    }
    
    // ======== Phase 3: ESP-NOW 초기화 (의존성 주입) ========
    // ESP-NOW 초기화
    ret = espnow_control.begin(1);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "ESP-NOW init failed: %s (continuing)", esp_err_to_name(ret));
    }
    
    // 의존성 주입 (초기화 완료 후)
    espnow_control.setRosBridge(&ros_bridge);
    espnow_control.setCanControl(&can_control);
    
    // ESP-NOW RX 태스크 시작
    ret = espnow_control.startRxTask(4096, 5);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "ESP-NOW RX task start failed: %s (continuing)", esp_err_to_name(ret));
    }
    
    // LCD 상태 업데이트
    lcd_control.initialize("READY", ros_bridge.isConnected());
    
    // UI 태스크 시작 (TaskManager에서 관리)
    ret = task_manager.createUiTask(&lcd_control, LCD_UPDATE_INTERVAL_MS, 
                                     LCD_UI_TASK_STACK_SIZE, LCD_UI_TASK_PRIORITY);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create UI task");
    } else {
        ESP_LOGI(TAG, "UI Task started (LCD:%dms)", LCD_UPDATE_INTERVAL_MS);
    }
    
    // micro-ROS 태스크 시작 (TaskManager에서 관리)
    ret = task_manager.createRosTask(&ros_bridge, &lcd_control, 
                                     ROS_TASK_STACK_SIZE, ROS_TASK_PRIORITY);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create micro-ROS task");
    } else {
        ESP_LOGI(TAG, "micro-ROS Task started");
    }
    
    ESP_LOGI(TAG, "=== System Ready ===");
    led_control.showSuccess();
    
    // 메인 루프 (모니터링)
    uint32_t loop_count = 0;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        ESP_LOGI(TAG, "System running - Free heap: %lu bytes", esp_get_free_heap_size());
        
        // 10번마다 Task 통계 출력 (50초마다)
        if (++loop_count >= 10) {
            task_manager.printTaskStats();
            loop_count = 0;
        }
    }
}
