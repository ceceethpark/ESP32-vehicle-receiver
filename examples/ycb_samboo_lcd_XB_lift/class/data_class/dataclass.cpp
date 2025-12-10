/*
 * dataclass.cpp
 *
 *  Created on: Jul 19, 2024
 *      Author: thpark
 */
#include "extern.h"
#include "dataclass.h"
#include "math.h"

data_class *pDataClass;

data_class::data_class()
{
	gMAIN.fet_temp = 0;
	gMAIN.m2_fet_temp = 0;
	memset(&sysFlag, 0, sizeof(sysFlag));
	memset(&rcu_data, 0, sizeof(rcu_data));
	memset(&setup_data, 0,	sizeof(CONFIG_TOTAL));
}

data_class::~data_class()
{
}

void data_class::CAN_PUT_CommandEvt(VCU_SERVICE_DATA_UNIT vcu_sdu)
{
	uint8_t buf[8];
	buf[0]=HI_UINT16(vcu_sdu.orgspeed);
	buf[1]=LO_UINT16(vcu_sdu.orgspeed);
	buf[2]=HI_UINT16(vcu_sdu.limit);
	buf[3]=LO_UINT16(vcu_sdu.limit);
	buf[4]=0xAA;
	buf[5]=0xBB;
	buf[6]=vcu_sdu.toggle.u8;
	buf[7]=vcu_sdu.btn.u8;
	pCAN->put_canTxd(CAN_TX_PUT_CMD, buf);
	//HAL_Delay(1);
}

void data_class::CAN_SaveConfigEvent()
{
	printf("CAN_SaveConfigEvent\r\n");
	//CAN_TX_SAVE_CMD
	uint8_t buf[64];
	uint8_t i,sum=0;
	setup_data.system_conf2.checkSum=0x00;
	memcpy(buf, &setup_data, sizeof(setup_data));
	for(i=0;i<38;i++)sum=sum+buf[i];
	sum= sum &0xff;
	buf[38]=sum;
	//for(i=0;i<40;i++)printf("[%02d][%03d]\r\n",i, buf[i]);
	//printf("battery_voltage[%d]", setup_data.system_conf1.battery_voltage);
	//printf("brake_delay[%d]", setup_data.system_conf2.brake_delay);
	for(i=0;i<5;i++) {
		pCAN->put_canTxd(CAN_TX_SAVE_CMD+i, buf+(i*8));
		HAL_Delay(5);
	 }
}

void data_class::CAN_Response(uint8_t *buf)
{
	can_resp.ex_pwm1=buf[0];
	can_resp.ex_pwm2=buf[1];
	can_resp.BatteryVolt=BUILD_UINT16(buf[3], buf[2]);
	can_resp.fet_temp=buf[4];
	can_resp.motor_temp=buf[5];
	can_resp.motor_rpm=buf[6];
	can_resp.flg_state.u8=buf[7];
	recv_can_emergency=can_resp.flg_state.bisang;
}

void data_class::GET_SystemConfig(){

	uint8_t buf[8];
	uint8_t i=0;
	string str="";
	TX_COMAND_BYTE cmd;
	memset(buf,0,8);
	cmd.b.retreve=1;
	buf[0]=0XAA;
	buf[1]=cmd.u8;
	buf[7]='@';

	pCAN->CAN_BUF_IDX=0;
	pCAN->put_canTxd(CAN_TX_GET_CONFIG, buf);
	HAL_Delay(10);
	for(;;){
		if(pCAN->CAN_BUF_IDX==0x3F){
			//printf("...\r\n");
			sysFlag.canReady=1;
			pCAN->CAN_BUF_IDX=0;
			break;
		}
		i++;
		i %=100;
		str=format("연결중....[%02X][%02d]",pCAN->CAN_BUF_IDX, i);
		pIli9488->draw16_string(100, 60, TFT_BLACK,  gBG_COLOR, (char*)str.c_str(),2, 0);
		pCAN->put_canTxd(CAN_TX_GET_CONFIG, buf);
		__HAL_IWDG_RELOAD_COUNTER(&hiwdg);
		HAL_Delay(200);
	}
	HAL_Delay(10);
	printf("(2)read config OKK~~ canReady[%d] sz[%d]\r\n",sysFlag.canReady, sizeof(CONFIG_TOTAL));
#if 1
	//test
	if(sysFlag.canReady){
		memcpy(&setup_data, &pCAN->CAN_RX_BUF, sizeof(setup_data));
		//memcpy(&pData1, &setup_data.system_conf1, sizeof(SYSTEM_CONF1));
		//memcpy(&pData2, &setup_data.system_conf2, sizeof(SYSTEM_CONF2));
		//for(i=0;i<10;i++) printf("pData1[%02d][%04x]\r\n",i,pData1[i]);
		//for(i=0;i<10;i++) printf("pData2[%02d][%04x]\r\n",i,pData2[i]);
	}
#endif
	//for(i=0;i<8;i++) printf("rd_buf[%d] : [%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X]\r\n", i,rd_buf[i][0],rd_buf[i][1],rd_buf[i][2],rd_buf[i][3],rd_buf[i][4],rd_buf[i][5],rd_buf[i][6],rd_buf[i][7] );
}

