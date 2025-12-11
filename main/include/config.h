#ifndef CONFIG_H
#define CONFIG_H

#include "driver/gpio.h"
#include "driver/uart.h"

/**
 * ESP32 Micro Hub - Configuration Header
 * 모든 핀 정의 및 시스템 설정을 통합 관리
 */

// ==========================================
// GPIO Pin Definitions (Based on Actual Hardware Schematic)
// ==========================================

// UART0 (Debug/Programming) - DO NOT USE IN CODE
// TXD0: GPIO 1 (USB Serial)
// RXD0: GPIO 3 (USB Serial)

// CAN Bus (TWAI) Pins - 회로도 확인됨
#define CAN_TX_PIN              GPIO_NUM_16     // IO27 from schematic
#define CAN_RX_PIN              GPIO_NUM_17     // IO26 from schematic

// micro-ROS UART Pins (Serial1) - UART 헤더 사용
#define ROS_UART_NUM            UART_NUM_1
#define ROS_TX_PIN              GPIO_NUM_23              // GPIO17 (TXD1) - UART 헤더
#define ROS_RX_PIN              GPIO_NUM_22              // GPIO16 (RXD1) - UART 헤더
#define ROS_UART_BAUD_RATE      921600

// I2C Pins (IMU 센서용 - 회로도 확인됨)
#define I2C_SDA_PIN             GPIO_NUM_13     // IO13 (pSDA)
#define I2C_SCL_PIN             GPIO_NUM_2      // IO2 (pSCL)
#define IMU_INT_PIN             GPIO_NUM_15     // IO15 (IMU_INT)

// ST7789 LCD SPI Pins - 회로도 확인됨
#define LCD_HOST_ID             SPI2_HOST
#define LCD_MOSI_PIN            GPIO_NUM_26     // IO26 (MOSI)
#define LCD_MISO_PIN            GPIO_NUM_12     // IO12 (MISO) - 회로도에 있음
#define LCD_SCLK_PIN            GPIO_NUM_19     // IO19 (SCL)
#define LCD_CS_PIN              GPIO_NUM_25      // IO25 (CS)
#define LCD_DC_PIN              GPIO_NUM_35      // Data/Command (추정)
#define LCD_RST_PIN             GPIO_NUM_0     // Reset (추정)

// LED Pins - 회로도 LED_CON 확인됨
#define LED1_PIN                GPIO_NUM_27     // LED1
#define LED2_PIN                GPIO_NUM_14     // LED2
//#define STATUS_LED_PIN          GPIO_NUM_12     // Built-in LED - IO2는 I2C_SCL

// PCA9555 I2C GPIO Expander - 회로도 확인됨
// I2C 주소: 0x20 (기본) 또는 회로도 확인 필요
#define PCA9555_I2C_ADDR        0x20            // PCA9555 I2C 주소

// PCA9555 Input Pins (IOI_0 ~ IOI_9) - 10개 입력
#define PCA_IOI_0               0               // IOI_0: SELECT
#define PCA_IOI_1               1               // IOI_1: DOWN
#define PCA_IOI_2               2               // IOI_2: RIGHT
#define PCA_IOI_3               3               // IOI_3: LEFT
#define PCA_IOI_4               4               // IOI_4: UP
#define PCA_IOI_5               5               // IOI_5: pI_PWR_N (PWR-)
#define PCA_IOI_6               6               // IOI_6
#define PCA_IOI_7               7               // IOI_7
#define PCA_IOI_8               8               // IOI_8
#define PCA_IOI_9               9               // IOI_9

// PCA9555 Output Pins (IOO_0 ~ IOO_5) - 6개 출력
#define PCA_IOO_0               10              // IOO_0
#define PCA_IOO_1               11              // IOO_1
#define PCA_IOO_2               12              // IOO_2
#define PCA_IOO_3               13              // IOO_3
#define PCA_IOO_4               14              // IOO_4
#define PCA_IOO_5               15              // IOO_5

// Button Mapping (PCA9555 기반)
#define BTN_SELECT              PCA_IOI_0       // SELECT 버튼
#define BTN_DOWN                PCA_IOI_1       // DOWN 버튼
#define BTN_RIGHT               PCA_IOI_2       // RIGHT 버튼
#define BTN_LEFT                PCA_IOI_3       // LEFT 버튼
#define BTN_UP                  PCA_IOI_4       // UP 버튼
#define BTN_POWER               PCA_IOI_5       // POWER 버튼 (pI_PWR_N)

// ESP32 Direct GPIO Buttons (백업/추가 버튼용)
#define BTN_EMERGENCY_PIN       GPIO_NUM_34     // Input only (비상정지)
#define BTN_RUN_PIN             GPIO_NUM_35     // Input only (실행/정지)

// ==========================================
// ROS Configuration
// ==========================================

// ROS Enable/Disable
#define ROS_ENABLED_DEFAULT         true        // true: ROS publish mode, false: Direct control mode

// ROS Node Configuration
#define ROS_NODE_NAME               "esp32_micro_hub"
#define ROS_NAMESPACE               ""

// ROS Topics
#define ROS_TOPIC_REMOTE_CONTROL    "/remote_control"
#define ROS_TOPIC_VEHICLE_STATUS    "/vehicle_status"
#define ROS_TOPIC_MOTOR_STATE       "/motor_state"
#define ROS_TOPIC_BATTERY_STATE     "/battery_state"

