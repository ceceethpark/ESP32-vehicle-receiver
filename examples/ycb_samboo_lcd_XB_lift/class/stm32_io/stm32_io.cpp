//#include "keypad_adc.h"
#include "extern.h"
#include "adc.h"
#include "tim.h"
//#include "sub_menu_string.h"

#define MAX_CTRL_STEP 3
#define SENSOR_NUM 3

//#define KEY_UP 			0x01
//#define KEY_DN     		0x02
#define KEY_DETAIL     	0x04
#define KEY_HOME_MENU   0x10
#define KEY_SETUP_NEXT  0x20
#define KEY_ITEM        0x40
#define LONG_PRESS_ON_CNT    30
#define LONG_PRESS_OFF_CNT   10

//extern RTC_API *pRTC_API;
//extern DATA_CLASS *pDataClass;

STM32_IO *pSTM32_IO;

extern uint8_t sub_item_cnt_arry[4][8][4];

bool buttonState();

STM32_IO::STM32_IO() {
	STM32_IO_init();

}

STM32_IO::~STM32_IO() {
}

void STM32_IO::STM32_IO_init(){
	memset(&navi_state, 0, sizeof(navi_state));
	vKEY_BUFFER.clear();
	//vRELAY_CONTROL_EVENT.clear();
}

void STM32_IO::BEEP_ON(void){
	buzzer_cnt=0;
	//if(_SELJUNG_BUZZER==0)return;
	//HAL_GPIO_WritePin(pBEEP_GPIO_Port, pBEEP_Pin, GPIO_PIN_SET);
}
void STM32_IO::BEEP_OFF(void) {
	//HAL_GPIO_WritePin(pBEEP_GPIO_Port, pBEEP_Pin, GPIO_PIN_RESET);
}

#define LEAP_KEY_TIMEOUT 5

