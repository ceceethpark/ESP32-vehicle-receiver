/*
 * dataclass.h
 *
 *  Created on: Jul 19, 2024
 *      Author: thpark
 */

#ifndef DATA_CLASS_DATACLASS_H_
#define DATA_CLASS_DATACLASS_H_


class data_class
{
private:

	int8_t wait_time_out=5;
	int16_t FormSensorMnt_CalCPUTemp(uint16_t value);
	int16_t get_ntc_temperature(uint16_t adc);
	void Get_AdcData();
	void Light_Ctrl();
	uint16_t get_voltage(uint16_t value);
	uint16_t get_m1_filter(uint16_t adc);
	uint16_t get_m2_filter(uint16_t adc);
	uint16_t get_m3_filter(uint16_t adc);
	uint16_t get_m4_filter(uint16_t adc);

	void CAN_Publish(uint32_t id);
	void CAN_PUT_COMAND_EVT();

	uint8_t battery_check_timeout=BATTERY_CHECK_TIME_OUT;
	uint8_t cruise_Flag=0;
	uint8_t Leap_Flag=0;
	uint16_t UI_TimeOut=3;
	uint8_t Leap_key_debounce_cnt=5;
	uint8_t cruise_key_debounce_cnt=5;
	VCU_SERVICE_DATA_UNIT MakeVCU_Data();
	void CAN_PUT_CommandEvt(VCU_SERVICE_DATA_UNIT vcu_sdu);
	uint16_t BattVoltage, exBattVoltage;
public:
	MAIN_MAIN gMAIN;
	VCU_SERVICE_DATA_UNIT vcu_sdu;
	CONFIG_TOTAL setup_data;
	SYSTEM_CONF1 system_conf1;
	SYSTEM_CONF1 system_conf2;
	CAN_RESPONSE can_resp;
	MY_TOGGLE my_toggle;
	MY_BUTTON my_button;
	BIT_SYSTEM_FLAG sysFlag;

	int16_t org_speed, cruise_speed;
	int16_t leap_time;
	uint16_t Leap_TimeOut=0;

	uint8_t state=0;
	uint8_t toggle=0;
	uint8_t ex_state=0xFF;
	uint8_t fet_teperature=0;
	uint8_t motor_teperature=0;
	uint8_t over_heating=0;

///For RCU_LCD++
	RCU_DATA_STRUCTURE rcu_data;
	DCU_DATA_STRUCTURE dcu_data;
	VCU_CAN_TX_RUN can_tx_run;
	VCU_CAN_TX_SET can_tx_set;

	uint16_t ladcValue[9];
	int16_t limit_speed=0;
	uint8_t recv_can_emergency=0;
	//int16_t trottle_percent=0;
	//uint8_t setting_mode=0;
///For RCU_LCD--

	data_class();
	virtual ~data_class();
	void SYSTEM_POWER(uint8_t on);
	void GET_SystemConfig();
	void one_millisec_routine();
	void ten_millisec_routine();
	void hnd_millisec_routine();

	void onesec_routine();
	void change_direction();
	uint8_t get_state();
	void change_subroutine(uint8_t state);
	void CAN_Response(uint8_t *buf);
	void CAN_SaveConfigEvent();
	uint16_t get_battery_filter(uint16_t adc);
};

#endif /* DATA_CLASS_DATACLASS_H_ */
