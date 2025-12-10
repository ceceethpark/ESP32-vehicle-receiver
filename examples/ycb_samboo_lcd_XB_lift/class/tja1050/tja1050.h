/*
 * tja1050.h
 *
 *  Created on: Jul 15, 2024
 *      Author: thpark
 */

#ifndef TJA1050_TJA1050_H_
#define TJA1050_TJA1050_H_

#define xCAN20B

#define CAN_TX_GET_CONFIG  0x0700//0700~070F
#define CAN_TX_PUT_CMD     0x0701
#define CAN_TX_SAVE_CMD    0x0708

#define CAN_RX_DATA_ID        0x05B0
#define CAN_RX_RESPONSE_ID    0x05B8

//typedef struct tag_cctrlBit
//{
//	uint32_t b00: 1;
//	uint32_t b01: 1;
//	uint32_t b02: 1;
//	uint32_t b03: 1;
//	uint32_t b04: 1;
//	uint32_t b05: 1;
//	uint32_t b06: 1;
//	uint32_t b07: 1;
//	uint32_t b08: 1;
//	uint32_t b09: 1;
//	uint32_t b10: 1;
//	uint32_t b11: 1;
//	uint32_t b12: 1;
//	uint32_t b13: 1;
//	uint32_t b14: 1;
//	uint32_t b15: 1;
//	uint32_t b16: 1;
//	uint32_t b17: 1;
//	uint32_t b18: 1;
//	uint32_t b19: 1;
//	uint32_t b20: 1;
//	uint32_t b21: 1;
//	uint32_t b22: 1;
//	uint32_t b23: 1;
//	uint32_t b24: 1;
//	uint32_t b25: 1;
//	uint32_t b26: 1;
//	uint32_t b27: 1;
//	uint32_t b28: 1;
//	uint32_t b29: 1;
//	uint32_t b30: 1;
//	uint32_t b31: 1;
//}BIT_MADK;

//typedef union _UNION_MASK
//{
//	uint32_t value;
//	BIT_MADK b;
//}UNION_MASK;

typedef struct {
  uint8_t retreve:1;
  uint8_t saving:1;
  uint8_t x2:1;
  uint8_t x3:1;
  uint8_t x4:1;
  uint8_t x5:1;
  uint8_t x6:1;
  uint8_t x7:1;
}BIT_COMMAND;

typedef union _TX_CONFIG_BIT
{
	uint8_t u8;
	BIT_COMMAND b;
}TX_COMAND_BYTE;


//typedef struct {
//  uint8_t rxData[8];
//}CAN_TX_CONFIG_DATA;


typedef struct {
   uint8_t rxData[8];
   uint8_t txData[8];
} CAN_User_InitTypeDef;

typedef struct {
  int16_t volt_main; //2byte
  int16_t volt_dcdc; //2byte
  int16_t volt_rev; //2byte
}VOLT_MAIN;

typedef struct {
  int16_t cvolt_avg; //2byte
  int16_t consumption; //2byte
  int16_t current; //2byte
  int16_t soc; //2byte
}SOC_TABLE;


class tja1050
{
private:
	//uint32_t  id_change(uint32_t src);
	//uint8_t  get_sub_id();
public:
	tja1050();
	virtual ~tja1050();
	void init();
	void can_start();
	void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan_t);
	void put_canTxd(uint32_t tx_id, uint8_t *data);

	uint8_t can_recved=0;
	uint32_t can_rx_id=0x07B0;
	uint32_t can_tx_id=0x07A0;

	uint32_t TxMailbox;
	uint8_t can_tx[8];
	RX_CAN_DATA mc1_can_rx_data;
	RX_CAN_DATA mc2_can_rx_data;
	uint8_t can_exist=0;
	VOLT_MAIN volt_main;
	SOC_TABLE soc;
	uint8_t CAN_RX_BUF[8][8];
	//uint8_t CAN_RX_rspBuf[8];
	uint8_t CAN_BUF_IDX;
	uint16_t canAliveTimeOut=CAN_ALIVE_TIMEOUT;

};

#endif /* TJA1050_TJA1050_H_ */