void data_class::SYSTEM_POWER(uint8_t on)
{
	//data_init();
	pIli9488->ILI9488_DRV_Init();
	rcu_data = pZIGBEE->zigbee_init_rcuData(rcu_data);
	if (on)
	{
		printf("##POWER(ON)\r\n");
		pIli9488->ILI9488_Draw_Rectangle(MAIN_X, MAIN_Y, 480, 320, gBG_COLOR);
		pIli9488->LCD_BACK_LIGHT(ON);
		GET_SystemConfig();

		pLCD_MAIN->initPage();

		gMAIN.relay.MC1 = 1;
		gMAIN.relay.MC2 = 1;
		gMAIN.relay.BRAKE = 0;
		gMAIN.relay.FAN1 = 0;
		gMAIN.relay.FAN2 = 0;
	}
	else
	{ //power off
		printf("##POWER(OFF)\r\n");
		pIli9488->LCD_BACK_LIGHT(OFF);
		gMAIN.relay.MC1 = 0;
		gMAIN.relay.MC2 = 0;
		gMAIN.relay.BRAKE = 1;
		gMAIN.relay.FAN1 = 0;
		gMAIN.relay.FAN2 = 0;
	}
}


void data_class::Light_Ctrl()
{
//	if (gMAIN.relay.LLIGHT)
//		HAL_GPIO_WritePin(pRY1_GPIO_Port, pRY1_Pin, GPIO_PIN_RESET);
//	else
//		HAL_GPIO_WritePin(pRY1_GPIO_Port, pRY1_Pin, GPIO_PIN_SET);
//
//	if (gMAIN.relay.RLIGHT)
//		HAL_GPIO_WritePin(pRY2_GPIO_Port, pRY2_Pin, GPIO_PIN_RESET);
//	else
//		HAL_GPIO_WritePin(pRY2_GPIO_Port, pRY2_Pin, GPIO_PIN_SET);
}

void data_class::CAN_Publish(uint32_t id)
{
	uint8_t buf[8] ={0, };
	TX_COMAND_BYTE cmd;
	cmd.b.retreve=1;
	buf[0]=cmd.u8;
	pCAN->put_canTxd(id, buf);
}

void data_class::ten_millisec_routine()
{
	Get_AdcData();
	//read_in_port();
}

void data_class::hnd_millisec_routine()
{
	//HAL_GPIO_TogglePin(pRY4_GPIO_Port, pRY4_Pin);//for test timer
	//leap timeout++
	if(sysFlag.leap){
		if(Leap_TimeOut)Leap_TimeOut--;
		if(Leap_TimeOut==0)sysFlag.leap=0;
	}
	//leap timeout--

	if(pCAN->canAliveTimeOut>0)pCAN->canAliveTimeOut--;
	if(UI_TimeOut>0)UI_TimeOut--;
	//if(HOLD_TimeOut>0)HOLD_TimeOut--;	//hold timeOut
	if(UI_TimeOut==0){
		UI_TimeOut=2;
		draw_UI_flag=1;
	}
	vcu_sdu=MakeVCU_Data();
	CAN_PUT_CommandEvt(vcu_sdu);
	//CAN_Publish();
	//pZIGBEE->zigbee_send();
	//if(pZIGBEE->zigbee_remain>0)pZIGBEE->zigbee_remain--;
}