// ROS Task Configuration
#define ROS_TASK_STACK_SIZE         10240       // 10KB stack for micro-ROS task
#define ROS_TASK_PRIORITY           5           // FreeRTOS task priority
#define ROS_RECONNECT_INTERVAL      5000        // Reconnect interval in ms

// ROS Transport Type
#define ROS_USE_SERIAL_TRANSPORT    true        // true: Serial, false: WiFi
#define ROS_SERIAL_INIT_DELAY       100         // Serial initialization delay in ms

// ROS Logging
#define ROS_DEBUG_ENABLED           true        // Enable ROS debug messages

// ROS WiFi Configuration (Alternative to Serial)
#define ROS_WIFI_AGENT_IP           "192.168.1.100"
#define ROS_WIFI_AGENT_PORT         8888
#define ROS_WIFI_SSID               "esp32_micro_hub"
#define ROS_WIFI_PASSWORD           "password"

// ==========================================
// CAN Bus Configuration
// ==========================================

#define CAN_BITRATE                 500000      // 500 Kbps
#define CAN_RX_TASK_STACK_SIZE      4096
#define CAN_RX_TASK_PRIORITY        4
#define CAN_TX_QUEUE_SIZE           10
#define CAN_RX_QUEUE_SIZE           10

// ==========================================
// ESP-NOW Configuration
// ==========================================

#define ESPNOW_WIFI_CHANNEL         1
#define ESPNOW_TASK_STACK_SIZE      4096
#define ESPNOW_TASK_PRIORITY        5
#define ESPNOW_QUEUE_SIZE           10
#define REMOTE_TIMEOUT_MS           3000        // Remote control timeout

// ==========================================
// Button Configuration
// ==========================================

#define BUTTON_DEBOUNCE_MS          50          // Debounce delay
#define BUTTON_LONG_PRESS_MS        1000        // Long press threshold
#define BUTTON_SCAN_INTERVAL_MS     100         // Button scan interval in UI task

// ==========================================
// LCD UI Configuration
// ==========================================

#define CFG_LCD_WIDTH               240
#define CFG_LCD_HEIGHT              320
#define CFG_LCD_SPI_CLOCK_HZ        40000000    // 40 MHz
#define LCD_UI_TASK_STACK_SIZE      8192        // 8KB stack
#define LCD_UI_TASK_PRIORITY        3
#define LCD_UPDATE_INTERVAL_MS      1000        // LCD refresh interval (1 second)

// ==========================================
// LED Configuration
// ==========================================

#define LED_BLINK_SLOW_MS           1000
#define LED_BLINK_FAST_MS           200
#define LED_PULSE_PERIOD_MS         2000

// ==========================================
// System Configuration
// ==========================================

#define SERIAL_DEBUG_BAUD_RATE      115200
#define MAIN_LOOP_DELAY_MS          10
#define WATCHDOG_TIMEOUT_SEC        30

// FreeRTOS Task Priorities (높을수록 우선순위 높음)
// Priority 5: micro-ROS, ESP-NOW (실시간 통신)
// Priority 4: CAN RX (차량 상태 수신)
// Priority 3: UI Task (버튼 스캔 + LCD 업데이트)
// Priority 1: Idle tasks

// ==========================================
// Hardware Information
// ==========================================

/**
 * ESP32-WROOM-32 Pin Map
 * 
 * ┌─────────────────────────────────────┐
 * │         ESP32 Pin Assignments       │
 * ├─────────────────────────────────────┤
 * │ Function    │ GPIO  │ Notes         │
 * ├─────────────┼───────┼───────────────┤
 * │ LED         │ 2     │ Built-in LED  │
 * │ LCD DC      │ 4     │ Data/Command  │
 * │ LCD CS      │ 5     │ Chip Select   │
 * │ LCD BL      │ 13    │ Backlight     │
 * │ BTN RIGHT   │ 14    │ Pull-up       │
 * │ LCD RST     │ 15    │ Reset         │
 * │ ROS RX      │ 16    │ UART1 RXD     │
 * │ ROS TX      │ 17    │ UART1 TXD     │
 * │ LCD SCLK    │ 18    │ SPI Clock     │
 * │ LCD MOSI    │ 23    │ SPI MOSI      │
 * │ CAN TX      │ 21    │ TWAI TX       │
 * │ CAN RX      │ 22    │ TWAI RX       │
 * │ BTN FORWARD │ 25    │ Pull-up       │
 * │ BTN BACK    │ 26    │ Pull-up       │
 * │ BTN LEFT    │ 27    │ Pull-up       │
 * │ BTN LIFT UP │ 12    │ Pull-up       │
 * │ BTN LIFT DN │ 13    │ Pull-up       │
 * │ BTN CASTER  │ 32    │ Pull-up       │
 * │ BTN MODE    │ 33    │ Pull-up       │
 * │ BTN EMERG   │ 34    │ Input only    │
 * │ BTN RUN     │ 35    │ Input only    │
 * └─────────────┴───────┴───────────────┘
 * 
 * Note: GPIO 34-39 are input only (no pull-up/pull-down)
 *       GPIO 6-11 are connected to flash (do not use)
 *       GPIO 0, 2 used for boot mode selection
 */

#endif // CONFIG_H
