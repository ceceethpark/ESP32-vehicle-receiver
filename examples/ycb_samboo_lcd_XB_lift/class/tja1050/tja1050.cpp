/*
 * tja1050.cpp
 *
 *  Created on: Jul 15, 2024
 *      Author: thpark
 */
#include "extern.h"
#include "tja1050.h"

tja1050 *pCAN;



tja1050::tja1050()
{
	// TODO Auto-generated constructor stub
	//init();
	CAN_BUF_IDX=0;
}

tja1050::~tja1050()
{
	// TODO Auto-generated destructor stub
}


void tja1050::init(void)
{
  //printf("TJA1050::init\r\n");
  can_start();
  can_rx_id=CAN_RX_DATA_ID;//RX_CANID;
  memset(&mc1_can_rx_data,0,8);
  memset(&mc2_can_rx_data,0,8);
  can_start();
}

void tja1050::can_start()
{
  CAN_FilterTypeDef sFilterConfig;

#ifdef CAN20B
  uint32_t filter_id=id_change(can_rx_id);
  uint32_t filter_mask=id_change(0x1FFFFFFE);//1111 1111 1111 1100 last 2bit unmask 0~3(DECEE00~DECEE03)

  uint32_t FilterIdHigh=((filter_id << 5)  | (filter_id >> (32 - 5))) & 0xFFFF;
  uint32_t FilterIdLow= (filter_id >> (11 - 3)) & 0xFFF8;

  uint32_t FilterMaskIdHigh=((filter_mask << 5)  | (filter_mask >> (32 - 5))) & 0xFFFF;
  uint32_t FilterMaskIdLow= (filter_mask >> (11 - 3)) & 0xFFF8;

  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh =FilterIdHigh; // STID[10:0] & EXTID[17:13]
  sFilterConfig.FilterIdLow = FilterIdLow; // EXID[12:5] & 3 Reserved bits
  sFilterConfig.FilterMaskIdHigh =FilterMaskIdHigh;
  sFilterConfig.FilterMaskIdLow =FilterMaskIdLow;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14;

#else
//0x5B0~0x5BF
  uint32_t filter_id=can_rx_id<<5;
  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;
  sFilterConfig.FilterIdHigh =filter_id;//FilterIdHigh; // STID[10:0] & EXTID[17:13]
  sFilterConfig.FilterIdLow = 0;//FilterIdLow; // EXID[12:5] & 3 Reserved bits
  sFilterConfig.FilterMaskIdHigh =0x7F0<<5;//FilterMaskIdHigh;
  sFilterConfig.FilterMaskIdLow =0x7F0<<5;//0;//FilterMaskIdLow;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14;
#endif

   HAL_CAN_ConfigFilter(&hcan, &sFilterConfig);

  if (HAL_CAN_Start(&hcan) != HAL_OK)
  {
     // Error_Handler();
  }
  if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK){
	 // printf("error !! HAL_CAN_ActivateNotification can_rx_id[%x] filter_id[%x]\r\n",can_rx_id, filter_id);
    Error_Handler();
  }
}


void tja1050::put_canTxd(uint32_t tx_id, uint8_t *data) // CAN
{
    CAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[8];
    memcpy(TxData, data ,8);
    TxHeader.StdId = tx_id;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.DLC = 8;
    TxHeader.TransmitGlobalTime = DISABLE;

    TxMailbox = HAL_CAN_GetTxMailboxesFreeLevel(&hcan);
    if ((HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox))!=HAL_OK){
                 Error_Handler();
     }
    while (HAL_CAN_IsTxMessagePending(&hcan, TxMailbox));
}


void tja1050::HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan_t)
{
    CAN_RxHeaderTypeDef RxHeader;


    uint8_t RxData[8]={0,};
    uint8_t rx_idx=0;

	if (hcan_t->Instance == CAN1) { // OK?
	HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &RxHeader, RxData);
	//printf("StdID: %04lx, IDE: %ld, DLC: %ld\r\n", RxHeader.StdId, RxHeader.IDE, RxHeader.DLC);
	//printf("@@@StdID[%04lx]  RxData: %x %x %x %d %x %d %x %x\r\n", RxHeader.StdId, RxData[0], RxData[1], RxData[2], RxData[3], RxData[4], RxData[5], RxData[6], RxData[7]);
	canAliveTimeOut=CAN_ALIVE_TIMEOUT;
	if(RxHeader.StdId>=CAN_RX_DATA_ID && RxHeader.StdId<=0x5B5){
		rx_idx=RxHeader.StdId - CAN_RX_DATA_ID;//0x5B0~0x5B5
		memcpy(&CAN_RX_BUF[rx_idx], RxData,8);
		_SetBit(CAN_BUF_IDX,rx_idx);
		//printf("@@@std[%04lX] CAN_BUF_IDX[%x]\r\n",RxHeader.StdId, CAN_BUF_IDX);
	}
	else if(RxHeader.StdId==CAN_RX_RESPONSE_ID){
		//printf("CAN_RX_RESPONSE_ID [%04lX]\r\n",RxHeader.StdId);
		pDataClass->CAN_Response(RxData);

	}


   }
}