void STM32_IO::Push_Key(){
	uint16_t value=0;
	uint8_t	toggle=0;
	uint8_t key_value=NO_PRESS;
	static uint8_t btn_timeout=LEAP_KEY_TIMEOUT;
//	uint8_t stop_tbl[4]={3, 1, 2, 0};
	if(btn_timeout>0)btn_timeout--;

	toggle|=!!(HAL_GPIO_ReadPin(pIO_AN_GPIO_Port, pIO_AN_Pin));toggle<<=1;
	toggle|=!!(HAL_GPIO_ReadPin(pIO_AP_GPIO_Port, pIO_AP_Pin));toggle %=3;
	//printf("Push_Key->jenhujin[%d] ",toggle);
	if(!navi_state.system_setup) {
		pDataClass->my_toggle.b.jenhujin=toggle;
		if(toggle==0){
			pDataClass->sysFlag.leap=0;
			pDataClass->Leap_TimeOut=0;
		}
	}


	toggle=0;
	toggle|=!!(HAL_GPIO_ReadPin(pIO_BN_GPIO_Port, pIO_BN_Pin));toggle<<=1;
	toggle|=!!(HAL_GPIO_ReadPin(pIO_BP_GPIO_Port, pIO_BP_Pin));toggle %=3;
	if(!navi_state.system_setup) pDataClass->my_toggle.b.lift1_ud=toggle;

	toggle=0;
	toggle|=!!(HAL_GPIO_ReadPin(pIO_CN_GPIO_Port, pIO_CN_Pin));toggle<<=1;
	toggle|=!!(HAL_GPIO_ReadPin(pIO_CP_GPIO_Port, pIO_CP_Pin));toggle %=3;
	if(!navi_state.system_setup) pDataClass->my_toggle.b.lift2_ud=toggle;

	toggle=0;
	toggle|=!!(HAL_GPIO_ReadPin(pIO_DP_GPIO_Port, pIO_DP_Pin));//toggle<<=1;
	if(toggle){
		printf("[KEY_BISANG][%d]\r\n", toggle);
		if(pDataClass->my_button.b.emergency==0){
			pDataClass->my_button.b.emergency=1;
			pDataClass->sysFlag.bisang=1;
		}
	}


	pDataClass->my_button.b.running=0;
	if(!!(HAL_GPIO_ReadPin(pIO_DN_GPIO_Port, pIO_DN_Pin))==0){
		pDataClass->my_button.b.running=1;
		//HAL_Delay(200);
	}

	pDataClass->my_button.b.leap=0;
	if(btn_timeout==0){
		if(!!(HAL_GPIO_ReadPin(pIO_1P_GPIO_Port, pIO_1P_Pin))==0){
			btn_timeout=LEAP_KEY_TIMEOUT;
			pDataClass->my_button.b.leap=1;
			vKEY_BUFFER.push_back(KEY_LEAP);
			//HAL_Delay(500);
			return;
		}
	}

//	if(navi_state.system_setup){
//		printf("[KEY_BISANG]saving set_up\r\n");
//		pDataClass->CAN_SaveConfigEvent();
//		navi_state.system_setup = 0;
//		SYSTEM_MENU();
//	}


	if(!!(HAL_GPIO_ReadPin(pIO_AN_GPIO_Port, pIO_AN_Pin))==0)_SetBit(value, 0);
	if(!!(HAL_GPIO_ReadPin(pIO_AP_GPIO_Port, pIO_AP_Pin))==0)_SetBit(value, 1);
	if(!!(HAL_GPIO_ReadPin(pIO_BN_GPIO_Port, pIO_BN_Pin))==0)_SetBit(value, 2);
	if(!!(HAL_GPIO_ReadPin(pIO_BP_GPIO_Port, pIO_BP_Pin))==0)_SetBit(value, 3);

	if(!!(HAL_GPIO_ReadPin(pIO_CN_GPIO_Port, pIO_CN_Pin))==0)_SetBit(value, 4);
	if(!!(HAL_GPIO_ReadPin(pIO_CP_GPIO_Port, pIO_CP_Pin))==0)_SetBit(value, 5);
	if(!!(HAL_GPIO_ReadPin(pIO_DN_GPIO_Port, pIO_DN_Pin))==0)_SetBit(value, 6);//foot switch (run)
	if(!!(HAL_GPIO_ReadPin(pIO_DP_GPIO_Port, pIO_DP_Pin))==0)_SetBit(value, 7);//비상이 안눌러있는 상태에서

	if(!!(HAL_GPIO_ReadPin(pIO_1N_GPIO_Port, pIO_1N_Pin))==0)_SetBit(value, 8);
	if(!!(HAL_GPIO_ReadPin(pIO_1P_GPIO_Port, pIO_1P_Pin))==0)_SetBit(value, 9);//찔끔

	printf("[Push_Key]value[%04X]\r\n",value);

	switch(value){
		case 0:
			pDataClass->my_button.u8=0;
			key_value=NO_PRESS;
			break;
		case 1:
			if(navi_state.system_setup)	key_value=KEY_UP;
			//else key_value=KEY_LIFT_UP;
			break;
		case 2:
			if(navi_state.system_setup)key_value=KEY_DN;
			//else key_value=KEY_LIFT_DN;
			break;
		case 4:
			if(navi_state.system_setup) key_value=KEY_LEFT;
			//else key_value=KEY_LEFT_STOP;
			break;
		case 8:
			if(navi_state.system_setup)	key_value=KEY_RIGHT;
			//else key_value=KEY_RIGHT_STOP;
			break;
		case 16:key_value=KEY_BISANG;   break;
		case 32:key_value=KEY_CRUISE; HAL_Delay(200);	break;
		//case 64:key_value=KEY_LEAP;   HAL_Delay(200);	break;
		case 28:key_value=KEY_SETING; HAL_Delay(200);	break;//8+4+16=28
		default:
			key_value=NO_PRESS;
			break;
	}

	if(key_value!=NO_PRESS){
		if(beep) BEEP_ON();
		if(key_value!=KEY_ONLY_BEEP) vKEY_BUFFER.push_back(key_value);
		HAL_Delay(200);
	}
}

uint8_t STM32_IO::Get_Key_Buffer(){
	uint8_t key=0;
	if(vKEY_BUFFER.size()){
		key=vKEY_BUFFER[0];
		vKEY_BUFFER.erase(vKEY_BUFFER.begin());
	}
	return key;
}

void STM32_IO::Clear_Key_Buffer(){
	vKEY_BUFFER.clear();
}


void STM32_IO::SYSTEM_MENU(void) {
	MENU_DRAW_EVENT draw_event;
#if 1
	printf("[SYSTEM_MENU] MENU_KeyProcess navi_state.system_setup[%d]\r\n", navi_state.system_setup);
#endif
	draw_event=KEY_CANCEL_EVENT;
	draw_event=KEY_HOME_SETUP_EVENT;
	memset(&navi_state, 0, sizeof(navi_state));
	navi_state.system_setup = 0;
	//navi_state.depth=0;
	//pDataClass->SYSTEM_ChangeToNormal();
	pLCD_MAIN->NAVI_ChangeMode(navi_state, draw_event, 0);
	return;
}

void STM32_IO::LEFT_RIGHT_KeyProcess(uint8_t left, uint8_t plus) {

	MENU_DRAW_EVENT draw_event;
//	int page_cnt=0;
	int lplus;
	uint8_t sub=navi_state.sub;

	lplus=(left)? plus*(-1): plus;
	printf("(1)[LEFT_RIGHT_KeyProcess] sub[%d] item[%d]\r\n", sub, navi_state.item);

	draw_event=KEY_CANCEL_EVENT;
	//if(navi_state.item==0)
	if(navi_state.system_setup){
		if(navi_state.item==0){
			draw_event=KEY_TOP_MODE_SETUP_NEXT_EVENT;
			sub=sub+lplus; sub%=2;
			navi_state.sub =sub;
		}
		else{
			draw_event= KEY_ITEM_VALUE_UPDN_EVENT;
		}
		printf("(2)[LEFT_RIGHT_KeyProcess] draw_event[%d] navi_state.sub[%d] item[%d]\r\n",draw_event, navi_state.sub, navi_state.item);
		pLCD_MAIN->NAVI_ChangeMode(navi_state, draw_event, lplus);
	}
	return;
}