void data_class::onesec_routine()
{
	toggle ^= 1;
	int16_t voltage;
	//CAN_Publish(CAN_TX_GET_CONFIG);
	if (wait_time_out > 0)	wait_time_out--;

//battery voltage++
	if(battery_check_timeout>0)battery_check_timeout--;
	BattVoltage=get_voltage(get_battery_filter(ladcValue[5]));
	if(BattVoltage<exBattVoltage)gMAIN.InstantaneousVoltage =BattVoltage;
	else if(BattVoltage>(exBattVoltage+10))gMAIN.InstantaneousVoltage =BattVoltage;
	else gMAIN.InstantaneousVoltage =exBattVoltage;
//battery voltage--
	//gMAIN.InstantaneousVoltage = get_voltage(get_battery_filter(ladcValue[5]));
	//gMAIN.InstantaneousVoltage=get_voltage(ladcValue[5]);
	if(battery_check_timeout){
		//gMAIN.battery_voltage = voltage;
	}
	//printf("gMAIN.battery_voltage[%d] gMAIN.InstantaneousVoltage[%d]\r\n", gMAIN.battery_voltage, gMAIN.InstantaneousVoltage);

	if(pZIGBEE->zigbee_remain>0)pZIGBEE->zigbee_remain--;
	if(pZIGBEE->zigbee_remain)
	{
		//gMAIN.fet_temp=dcu_data.ntc_fet;
		//gMAIN.motor_temp=dcu_data.ntc_mot;
		//gMAIN.rssi=dcu_data.rssi;
		//gMAIN.charger_voltage = 0;
	}
	else {
		//gMAIN.InstantaneousVoltage=0;
		//gMAIN.battery_voltage = 0;
		//gMAIN.charger_voltage = 0;
	}
//	if (fet_teperature > 40)
//	{
//		gMAIN.relay.FAN1 = 1;
//		gMAIN.relay.FAN2 = 1;
//	}
//	if (fet_teperature < 35)
//	{
//		gMAIN.relay.FAN1 = 0;
//		gMAIN.relay.FAN2 = 0;
//	}
	//gMAIN.m1_motor_temp=pCAN->mc1_can_rx_data.motor_temp;
	//gMAIN.m2_motor_temp=pCAN->mc2_can_rx_data.motor_temp;
//	gMAIN.m1_state_flg.u8 = pCAN->mc1_can_rx_data.state.u8;
//	gMAIN.m2_state_flg.u8 = pCAN->mc2_can_rx_data.state.u8;
//	if(pRS485->rs485_sht20.state_bit.exist){
//		measured_temperature=pRS485->rs485_sht20.temperature + SENSOR_TEMP_CALIBRATION;
//		measured_humidity=(int)pRS485->rs485_sht20.humidity+SENSOR_HUMI_CALIBRATION;
//		measured_ozone=(int)pRS485->rs485_sht20.o3_ppb;
//	}
//	else {
//		measured_temperature=230;
//		measured_humidity=500;
//		measured_ozone=0;
//	}
	if(pSTM32_IO->navi_state.system_setup==0) pLCD_MAIN->update_DrawMainMenu(1);

#if 1
	printf("\r\nDCU=================== zigbee_remain[%d]\r\n", pZIGBEE->zigbee_remain);
	//CAN_RX_rspBuf
	printf("rpm[%04d] expwm1[%04d][%04d] fT[%03d] mT[%03d] state[%02X]\r\n",can_resp.motor_rpm, can_resp.ex_pwm1,can_resp.ex_pwm2, can_resp.fet_temp, can_resp.motor_temp, can_resp.flg_state);

	//printf("TOGGLE_B[%d]\r\n", gMAIN.quad_sw1.b.TOGGLE_B);
	if(pZIGBEE->zigbee_remain){
		//printf("joy_x[%.3f] joy_y[%.3f]\r\n",joy_x, joy_y);
		//printf("(1)RCU speed[%d] limit[%d] leap_time[%d]\r\n", rcu_data.speed, rcu_data.limit, leap_time);
		//printf("(2)DCU speed[%d] limit[%d] leap_time[%d]\r\n", dcu_data.speed, dcu_data.limit, leap_time);
		//printf("RCU(1)jenhujin[%d] stop_lr[%d]\r\n", rcu_data.btn1.b.jenhujin, rcu_data.btn1.b.stopBtn_lr);
		//printf("DCU(2)jenhujin[%d] stop_lr[%d]\r\n", dcu_data.btn1.b.jenhujin, dcu_data.btn1.b.stopBtn_lr);
		//printf("DCU fet_temp[%d] motor_temp[%d]\r\n", gMAIN.fet_temp, gMAIN.motor_temp);
		//printf("DCU embrake[%d] emergency[%d]\r\n", dcu_data.btn1.b.embrake, dcu_data.btn1.b.emergency);
		//printf("DCU jenhujin[%d] stop_lr[%d] lift[%d]\r\n",dcu_data.btn1.b.jenhujin,dcu_data.btn1.b.stopBtn_lr,dcu_data.btn1.b.liftBtn_ud);
		//printf("RSSI[%d] Vbat[%d] \r\n", gMAIN.rssi, gMAIN.battery_voltage);
		//printf("qSW1[%2x] qSW2[%2x]\r\n", gMAIN.quad_sw1.u8, gMAIN.quad_sw2.u8);
	}
#endif

//	printf("ladcValue[%04d][%04d][%04d][%04d]-[[%04d][%04d][[%04d]\r\n",
//			ladcValue[0], ladcValue[1], ladcValue[2], ladcValue[3],
//			ladcValue[4], ladcValue[5], ladcValue[6]);
}

