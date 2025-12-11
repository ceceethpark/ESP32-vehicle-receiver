#include "can_control.h"
#include <string.h>

static const char *TAG = "CAN_CONTROL";
static bool is_initialized = false;

/**
 * CAN 초기화
 */
esp_err_t can_control_init(void)
{
    if (is_initialized) {
        ESP_LOGW(TAG, "CAN already initialized");
        return ESP_OK;
    }

    // TWAI 일반 설정
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_GPIO, CAN_RX_GPIO, TWAI_MODE_NORMAL);
    
    // TWAI 타이밍 설정 (500 Kbps)
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    
    // TWAI 필터 설정 (모든 메시지 수신)
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // TWAI 드라이버 설치
    esp_err_t ret = twai_driver_install(&g_config, &t_config, &f_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install TWAI driver: %s", esp_err_to_name(ret));
        return ret;
    }

    // TWAI 시작
    ret = twai_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start TWAI: %s", esp_err_to_name(ret));
        twai_driver_uninstall();
        return ret;
    }

    is_initialized = true;
    ESP_LOGI(TAG, "CAN initialized (TX: GPIO%d, RX: GPIO%d, 500 Kbps)", CAN_TX_GPIO, CAN_RX_GPIO);
    return ESP_OK;
}

/**
 * CAN 종료
 */
void can_control_deinit(void)
{
    if (!is_initialized) {
        return;
    }

    twai_stop();
    twai_driver_uninstall();
    is_initialized = false;
    ESP_LOGI(TAG, "CAN deinitialized");
}

/**
 * 모터 명령 전송
 */
esp_err_t can_send_motor_command(uint8_t speed, uint8_t direction)
{
    if (!is_initialized) {
        ESP_LOGE(TAG, "CAN not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    twai_message_t message = {
        .identifier = CAN_ID_MOTOR_CMD,
        .data_length_code = 2,
        .data = {speed, direction, 0, 0, 0, 0, 0, 0}
    };

    esp_err_t ret = twai_transmit(&message, pdMS_TO_TICKS(100));
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Motor command sent: speed=%d, dir=%d", speed, direction);
    } else {
        ESP_LOGW(TAG, "Failed to send motor command: %s", esp_err_to_name(ret));
    }

    return ret;
}

/**
 * 리프트 명령 전송
 */
esp_err_t can_send_lift_command(uint8_t state)
{
    if (!is_initialized) {
        ESP_LOGE(TAG, "CAN not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    twai_message_t message = {
        .identifier = CAN_ID_LIFT_CMD,
        .data_length_code = 1,
        .data = {state, 0, 0, 0, 0, 0, 0, 0}
    };

    esp_err_t ret = twai_transmit(&message, pdMS_TO_TICKS(100));
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Lift command sent: state=%d", state);
    } else {
        ESP_LOGW(TAG, "Failed to send lift command: %s", esp_err_to_name(ret));
    }

    return ret;
}

/**
 * 캐스터 명령 전송
 */
esp_err_t can_send_caster_command(uint8_t state)
{
    if (!is_initialized) {
        ESP_LOGE(TAG, "CAN not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    twai_message_t message = {
        .identifier = CAN_ID_CAST_CMD,
        .data_length_code = 1,
        .data = {state, 0, 0, 0, 0, 0, 0, 0}
    };

    esp_err_t ret = twai_transmit(&message, pdMS_TO_TICKS(100));
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Caster command sent: state=%d", state);
    } else {
        ESP_LOGW(TAG, "Failed to send caster command: %s", esp_err_to_name(ret));
    }

    return ret;
}

/**
 * CAN 수신 태스크
 */
void can_rx_task(void *arg)
{
    twai_message_t message;
    
    ESP_LOGI(TAG, "CAN RX task started");

    while (1) {
        // CAN 메시지 수신 (블로킹)
        esp_err_t ret = twai_receive(&message, portMAX_DELAY);
        
        if (ret == ESP_OK) {
            // 차량 상태 메시지 처리
            if (message.identifier == CAN_ID_VEHICLE_STATUS && message.data_length_code >= 6) {
                uint8_t speed = message.data[0];
                uint8_t direction = message.data[1];
                uint8_t battery = message.data[2];
                int16_t motor_temp = (int16_t)((message.data[3] << 8) | message.data[4]);
                uint16_t motor_current = (uint16_t)((message.data[5] << 8) | message.data[6]);
                
                ESP_LOGI(TAG, "Vehicle status: speed=%d, dir=%d, batt=%d%%, temp=%d°C, curr=%dmA",
                        speed, direction, battery, motor_temp, motor_current);
                
                // TODO: micro-ROS로 발행 또는 LCD 업데이트
            } else {
                ESP_LOGD(TAG, "Received CAN ID: 0x%03X, DLC: %d", message.identifier, message.data_length_code);
            }
        } else if (ret == ESP_ERR_TIMEOUT) {
            // 타임아웃 (정상)
        } else {
            ESP_LOGW(TAG, "CAN receive error: %s", esp_err_to_name(ret));
        }
    }
}