void STM32_IO::UPDN_KeyProcess(uint8_t up, uint8_t plus) {

	MENU_DRAW_EVENT draw_event;
	int total_item_cnt=0, item_cnt;
	int plus_value= (up)? plus:plus*(-1);
	int disp_value=0;
	//if(plus>1)plus_value=(plus_value/10)*10;
#if 1
	draw_event=KEY_CANCEL_EVENT;
	if(navi_state.system_setup){
			//if(navi_state.sub>5)return;
			total_item_cnt =UI_Info.sub_menu_cnt[0];
			item_cnt=navi_state.item;
			if(up)item_cnt--;
			else item_cnt++;
			if(item_cnt<0)item_cnt=0;
			item_cnt %= total_item_cnt;
			printf("UPDN_KeyProcess[%d/%d] \r\n", item_cnt, total_item_cnt);
			navi_state.item=item_cnt;
			draw_event= KEY_TOP_MODE_ITEM_EVENT;
			pLCD_MAIN->NAVI_ChangeMode(navi_state, draw_event, 0);
	}
#endif
}

void STM32_IO::SYSTEM_SELJUNG(void) {
	MENU_DRAW_EVENT draw_event;
#if 1
	printf("[SYSTEM_SELJUNG] system_setup[%d] sub[%d] \r\n", navi_state.system_setup, navi_state.sub);
#endif
	draw_event=KEY_CANCEL_EVENT;
	if (navi_state.system_setup == 0) {
		draw_event=KEY_TOP_MODE_SETUP_NEXT_EVENT;
		memset(&navi_state, 0, sizeof(navi_state));
		navi_state.system_setup=1;
		navi_state.sub=0;
		//navi_state.depth=2;
		pLCD_MAIN->NAVI_ChangeMode(navi_state, draw_event,0);
	}
	return;
}


void STM32_IO::FACTORY_INIT(){
	printf("============Factory init====================\r\n");
	//memcpy(&pLCD_MAIN->UI_Data.data_arry, &init_data, sizeof(init_data));
	//memcpy(&pLCD_MAIN->UI_Data.target, &init_system_target, sizeof(init_system_target));
	//pDataClass->vRec.clear();
}


void STM32_IO::Main_IRQ_KEY_Process()
{
	eButtonEvent key;
	Push_Key();
	key = (eButtonEvent) Get_Key_Buffer();
	if(key==0) return;

	if(pDataClass->sysFlag.cruise){
		pDataClass->sysFlag.cruise=0;
		if(key==KEY_CRUISE) key=NO_PRESS;
	}

	printf("[GET] Main_IRQ_KEY_Process[%d] \r\n",key);
	switch ((uint8_t)key)
	{
		case NO_PRESS:	break;
		case KEY_UP: UPDN_KeyProcess(1, 1);	break;
		case KEY_DN: UPDN_KeyProcess(0, 1);	break;
		case KEY_LEFT:	LEFT_RIGHT_KeyProcess(1,1);	break;
		case KEY_RIGHT:	LEFT_RIGHT_KeyProcess(0,1);	break;
		case KEY_SETING: SYSTEM_SELJUNG();	break;
		case KEY_BISANG:
				pDataClass->sysFlag.bisang=1;
				if(navi_state.system_setup){
					printf("[KEY_BISANG]saving set_up\r\n");
					pDataClass->CAN_SaveConfigEvent();
					navi_state.system_setup = 0;
					SYSTEM_MENU();
				}
				break;
		case KEY_CRUISE:
//				pDataClass->sysFlag.cruise=1;
//				pDataClass->cruise_speed=pDataClass->org_speed;
				break;
		case KEY_LEAP:
				if(pDataClass->sysFlag.leap){
					pDataClass->sysFlag.leap=0;
					pDataClass->Leap_TimeOut=0;
				}
				else{
					pDataClass->sysFlag.leap=1;
					pDataClass->Leap_TimeOut=pDataClass->leap_time;
				}
				break;
		case KEY_LIFT_UP:
		case KEY_LIFT_DN:
		case KEY_LEFT_STOP:
		case KEY_RIGHT_STOP:
			break;
	}
	//key_action_delay = KEY_ACTION_DELAY;
	Clear_Key_Buffer();
	key = NO_PRESS;
}