VCU_SERVICE_DATA_UNIT data_class::MakeVCU_Data()
{
	VCU_SERVICE_DATA_UNIT lsdu;
	uint8_t jenhujin=0,sum,cruise_key=0,stop_key=0, leap_key=0;

	int16_t speed1, speed2,limit,leap;
	float fTottle;// = (ladcValue[0] / 4096.0f);//0~1.0f
	float fLimit =  (ladcValue[0] / 4096.0f);//0~1.0f
	float fLeap =   (ladcValue[1] / 4096.0f);//0~1.0f

	fTottle=(my_button.b.running) ? 1.0f:0.0f;

	org_speed=speed1=speed2= (int16_t)(fTottle*TROTTLE_MAX);
	limit_speed=limit= (int16_t)( fLimit*LIMIT_MAX);
	leap_time=leap = (int16_t)(fLeap*LEAP_TIME_MAX);

	if(leap>=LEAP_TIME_MAX)leap=LEAP_TIME_MAX-1;
	else if(leap<5)leap=0;

	//leap++
	if(sysFlag.leap){
	  org_speed=1000;
//	  limit=limit_speed;
	}
	//leap--

	jenhujin=my_toggle.b.jenhujin;
	stop_key=my_toggle.b.stop_lr;
	//if(pDataClass->setup_data.system_conf1.cart_type)stop_key=0;
	printf("my_toggle[%02X][%02X][%02X] my_button[%02X]\r\n",my_toggle.b.jenhujin, my_toggle.b.stop_lr,  my_toggle.b.lift1_ud, my_button.u8);

#if 0
//stop++
	//rcu_data.btn1.b.stopBtn_lr=current_stopKey;

	if(jenhujin==0){//Spin condition: 중립, 쓰로틀값이 유효, 좌우 스톱인 경우 스핀동작
		switch(stop_key)
		{
			case 0:	//no press stop
				speed1=0;//org_speed;
				speed2=0;//org_speed;
				break;
			case 1:
				speed1=0;
				speed2=org_speed;
				break;
			case 2:
				speed1=org_speed;
				speed2=0;
				break;
			case 3:
				speed1=0;
				speed2=0;
				break;
		}
	}
	else if(jenhujin==1){
		switch(stop_key)
		{
			case 0:	//no press stop
				speed1=org_speed;
				speed2=org_speed;
				break;
			case 1:
				speed1=0;
				speed2=org_speed;
				break;
			case 2:
				speed1=org_speed;
				speed2=0;
				break;
			case 3:
				speed1=0;
				speed2=0;
				break;
		}
	}
	else if(jenhujin==2){
		switch(stop_key)
		{
			case 0:	//no press stop
				speed1=org_speed;
				speed2=org_speed;
				break;
			case 1:
				speed1=0;
				speed2=org_speed;
				break;
			case 2:
				speed1=org_speed;
				speed2=0;
				break;
			case 3:
				speed1=0;
				speed2=0;
				break;
		}
	}

//	printf("(1)state[%d]-[%d][%d] spin_key[%d] lock_key[%d]\r\n",JuhangState, rcu_data.btn1.b.jenhujin, rcu_data.btn1.b.stopBtn_lr,spin_key,lock_key);
//stop--
#endif
	if(jenhujin==0){
		speed1=speed2=0;
		limit=0;
	}
	if(sysFlag.bisang){
		jenhujin=0;
		org_speed=0;
		limit=0;
	}

	lsdu.orgspeed=org_speed;
	lsdu.speed1=speed1;
	lsdu.speed2=speed2;
	lsdu.limit=limit;
	lsdu.toggle.b.jenhujin=jenhujin;
	lsdu.toggle.b.stop_lr= 3;//my_toggle.b.stop_lr;//stop_key;
	lsdu.toggle.b.lift1_ud= my_toggle.b.lift1_ud;
	lsdu.toggle.b.lift2_ud= my_toggle.b.lift2_ud;
	lsdu.btn.b.emergency = sysFlag.bisang;//my_button.b.emergency;
	//jenhujin=my_toggle.b.jenhujin;
	//stop_key=my_toggle.b.stop_lr;
//	printf("jenhujin[%02d] org_speed[%03d] speed1[%03d] speed2[%03d] limit[%03d] leap[%03d] \r\n",jenhujin, org_speed,  speed1, speed2, limit, leap);

////checksum++
//	memcpy(pBuf, &rcu_data, 16);
//	sum=0;
//	for(i=0;i<14;i++)sum=sum+pBuf[i];
//	sum=sum&0xFF;
////checksum--
//	rcu_data.checksum=sum;
#if 0
	memset(pBuf, 0, 16);
	memcpy(pBuf, &rcu_data, 16);
	sum=0;
	for(i=0;i<14;i++)sum=sum+pBuf[i];
	sum=sum&0xFF;
	printf("sum[%x] pBuf[%x] rcu_data.checksum[%x]\r\n", sum, pBuf[14], rcu_data.checksum);
	printf("sum[%x] pBuf[%x] [%02X][%02X][%02X][%02X]-[%02X][%02X][%02X][%02X]-[%02X][%02X][%02X][%02X]-[%02X][%02X][%02X][%02X]\r\n",
			sum, pBuf[14], pBuf[0],pBuf[1],pBuf[2],pBuf[3],pBuf[4],pBuf[5],pBuf[6],pBuf[7],pBuf[8],pBuf[9],pBuf[10],pBuf[11],pBuf[12],pBuf[13],pBuf[14],pBuf[15]);
	//printf("speed[%d] limit[%d]\r\n",rcu_data.speed, rcu_data.limit);
#endif
	//printf("@@@@JuhangState[%d] speed[%d][%d] jenhujin[%d] stopBtn_lr[%d]\r\n",JuhangState, rcu_data.speed,rcu_data.limit, jenhujin,rcu_data.btn1.b.stopBtn_lr);
	return lsdu;
}

