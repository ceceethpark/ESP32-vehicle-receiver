/*
 * zigbee.h
 *
 *  Created on: Oct 11, 2024
 *      Author: thpark
 */

#ifndef ZIGBEE_ZIGBEE_H_
#define ZIGBEE_ZIGBEE_H_

#define DCU2RCU_STX 0xAF
#define RCU2DCU_STX 0xBF
#define ETX 0x40




class zigbee
{
public:
	zigbee();
	virtual ~zigbee();
	//DCU_TXD dcu_tx_data;
	RCU_DATA_STRUCTURE zigbee_init_rcuData(RCU_DATA_STRUCTURE lrcuTx);
	void zigbee_received(uint8_t *pData);
	void zigbee_send();
	uint8_t zigbee_remain=0;
};

#endif /* ZIGBEE_ZIGBEE_H_ */
