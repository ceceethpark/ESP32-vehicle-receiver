#ifndef CAN_CONTROL_H
#define CAN_CONTROL_H

#include "driver/twai.h"
#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

// CAN 핀 정의
#define CAN_TX_GPIO GPIO_NUM_21
#define CAN_RX_GPIO GPIO_NUM_22

// CAN 메시지 ID
#define CAN_ID_MOTOR_CMD    0x100
#define CAN_ID_LIFT_CMD     0x101
#define CAN_ID_CAST_CMD     0x102
#define CAN_ID_VEHICLE_STATUS 0x200

// 초기화
esp_err_t can_control_init(void);
void can_control_deinit(void);

// 명령 전송
esp_err_t can_send_motor_command(uint8_t speed, uint8_t direction);
esp_err_t can_send_lift_command(uint8_t state);
esp_err_t can_send_caster_command(uint8_t state);

// 상태 수신 (태스크에서 처리)
void can_rx_task(void *arg);

#ifdef __cplusplus
}
#endif

#endif // CAN_CONTROL_H