void data_class::Get_AdcData()
{
	uint8_t i;
	uint16_t buf[7];
	for (i = 0; i < 7; i++)
	{
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
		buf[i] = HAL_ADC_GetValue(&hadc1);
	}
	//ladcValue[0] = get_m1_filter(buf[0]);
	ladcValue[0] =buf[0];
	//ladcValue[1] = get_m2_filter(buf[1]);
	ladcValue[1] = buf[1];
	ladcValue[2] = get_m3_filter(buf[2]);
	//ladcValue[3] = get_m4_filter(buf[3]);
	ladcValue[3] = buf[3];
//--------------------------------------
	//over_heating=0;
//	if(fet_teperature>95){
//		over_heating=1;
//		ladcValue[0]=ladcValue[1]=ladcValue[2]=ladcValue[3]=0;
//	}
//	else if(fet_teperature<90){
//		over_heating=0;
//	}
//-------------------------------------
//	if((gMAIN.quad_sw2.b.TOGGLE_B & 0x02)) ladcValue[0]=0;//check run motor adc
//	if((gMAIN.quad_sw2.b.TOGGLE_B & 0x01)&& wait_time_out==0) {
//		Emergency=1;
//		//printf("Emergency ====================\r\n");
//	}
	for (i = 4; i < 7; i++)
	{
		ladcValue[i] = buf[i];
	}
//	if(Emergency){
//		ladcValue[0]=ladcValue[1]=ladcValue[2]=ladcValue[3]=0;
//	}
	gMAIN.battery_voltage = get_voltage(get_battery_filter(ladcValue[5]));
}

