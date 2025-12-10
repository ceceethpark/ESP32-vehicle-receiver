/*
 * zigbee.cpp
 *
 *  Created on: Oct 11, 2024
 *      Author: thpark
 */
#include "extern.h"
#include "zigbee.h"

zigbee *pZIGBEE;

zigbee::zigbee()
{
	// TODO Auto-generated constructor stub
	//memset(dcu_tx_data,0,sizeof(DCU_TXD));
}

zigbee::~zigbee()
{
	// TODO Auto-generated destructor stub
}

RCU_DATA_STRUCTURE zigbee::zigbee_init_rcuData(RCU_DATA_STRUCTURE lrcuTx){
	lrcuTx.stx=RCU2DCU_STX;
	lrcuTx.id=0xCC;
	lrcuTx.speed=0x00;
	lrcuTx.limit=0x00;
	lrcuTx.tbd[0]=0x00E0;
	lrcuTx.tbd[1]=0x01E1;
	lrcuTx.tbd[2]=0x02E2;
	lrcuTx.btn1.u8=0x00;
	lrcuTx.btn2.u8=0x00;
	lrcuTx.checksum=0x00;
	lrcuTx.etx=ETX;
	return lrcuTx;
}


void zigbee::zigbee_received(uint8_t *pData)
{
	DCU_DATA_STRUCTURE ldcu;
	uint8_t sum=0;

	for(int i=0;i<13;i++)sum=sum+pData[i];
	sum=sum&0xFF;
	memcpy(&ldcu, pData, 16);

	printf("@@@@sum[%x] pData[%x] checksum[%x]\r\n",sum, pData[14], ldcu.checksum);
	if(sum==ldcu.checksum && ldcu.id==ZIGBEE_MY_ID){
		zigbee_remain=ZIGBEE_REMAIN;
		memcpy(&pDataClass->dcu_data, pData, 16);
	}
#if 0
	printf("[%02X][%02X][%02X][%02X]-[%02X][%02X][%02X][%02X]-[%02X][%02X][%02X][%02X]-[%02X][%02X][%02X][%02X]\r\n",
			pData[0],pData[1],pData[2],pData[3],pData[4],pData[5],pData[6],pData[7],pData[8],pData[9],pData[10],pData[11],pData[12],pData[13],pData[14],pData[15]);
	printf("speed[%d] limit[%d] etx[%02X]\r\n",pDataClass->dcu_data.speed, pDataClass->dcu_data.limit, pDataClass->dcu_data.etx);
#endif
}

void zigbee::zigbee_send()
{
	uint8_t pData[20];
	memcpy(&pData, &pDataClass->rcu_data, 16);
	//printf("\r\n ===rcuTx(%x)===\r\n", pDataClass->rcu_data.stx);
	//for(int i=0;i<16;i++)printf("rcutx [%d][%x]\r\n",i, pData[i]);
	//for(int i=1;i<16;i++)pData[i]=i;
	//pData[15]=0x40;
	HAL_UART_Transmit(&huart1, (uint8_t*)pData, 16, 50);

}