uint16_t data_class::get_voltage(uint16_t value)
{
	uint16_t voltage;
	float t;
	t = (float) value * 3.3f / 0xfff; // 읽은 센서값을 전압으로 변경
	t = t * 520; //51K 1K x10
	voltage = (uint16_t) t;
	//printf("1)value[%d] voltage[%.2f]\r\n", value , t);
	return voltage;
}

int16_t data_class::FormSensorMnt_CalCPUTemp(uint16_t value)
{
	float t;
	static int16_t bf[10] =
	{ 0 };
	static uint8_t i = 0;
	uint8_t j;
	int16_t sum = 0;

	t = (float) value * 3.3f / 0xfff; // 읽은 센서값을 전압으로 변경
	// printf("1)value[%d] voltage[%.2f]\r\n", value , t);
	t = (1.43 - t) / 0.0043 + 25.0; // 슬로프오 오프셋을 계산
	//printf("2)value[%d] voltage[%.2f]\r\n", value , t);
	bf[i] = (int16_t) t; // 이동평균 계산을 위한 버퍼에 저장
	i++;
	i %= 10;
	sum = 0;
	for (j = 0; j < 10; j++) // 이동 평균 계산.
	{
		sum += bf[j];
	}
	sum /= 10;
	return sum;
}
uint16_t data_class::get_m1_filter(uint16_t adc)
{
	static float m1_value;
	m1_value = (m1_value * (1 - ADC_SENSITIVITY)) + (adc * ADC_SENSITIVITY);
	return (uint16_t) m1_value;
}

uint16_t data_class::get_m2_filter(uint16_t adc)
{
	static float m2_value;
	m2_value = (m2_value * (1 - ADC_SENSITIVITY)) + (adc * ADC_SENSITIVITY);
	return (uint16_t) m2_value;
}

uint16_t data_class::get_m3_filter(uint16_t adc)
{
	static float m3_value;
	float sesityity=0.05f;
	m3_value = (m3_value * (1 - sesityity)) + (adc * sesityity);
	return (uint16_t) m3_value;
}

uint16_t data_class::get_m4_filter(uint16_t adc)
{
	static float m4_value;
	m4_value = (m4_value * (1 - ADC_SENSITIVITY)) + (adc * ADC_SENSITIVITY);
	return (uint16_t) m4_value;
}

uint16_t data_class::get_battery_filter(uint16_t adc)
{
	static float bat_value;
	bat_value = (bat_value * (1 - ADC_SENSITIVITY)) + (adc * ADC_SENSITIVITY);
	return (uint16_t) bat_value;
}

int16_t data_class::get_ntc_temperature(uint16_t adc)
{
#define ADC_FULL_SCALE 330 //3.3V
#define PARAMETER_BETA 3435.0f
#define RES25oC        10000.0f  //25도 써미스트값
#define R10_RES        10000.0f  //ohm 10k 1%

	float ohm;
	int16_t temperaure;
	float fV, tempC;

	fV = adc * (ADC_FULL_SCALE / 4096.0f); //12bit
	ohm = ((ADC_FULL_SCALE / fV) - 1) * R10_RES; //ohm
	tempC = (PARAMETER_BETA
			/ (log(ohm / RES25oC) + (PARAMETER_BETA / (273.15f + 25.0f))))
			- 273.15f;
	temperaure = (int16_t) tempC;
#if 1
	printf("adc[%d] fVx100[%.2f] ohm[%.1f] temp[%.2f] temperaure[%d]\r\n", adc,
			fV, ohm, tempC, temperaure);
#endif

	return temperaure;
}
