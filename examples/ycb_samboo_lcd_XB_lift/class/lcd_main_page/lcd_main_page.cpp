#include "extern.h"
#include "lcd_main_page.h"
#include <string>
#include <cstdarg>
//#include <format> //c++20에서만 지원
//#include <iostream>
//#include "iwdg.h"
#include "gpio.h"

using namespace std;

LCD_MAIN *pLCD_MAIN;

int voltage_table[6]={12, 24, 36, 48, 60, 72};

LCD_MAIN::LCD_MAIN()
{
	color_dd_blue=pIli9488->rgb888torgb565(34, 61, 132);
}

LCD_MAIN::~LCD_MAIN()
{

}


void LCD_MAIN::initPage(void)
{

	Draw_Home();
//	pIli9488->ILI9488_DrawRGBImage(10,20,240,62,(uint8_t*)IMG_dd240x62);
//	pIli9488->ILI9488_DrawRGBImage(10,20,270,62,(uint8_t*)IMG_ycbrain_270x62);
}


//void LCD_MAIN::Update_Display_period(uint16_t Xpos, uint16_t Ypos, uint16_t period, uint16_t duration, uint16_t current)
//{
//
//}

void LCD_MAIN::Draw_graphic_battery_bar(uint8_t b1_cnt)
{

	uint8_t i;
	uint16_t bar_top=220;
	uint16_t bar_x0= GRAPHIC_BAR_X;
	uint16_t lbar_top=bar_top;
	uint16_t color;
	uint16_t batt_x0=bar_x0-20;
	lbar_top=bar_top+10;
//(1)BATTERY----------------------------------------------------


	for(i=0;i<4;i++){
		if(b1_cnt>i){
				color=LEVEL_NORMAL_COLOR;
		}
		else color=LEVEL_CLEAR_COLOR;
		pIli9488->ILI9488_Draw_Filled_Rectangle_Coord(batt_x0, lbar_top, batt_x0+GRAPHIC_BAR_WIDTH, lbar_top-GRAPHIC_BAR_PAINT_H, color);
		lbar_top-=GRAPHIC_BAR_TOTAL_H;
	}
}


void LCD_MAIN::Draw_graphic_JUHANG_bar(uint8_t b2_cnt, uint8_t b3_cnt, uint8_t jenhujin, uint8_t stop)
{

	uint8_t i;
	uint16_t bar_top=220;//-(GRAPHIC_BAR_TOTAL_H*4);
	uint16_t bar_x0= GRAPHIC_BAR_X;
	uint16_t bar_x1=bar_x0+80;
	uint16_t bar_x2=bar_x0+160;

	uint16_t lbar_top=bar_top;
	uint16_t color, lcolor, rcolor;

	color=LEVEL_CLEAR_COLOR;
	lcolor=(pDataClass->can_resp.flg_state.motor_dir1) ? MINJI_RED:MINJI_GREEN;
	rcolor=(pDataClass->can_resp.flg_state.motor_dir2) ? MINJI_RED:MINJI_GREEN;
//(Motor1)---------------------------------------------------
	lbar_top=bar_top;
	for(i=0;i<4;i++){
		if(b2_cnt>i){
				color=lcolor;
		}
		else color=LEVEL_CLEAR_COLOR;
		pIli9488->ILI9488_Draw_Filled_Rectangle_Coord(bar_x1, lbar_top, bar_x1+GRAPHIC_BAR_WIDTH, lbar_top-GRAPHIC_BAR_PAINT_H, color);
		lbar_top-=GRAPHIC_BAR_TOTAL_H;
	}
//(Motor2)---------------------------------------------------
//	lbar_top=bar_top;
//	for(i=0;i<4;i++){
//		if(b3_cnt>i){
//				color=rcolor;
//		}
//		else color=LEVEL_CLEAR_COLOR;
//		pIli9488->ILI9488_Draw_Filled_Rectangle_Coord(bar_x2, lbar_top, bar_x2+GRAPHIC_BAR_WIDTH, lbar_top-GRAPHIC_BAR_PAINT_H, color);
//		lbar_top-=GRAPHIC_BAR_TOTAL_H;
//	}
}


void LCD_MAIN::Draw_graphic_dd_bar(uint8_t max_cnt, uint8_t leap_cnt)
{

	uint8_t i;
	uint16_t bar_top=220;//-(GRAPHIC_BAR_TOTAL_H*4);

	//uint16_t bar_x0= GRAPHIC_BAR_X;
	uint16_t bar_x3=GRAPHIC_BAR_X+240;
	uint16_t bar_x4=GRAPHIC_BAR_X+320;

	uint16_t lbar_top=bar_top;
	uint16_t color;

//(4)------------------------------------------------------
	lbar_top=bar_top;
	for(i=0;i<4;i++){
		if(max_cnt>i){
				color=TFT_ORANGE;
		}
		else color=LEVEL_CLEAR_COLOR;
		pIli9488->ILI9488_Draw_Filled_Rectangle_Coord(bar_x3, lbar_top, bar_x3+GRAPHIC_BAR_WIDTH, lbar_top-GRAPHIC_BAR_PAINT_H, color);
		lbar_top-=GRAPHIC_BAR_TOTAL_H;
	}
//(5)------------------------------------------------------
	lbar_top=bar_top;
	for(i=0;i<4;i++){
		if(leap_cnt>i){
			color=TFT_PINK;//LEVEL_NORMAL_COLOR;
		}
		else color=LEVEL_CLEAR_COLOR;
		pIli9488->ILI9488_Draw_Filled_Rectangle_Coord(bar_x4, lbar_top, bar_x4+GRAPHIC_BAR_WIDTH, lbar_top-GRAPHIC_BAR_PAINT_H, color);
		lbar_top-=GRAPHIC_BAR_TOTAL_H;
	}
}


void LCD_MAIN::Draw_TopMenu_seljung(){
	pIli9488->ILI9488_Draw_Rectangle(MAIN_X, MAIN_Y, 480, 320, gBG_COLOR);
	Navi_Draw_Sub(0);
}

void LCD_MAIN::Draw_BattOutline(uint16_t xpos, uint16_t ypos){
	uint16_t bat_x, bat_y;
	bat_x=xpos;
	bat_y=ypos+40-10;
	pIli9488->ILI9488_Draw_HLineW(bat_x+20, bat_y-10, 38, 10, color_dd_blue);
	pIli9488->Draw_Hollow_Rectangle_Coord(bat_x, bat_y, bat_x+78, bat_y+68, color_dd_blue);
}

void LCD_MAIN::Draw_HBattOutline(uint16_t xpos, uint16_t ypos){
	uint16_t bat_x, bat_y;
	bat_x=xpos;
	bat_y=ypos+40-10;
	pIli9488->ILI9488_Draw_HLineW(bat_x+20, bat_y-10, 38, 10, color_dd_blue);
	pIli9488->Draw_Hollow_Rectangle_Coord(bat_x, bat_y, bat_x+78, bat_y+68, color_dd_blue);
}

void LCD_MAIN::Draw_TopMenu_Form()
{
	char buf[32]={0,};
	uint16_t xpos=0,ypos,bat_x, bat_y;
	ypos=20;
	xpos=GRAPHIC_BAR_X+220;
	pIli9488->ILI9488_Draw_HLineW(BAR_X, BAR_Y+10, BAR_W, BAR_H, BAR_COLOR);
///BATT Draw OutLine ++
	Draw_BattOutline(BAR_X, BAR_Y);
	//Draw_BattOutline(300, 10);
	//bat_x=BAR_X;
	//bat_y=BAR_Y+40-10;
	//pIli9488->ILI9488_Draw_HLineW(bat_x+20, bat_y-10, 38, 10, color_dd_blue);
	//pIli9488->Draw_Hollow_Rectangle_Coord(bat_x, bat_y, bat_x+78, bat_y+68, color_dd_blue);
//BATT Draw OutLine --
	pIli9488->ILI9488_DrawRGBImage(20,2,230,53,(uint8_t*)samboo_230x53);//samboo_230x53
	//pIli9488->ILI9488_DrawRGBImage(300,5,140,138,(uint8_t*)IMG_DL5000_140x138);
	string str=format("V%.1f %s%s",(float)REV_NUM/10.0f, __DATE__, __TIME__);
	pIli9488->draw16_string(260+12, TOP_SELJUNG_POS_Y1+30, MINJI_GRAY, MAIN_BG_COLOR, (char*)str.c_str(), 1, 0);
}

uint8_t LCD_MAIN::bar_step_calcuration(uint16_t value)
{
	uint8_t step=0;
	if(value>800)step=4;
	else if(value>512)step=3;
	else if(value>256)step=2;
	else if(value>100)step=1;
	else step=0;
	return step;
}

BATTERY_SOC LCD_MAIN::battery_step_calcuration(uint16_t volt_inx,uint16_t value)
{
	uint8_t step=0;
	BATTERY_SOC soc;
	float fValue;
	int i=0;
	float batt_table[21]={
		BATTERY24_100,//0
		BATTERY24_095,//1
		BATTERY24_090,//2
		BATTERY24_085,//3
		BATTERY24_080,//4
		BATTERY24_075,//5
		BATTERY24_070,//6
		BATTERY24_065,//7
		BATTERY24_060,//8
		BATTERY24_055,//9
		BATTERY24_050,//10
		BATTERY24_045,//11
		BATTERY24_040,//12
		BATTERY24_035,//13
		BATTERY24_030,//14
		BATTERY24_025,//15
		BATTERY24_020,//16
		BATTERY24_015,//17
		BATTERY24_010,//18
		BATTERY24_005,//19
		BATTERY24_000//20
	};
	soc.step=0;
	soc.soc=0;

	//value=(BATTERY24_030*20)+1;

	fValue=value*(24.0f/voltage_table[volt_inx]);
	for(i=0;i<21;i++){
		if((fValue/10.0f)>batt_table[i]) break;
	}

	if(i>20)i=20;
	soc.soc=(20-i)*5;
	soc.step=soc.soc/25;//MAX 57.6V
	return soc;
}

BATTERY_SOC LCD_MAIN::nabsan_battery_step_calcuration(uint16_t value)
{
	//uint8_t step=0;
	BATTERY_SOC soc;
	int i=0;
	uint8_t percent[9]={100,90,75,60,50,40,25,10,0};
	uint8_t step[9]=   {  4, 4, 3, 3, 2, 2, 1, 1,0};
	float batt_table[9]={
	 BATTERY12_100, BATTERY12_090, BATTERY12_075, BATTERY12_060, BATTERY12_050, BATTERY12_040, BATTERY12_025, BATTERY12_010	, BATTERY12_000};
	soc.step=0;	soc.soc=0;
	for(i=0;i<9;i++){
		if((value/20.0f)>batt_table[i]) break;
	}
	if(i>=9){soc.step=0;soc.soc=0;}
	else{
		soc.soc=percent[i];
		soc.step=step[i];
	}
	return soc;
}

void LCD_MAIN::Update_Wheel_icon(uint8_t toggle_sw, uint8_t stop)
{
	uint16_t LxPos=370,RxPos=430,yPos=16,Xwidth=20, Ywidth=30;
	uint8_t lcor=0,rcor=0;
	uint16_t color_table[3]={TFT_DARKGREY, MINJI_GREEN, MINJI_RED};
	if(toggle_sw==0){
		lcor=rcor=0;
	}
	else{
		switch(stop){
		case 0://no stop
			switch(toggle_sw){
				case 1:	lcor=rcor=1; break;
				case 2:	lcor=rcor=2;break;
			}
			break;
		case 1://left stop
			switch(toggle_sw){
				case 1:	lcor=0;rcor=1; break;
				case 2:	lcor=0;rcor=2;break;
			}
			break;
		case 2://right stop
			switch(toggle_sw){
				case 1:	lcor=1;rcor=0; break;
				case 2:	lcor=2;rcor=0; break;
			}
			break;
		case 3://all stop
			lcor=rcor=0;
			break;
		default:break;
		}
	}
	pIli9488->ILI9488_Draw_Filled_Rectangle_Coord(LxPos, yPos, LxPos+Xwidth, yPos+Ywidth, color_table[lcor]);
	pIli9488->ILI9488_Draw_Filled_Rectangle_Coord(RxPos, yPos, RxPos+Xwidth, yPos+Ywidth, color_table[rcor]);
}

void LCD_MAIN::Draw_JenHujin_toggle(uint8_t toggle){

	uint16_t Xpos_str_JENHUJIN_SW=20;
	uint16_t Ypos_str_JENHUJIN_SW=108;
	uint16_t len,lcolor=0;
	string str="";
	string str_state[4]={"정지","전진","후진",	""};
	switch(toggle){
		case 0:
			lcolor=MINJI_GRAY;
			break;
		case 1:
			lcolor=MINJI_GREEN;
			break;
		case 2:
			lcolor=MINJI_RED;
			break;
	}
	len=pIli9488->draw16_string(Xpos_str_JENHUJIN_SW, Ypos_str_JENHUJIN_SW, lcolor,  gBG_COLOR, (char*)str_state[toggle].c_str(),2, 0);
	//str=format(" %d%%  ",calcu_percent(pDataClass->vcu_sdu.orgspeed));
	//pIli9488->draw16_string(Xpos_str_JENHUJIN_SW+len, Ypos_str_JENHUJIN_SW, MINJI_GRAY,  gBG_COLOR, (char*)str.c_str(),2, 0);
}

uint8_t  LCD_MAIN::calcu_percent(int value)
{
	uint8_t percent=0;
	percent=value/10;
	if(percent>99) percent=100;
	return percent;
}

void  LCD_MAIN::Draw_Information(uint16_t xpos, uint16_t ypos)
{
	uint8_t info_idx=0;
	uint16_t Xpos_str_info=xpos;
	uint16_t Ypos_str_info=ypos;
	int16_t can_state_color=TFT_LIGHTGREY;
	string str_info[8]={
			"안전 운행    ",
			"충전 하세요  ",
			"과열 입니다  ",
			"통신 장애   ",
			"비상 정지   "
	};
	//if(batttery_inx==1)
	if(exBattVoltage<(BATTERY12_040*20)) info_idx=1;//40%미만일때

	if(pDataClass->over_heating) info_idx=2;
	if(!pDataClass->sysFlag.canReady)info_idx=3;
	if(pDataClass->recv_can_emergency)info_idx=4;
	if(pDataClass->sysFlag.bisang|| pDataClass->recv_can_emergency)info_idx=4;
	printf("-------info_idx[%d] Vavr[%d]\r\n",info_idx , exBattVoltage);
	switch(info_idx){
	case 0:
		pIli9488->draw16_string(Xpos_str_info, Ypos_str_info, TFT_DARKGREY,  gBG_COLOR, (char*)str_info[info_idx].c_str(),2, 0);
		break;
	case 1:
		if(pDataClass->toggle)	pIli9488->draw16_string(Xpos_str_info, Ypos_str_info, TFT_RED,  gBG_COLOR, (char*)str_info[info_idx].c_str(),2, 0);
		else pIli9488->draw16_string(Xpos_str_info, Ypos_str_info, TFT_DARKGREY,  gBG_COLOR, (char*)str_info[info_idx].c_str(),2, 0);
		break;
	case 2:
		if(pDataClass->toggle) pIli9488->draw16_string(Xpos_str_info, Ypos_str_info, TFT_RED,  gBG_COLOR, (char*)str_info[info_idx].c_str(),2, 0);
		else pIli9488->draw16_string(Xpos_str_info, Ypos_str_info, TFT_DARKGREY,  gBG_COLOR, (char*)str_info[info_idx].c_str(),2, 0);
		break;
	case 3:
	case 4:
		if(pDataClass->toggle) pIli9488->draw16_string(Xpos_str_info, Ypos_str_info, TFT_RED,  gBG_COLOR, (char*)str_info[info_idx].c_str(),2, 0);
		else pIli9488->draw16_string(Xpos_str_info, Ypos_str_info, TFT_DARKGREY,  gBG_COLOR, (char*)str_info[info_idx].c_str(),2, 0);
		break;
	}
	//if(pZIGBEE->zigbee_remain) zigbee_state_color=MINJI_GREEN;
	if(pCAN->canAliveTimeOut) can_state_color=TFT_BLUE_SKY;
	pIli9488->Draw_Filled_Circle(20, TOP_SELJUNG_POS_Y1+30+8, 5, can_state_color);
}

void LCD_MAIN::Update_Display_status()
{
	uint8_t B1,B2,max_cnt,leap_cnt,percent;
	uint16_t Vavr;
	uint8_t mJenhujin, mStop_lr,right_idx=0, baju_dir_idx,info_idx;
	int16_t speed1,speed2,limit_speed;
	int16_t speed1_percent, limit_percent;
	int16_t len;
	string str="";
	uint8_t batttery_inx=1;
	pDataClass->setup_data.system_conf1.battery_voltage=batttery_inx;
///////////////////////////////////////////////
	uint16_t Xpos_str_state=20;
	uint16_t Ypos_str_state=68;
	string str_state[12]={
			"정지      ",//cART_STOP
			"전방 주행 ",//cART_JENJIN
			"후방 주행 ",//cART_HUJIN
			"전방좌회전 ",//cART_JENJIN_LEFT_TURN
			"전방우회전",//cART_JENJIN_RIGHT_TURN
			"후방좌회전",//cART_HUJIN_LEFT_TURN
			"후방우회전",//cART_HUJIN_RIGHT_TURN
			"Non Spin",//cART_NONE_SPIN
			"좌방향회전",//cART_LEFT_SPIN
			"우방향회전",//cART_RIGHT_SPIN
			"비상정지",//cART_BISANG
			""//
	};
///////////////////////////////////////////////
	//리프트 위치
	uint16_t Xpos_str_function=240;
	uint16_t Ypos_str_function=68;

////////////////////////////////////////////////
//	uint16_t Xpos_str_info=180;
	uint16_t Ypos_str_info=108;
//////////////////////////////////////////////

	string strSpeedLeft[3]={"정지","전진","후진"};
	string strSpeedRight[3]={"정지","전진","후진"};
	string strLimit={"제한"};
	string strLeapTime={"Leap"};
	string strFET_temp =format("%03d/%03d",pDataClass->can_resp.fet_temp, pDataClass->can_resp.motor_temp);
	string strMOTOR_rpm =format("rpm:%03d",pDataClass->can_resp.motor_rpm);

	mJenhujin=pDataClass->vcu_sdu.toggle.b.jenhujin;
	mStop_lr=pDataClass->vcu_sdu.toggle.b.stop_lr;
	speed1=pDataClass->can_resp.ex_pwm1*10;
	speed2=pDataClass->can_resp.ex_pwm2*10;
	limit_speed=pDataClass->limit_speed;

	speed1_percent=pDataClass->can_resp.ex_pwm1;//calcu_percent(speed1);
//	speed2_percent=pDataClass->can_resp.ex_pwm2;//calcu_percent(speed2);
	limit_percent=calcu_percent(limit_speed);

//Vavr=pDataClass->get_battery_filter(pDataClass->can_resp.BatteryVolt);
//battery voltage++
	BattVoltage=pDataClass->can_resp.BatteryVolt;
	if(BattVoltage<exBattVoltage)Vavr =BattVoltage;
	else if(BattVoltage>(exBattVoltage+10))Vavr =BattVoltage;
	else Vavr =exBattVoltage;

	exBattVoltage=Vavr;//BattVoltage;
//battery voltage--
	if(Vavr>1000)Vavr=0;

	//batt_soc=battery_step_calcuration(pDataClass->setup_data.system_conf1.battery_voltage, Vavr);
//#### voltage_index=1;//24V 밧데리 고정  ++
	pDataClass->setup_data.system_conf1.battery_voltage=1;
	batt_soc=battery_step_calcuration(pDataClass->setup_data.system_conf1.battery_voltage, Vavr);
//#### voltage_index=1;//24V 밧데리 고정 --
//	printf("volt[%d] soc[%d] step[%d]\r\n",pDataClass->gMAIN.battery_voltage, batt_soc.soc, batt_soc.step);

	B1=bar_step_calcuration(speed1);
	B2=bar_step_calcuration(speed2);
	max_cnt=bar_step_calcuration(limit_speed);

	Draw_graphic_battery_bar(batt_soc.step);
	Draw_graphic_JUHANG_bar(B1, B2, mJenhujin, mStop_lr);
	//Draw_graphic_dd_bar(max_cnt, leap_cnt);

	pIli9488->draw16_string(GRAPHIC_BAR_X+(80*1), TOP_SELJUNG_POS_Y,  TFT_DARKGREY,  gBG_COLOR, (char*) strSpeedLeft[mJenhujin].c_str(),2, 0);//밑에 정지
	//pIli9488->draw16_string(GRAPHIC_BAR_X+(80*2), TOP_SELJUNG_POS_Y,  color_dd_blue,  gBG_COLOR, (char*) strSpeedRight[mJenhujin].c_str(),2, 0);
	//pIli9488->draw16_string(GRAPHIC_BAR_X+(80*3), TOP_SELJUNG_POS_Y,  color_dd_blue,  gBG_COLOR, (char*) strLimit.c_str(),2, 0);
	//pIli9488->draw16_string(GRAPHIC_BAR_X+(80*4), TOP_SELJUNG_POS_Y,  color_dd_blue,  gBG_COLOR, (char*) strLeapTime.c_str(),2, 0);

	str=format("%02d%%  ",batt_soc.soc);
	pIli9488->draw16_string(GRAPHIC_BAR_X+(80*0)-10, TOP_SELJUNG_POS_Y1-28,MINJI_GREEN,  gBG_COLOR, (char*)str.c_str(),2, 0);
	str=format("%.1fV  ",Vavr/10.0f);
	pIli9488->draw16_string(GRAPHIC_BAR_X+(80*0)-30, TOP_SELJUNG_POS_Y1,  TFT_DARKGREY,  gBG_COLOR, (char*)str.c_str(),2, 0);
/////
	pIli9488->draw16_string(GRAPHIC_BAR_X+(80*0), TOP_SELJUNG_POS_Y1+30,  TFT_DARKGREY, gBG_COLOR, (char*)strMOTOR_rpm.c_str(),1, 0);
	pIli9488->draw16_string(GRAPHIC_BAR_X+(80*1), TOP_SELJUNG_POS_Y1+30,  TFT_DARKGREY, gBG_COLOR, (char*)strFET_temp.c_str(),1, 0);

	str=format(" %d%%  ",calcu_percent(pDataClass->vcu_sdu.orgspeed));
	pIli9488->draw16_string(GRAPHIC_BAR_X+(80*2), TOP_SELJUNG_POS_Y1+30, TFT_DARKGREY,  gBG_COLOR, (char*)str.c_str(),1, 0);


	str=format("%d%%  ",speed1_percent);
	pIli9488->draw16_string(GRAPHIC_BAR_X+(80*1), TOP_SELJUNG_POS_Y1,  MINJI_RED,  gBG_COLOR, (char*)str.c_str(),2, 0);


	len=pIli9488->draw16_string(20, Ypos_str_info,  MINJI_RED,  gBG_COLOR, (char*)"Max ",2, 0);
	str=format("%3d%%",limit_percent);
	len=pIli9488->draw16_string(20+len, Ypos_str_info,  MINJI_GREEN,  gBG_COLOR, (char*)str.c_str(),2, 0);


//===============================
//----lift---
	Xpos_str_function=240;
	Ypos_str_function=170;

	if(pDataClass->my_toggle.b.jenhujin)len=pIli9488->draw16_string(Xpos_str_function, Ypos_str_function, MINJI_GREEN,  gBG_COLOR, (char*)"찔끔 ",2, 0);
	else len=pIli9488->draw16_string(Xpos_str_function, Ypos_str_function, TFT_DARKGREY,  gBG_COLOR, (char*)"찔끔 ",2, 0);

	if(pDataClass->Leap_TimeOut>0)
		str=format("%2.1f/%2.1f",pDataClass->Leap_TimeOut/10.0f, pDataClass->leap_time/10.0f);
	else
		str=format("%2.1f/%2.1f",pDataClass->leap_time/10.0f, pDataClass->leap_time/10.0f);
	//pIli9488->draw16_string(GRAPHIC_BAR_X+(80*4)-10, TOP_SELJUNG_POS_Y1,  TFT_PINK,  gBG_COLOR, (char*)str.c_str(),2, 0);

	len=pIli9488->draw16_string(Xpos_str_function+len, Ypos_str_function, MINJI_GREEN,  gBG_COLOR, (char*)str.c_str(),2, 0);

	Ypos_str_function=Ypos_str_function+40;
	len=pIli9488->draw16_string(Xpos_str_function, Ypos_str_function, MINJI_GREEN,  gBG_COLOR, (char*)"리프트 ",2, 0);
	switch(pDataClass->my_toggle.b.lift1_ud){
		case 0:
			pIli9488->draw16_string(Xpos_str_function+len, Ypos_str_function, TFT_DARKGREY,  gBG_COLOR, (char*)"중립  ",2, 0);
			break;
		case 1:
			pIli9488->draw16_string(Xpos_str_function+len, Ypos_str_function, MINJI_RED,  gBG_COLOR, (char*)"상승  ",2, 0);
			break;
		case 2:
			pIli9488->draw16_string(Xpos_str_function+len, Ypos_str_function, MINJI_RED,  gBG_COLOR, (char*)"하강  ",2, 0);
			break;
	}

//	Xpos_str_function=300;
	Ypos_str_function=Ypos_str_function+40;
	pIli9488->draw16_string(Xpos_str_function, Ypos_str_function, MINJI_GREEN,  gBG_COLOR, (char*)"캐스터 ",2, 0);
	switch(pDataClass->my_toggle.b.lift2_ud){
		case 0:
			pIli9488->draw16_string(Xpos_str_function+len, Ypos_str_function, TFT_DARKGREY,  gBG_COLOR, (char*)"중립  ",2, 0);
			break;
		case 1:
			pIli9488->draw16_string(Xpos_str_function+len, Ypos_str_function, MINJI_RED,  gBG_COLOR, (char*)"상승  ",2, 0);
			break;
		case 2:
			pIli9488->draw16_string(Xpos_str_function+len, Ypos_str_function, MINJI_RED,  gBG_COLOR, (char*)"하강  ",2, 0);
			break;
	}
//===============================
	Draw_Information(20, 68);
}


void LCD_MAIN::Footer_Message(char *msg, int x, int y, int width, uint16_t color)
{
/*	int str_len=pIli9488->draw16_length((char*)msg, 2);
	pIli9488->ILI9488_Draw_HLineW(x+20+str_len, y-5, width-str_len, 32, FOOTER_BG_COLOR);//앞지우기//FOOTER_BG_COLOR//TFT_YELLOW
	pIli9488->draw16_string(x+20, y-5, color, FOOTER_BG_COLOR, (char*)msg, 2, 0);*/
}


void LCD_MAIN::Draw_Home()
{
	printf("+++++++Draw_Home+++++++++++\r\n");
	pIli9488->ILI9488_Draw_Rectangle(MAIN_X, MAIN_Y, 480, 320, gBG_COLOR);
	if(pSTM32_IO->navi_state.system_setup) Draw_TopMenu_seljung();
	else Draw_TopMenu_Form();
}


//순차적으로 디스플레이 에러 메세지
void LCD_MAIN::Update_StatusMessage(){
	return;
/*	typedef struct _STATUS
	{
		string  str_id;
		uint16_t color_id;
	}STATUS;
	vector<STATUS> vStatusStr;
	STATUS mStatus;
	uint16_t color;
	//-----------      0          1               2             3            4            5             6         7	         8	      9		  10         11            12        13
	string str[14] =
			{ "경농산업", "히터가동", "이상고온오류", "이상저온오류", "항온동작중", "센서통신이상", "문열림",
					"HTC이상", "OCR이상", "경농산업","제상중","콤프전류이상","팬전류이상","히터전류이상"};
	vStatusStr.clear();

	//if(pDataClass->reg.relay[HEATER_RY].LED)		   { mStatus.str_id=str[1]; mStatus.color_id=TFT_YELLOW;vStatusStr.push_back(mStatus);}
	if(pDataClass->reg.sysFlag.b.HIGH_TEMPER_ALARM_ON) { mStatus.str_id=str[2]; mStatus.color_id=TFT_YELLOW;vStatusStr.push_back(mStatus);}
	if(pDataClass->reg.sysFlag.b.LOW_TEMPER_ALARM_ON)  { mStatus.str_id=str[3]; mStatus.color_id=TFT_WHITE;vStatusStr.push_back(mStatus);}
	if(pDataClass->reg.sysFlag.b.SUPER_COOL_HANGON)    { mStatus.str_id=str[4]; mStatus.color_id=TFT_YELLOW;vStatusStr.push_back(mStatus);}
	if(pDataClass->reg.sysFlag.b.SENSOR_COMM_ERROR)    { mStatus.str_id=str[5]; mStatus.color_id=TFT_RED;vStatusStr.push_back(mStatus);}
	if(pDataClass->reg.sysFlag.b.CHANGGO_OPEN_DOOR)    { mStatus.str_id=str[6]; mStatus.color_id=TFT_WHITE;vStatusStr.push_back(mStatus);}
	if(pDataClass->bGPIO_HTC)			               { mStatus.str_id=str[7]; mStatus.color_id=TFT_WHITE;vStatusStr.push_back(mStatus);}
	if(pDataClass->bOCR_ERROR)						   { mStatus.str_id=str[8]; mStatus.color_id=TFT_WHITE;vStatusStr.push_back(mStatus);}

	if(pDataClass->reg.relay[COMP_RY].error)       	   { mStatus.str_id=str[11]; mStatus.color_id=TFT_WHITE;vStatusStr.push_back(mStatus);}
	if(pDataClass->reg.relay[EVAFAN_RY].error)         { mStatus.str_id=str[12]; mStatus.color_id=TFT_WHITE;vStatusStr.push_back(mStatus);}
	if(pDataClass->reg.relay[HEATER_RY].error)         { mStatus.str_id=str[13]; mStatus.color_id=TFT_WHITE;vStatusStr.push_back(mStatus);}

	//printf("@@@@@[0]vStatusStr.size()[%d]\r\n",vStatusStr.size());
	//printf("\r\n@@@@@[0]vStatusStr.size()[%d]\r\n",vStatusStr.size());
	if(vStatusStr.size()==0){
		mStatus.str_id=str[0]; mStatus.color_id=TFT_GREEN;vStatusStr.push_back(mStatus);
	}
	Update_StatusMessage_cnt %= vStatusStr.size();
	string dis_str=vStatusStr[Update_StatusMessage_cnt].str_id;
	color=vStatusStr[Update_StatusMessage_cnt].color_id;//ALARM_STRING_NORMAL_COLOR;
	Footer_Message((char*)dis_str.c_str(), 15, 280, 210, color);
	Update_StatusMessage_cnt++;*/
}

//void LCD_MAIN::Update_StateMark()
//{
//	//uint16_t modem_state_color=TFT_LIGHTGREY;
//	uint16_t zigbee_state_color=TFT_LIGHTGREY;
//	int len,xpos;
//	char buf[32]={0,};
//	//printf("Build: %s %s\n", __DATE__, __TIME__);
//	sprintf(buf,"Rev %.1f %s %s",(float)REV_NUM/10.0f,  __DATE__, __TIME__);
//	if(pZIGBEE->zigbee_remain) zigbee_state_color=MINJI_GREEN;
//	pIli9488->Draw_Filled_Circle(300, 10, 5, zigbee_state_color);
//	len=pIli9488->draw16_string(300+12, 2, MINJI_GRAY, MAIN_BG_COLOR, (char*)buf, 1, 0);
//
//}

ITEM_DISPLAY_STRING LCD_MAIN::Navi_MENU_updn_make_string(uint8_t sub, uint8_t item)
{
	ITEM_DISPLAY_STRING dis_str;
	MENU_ITEM_STRING menu_string;

	//MENU_DATA menu_value;
	uint16_t menu_value[16];

	int posA, posB, posC, iA,iB,iC;
	int sub_menu_cnt = UI_Info.sub_menu_cnt[sub];
	posA = (item > 0) ? item - 1 : sub_menu_cnt - 1; //
	posB = item;
	posC = ((item + 1) >= sub_menu_cnt) ? 0 : item + 1; //

	dis_str=mk_Item_string(sub, posA,posB, posC);

	memcpy(&menu_string,&UI_Info.value_arry[sub],sizeof(menu_string));
	if(sub==0)	memcpy(menu_value,&pDataClass->setup_data.system_conf1, sizeof(SYSTEM_CONF1));
	else if(sub==1)	memcpy(menu_value,&pDataClass->setup_data.system_conf2,sizeof(SYSTEM_CONF2));

	if(sub==0){//battery voltage display
		menu_value[1]=voltage_table[menu_value[1]];
	}

	iA=	menu_value[posA];
	iB=	menu_value[posB];
	iC=	menu_value[posC];

	sprintf(dis_str.A_val, menu_string.str[posA],iA);
	sprintf(dis_str.B_val, menu_string.str[posB],iB);
	sprintf(dis_str.C_val, menu_string.str[posC],iC);

#if 0
 	printf("item[%d] ##POS A:%d B:%d C:%d\r\n",item, posA, posB, posC);
	printf("item[%d] ##STR A:%s B:%s C:%s\r\n",item, dis_str.A_str, dis_str.B_str, dis_str.C_str);
	printf("item[%d] VALUE A:%s B:%s C:%s\r\n",item, dis_str.A_val, dis_str.B_val, dis_str.C_val);
#endif
	return dis_str;
}


void LCD_MAIN::Navi_Draw_Item_ValueUpdate(NAVI_BIT navi, int16_t position)
{

	uint8_t sub, item;
	int value,lvalue;
	uint16_t menu_value[16];
	ITEM_DISPLAY_STRING dis_str;
	int ypos;
	sub = navi.sub;
	item = navi.item;   //타이틀 건너뛰고
	ypos = Navi_Item_Ypos(item);   //item 0 --> 1에 그림

	if(sub==0)	memcpy(menu_value,&pDataClass->setup_data.system_conf1, sizeof(SYSTEM_CONF1));
	else memcpy(menu_value,&pDataClass->setup_data.system_conf2, sizeof(SYSTEM_CONF2));
	value = menu_value[item];
	//value = pDataClass->setup_data.data_arry[sub].idata[item];
	value=value+position;
	//if(abs(position)>1)value=(value/10)*10;

	printf("(xxxx)>>>Navi_Draw_Item sub[%d] item[%d] value[%d] position[%d]\r\n",	sub, item, value, position);

	switch (item)
	{
	case 0:
		break;
	case 1:
		switch(sub){
		case 0://Battery voltage
			if (value<0)value = 0;
			else if (value > 5)value = 5;
			break;
		case 1://tottle offset
			if (value<0)value = 0;
			else if (value > 500)value = 500;
			break;
		}
		//UI_Data.data_arry[sub].idata[item] = value;
	case 2:
		switch(sub){
		case 0://Limit Current
			if (value<0)value = 0;
			else if (value > 300)value = 300;
			break;
		case 1://tottle inflection
			if (value<0)value = 0;
			else if (value > 1000)value = 1000;
			break;
		}
		//UI_Data.data_arry[sub].idata[item] = value;
		//printf("(zzzzz)>>>Navi_Draw_Item sub[%d] item[%d] value[%d] position[%d]\r\n",	sub, item, value, position);
		break;
	case 3://트로틀옵셋
		switch(sub){
		case 0://Limit Motor temperature
			if (value<0)value = 0;
			else if (value > 150)value = 150;
			break;
		case 1://forward
			if (value<0)value = 0;
			else if (value > 100)value = 100;
			break;
		}
		//UI_Data.data_arry[sub].idata[item] = value;
		break;
	case 4://Accel
		switch(sub){
		case 0://Limit FETr temperature
			if (value<0)value = 0;
			else if (value > 150)value = 150;
			break;
		case 1://backward
			if (value<0)value = 0;
			else if (value > 100)value = 100;
			break;
		}
		//UI_Data.data_arry[sub].idata[item] = value;
	case 5:	//Decel
		switch(sub){
		case 0://Low Battery
			if (value<0)value = 0;
			else if (value > 80)value = 80;
			break;
		case 1://Accel
			if (value<0)value = 0;
			else if (value > 100)value = 100;
			break;
		}
		//UI_Data.data_arry[sub].idata[item] = value;
		break;
	case 6:
		switch(sub){
		case 0://cart type 0:궤도 1:바퀴
			if (value<0)value = 0;
			else if (value > 1)value = 1;
		case 1://Decel
			if (value<0)value = 0;
			else if (value > 100)value = 100;
			break;
		}
		//UI_Data.data_arry[sub].idata[item] = value;
		break;

	case 7:
		switch(sub){
		case 0://Motor1 Polarity
			if (value<0)value = 0;
			else if (value > 1)value = 1;
			break;
		case 1://Brake delay
			if (value<0)value = 0;
			else if (value > 600)value = 600;
			break;
		}
		//UI_Data.data_arry[sub].idata[item] = value;
		break;
	case 8: //브레이크지연(ms)
		switch(sub){
		case 0://Motor2 Polarity
			if (value<0)value = 0;
			else if (value > 1)value = 1;
			break;
		case 1://Brake rate
			if (value<0)value = 0;
			else if (value > 10)value = 10;
			break;
		}
		//UI_Data.data_arry[sub].idata[item] = value;
	default:
		break;
	}

	menu_value[item]=value;
	if(sub==0)	memcpy(&pDataClass->setup_data.system_conf1, menu_value, sizeof(SYSTEM_CONF1));
	else memcpy(&pDataClass->setup_data.system_conf2, menu_value, sizeof(SYSTEM_CONF2));

	//pDataClass->setup_data.data_arry[sub].idata[item] = value;
	dis_str = Navi_MENU_updn_make_string(sub, item);
	pIli9488->ILI9488_Draw_Filled_Rectangle_Coord(SET_VALUE_REF_X, ypos, 480,	ypos + 30, ITEM_BG_COLOR);
	pIli9488->draw16_string(SET_VALUE_REF_X, ypos, ITEM_SELECTED_COLOR, ITEM_BG_COLOR, (char*) dis_str.B_val, 2, 0);

}


void LCD_MAIN::NAVI_Draw_TopMode(NAVI_BIT navi)
{
	//DRAW_MUTEX = 1;
	//if (navi.system_setup==0) Draw_Home();
	//else Navi_Draw_Sub(navi);
	//DRAW_MUTEX = 0;
	Draw_Home();
}

void LCD_MAIN::Draw_Key_Event()
{
	NAVI_BIT navi=uiEvent.navi;
	MENU_DRAW_EVENT draw_event=uiEvent.draw_event;
	int16_t position=uiEvent.position;
	uiEvent.event_flag=0;

	switch (draw_event)
	{
	case KEY_HOME_SETUP_EVENT: //top_mode change
		//pFlashClass->SPI_FLASH_UPDATE_SELJUNG();//ppp 23.02.03
		NAVI_Draw_TopMode(navi);
		break;
	case KEY_TOP_MODE_SETUP_NEXT_EVENT: //change sub
		Navi_Draw_Sub(navi.sub);
		break;
	case KEY_TOP_MODE_ITEM_EVENT: //change sub
		Navi_Draw_Item(navi.sub, navi.item, 1, 1);
		break;
	case KEY_ITEM_VALUE_UPDN_EVENT:
		Navi_Draw_Item_ValueUpdate(navi, position);
		break;
	case KEY_SELJUNG_UPDN_EVENT: //SetTemperature
		//Update_Display_Seljung((TOP_MENU) navi.top_title);
		//pFlashClass->SPI_FLASH_UPDATE_SELJUNG();//ppp 23.02.03
		break;
	case KEY_TOP_MENU_CHANGE_EVENT:
		//Draw_Home((TOP_MENU) navi.top_title);
		//pFlashClass->SPI_FLASH_UPDATE_SELJUNG();//ppp 23.02.03
		break;
	default:
		break;

	}
	//pFlashClass->SPI_FLASH_UPDATE_SELJUNG(); //ppp 23.02.03
}

void LCD_MAIN::NAVI_ChangeMode(NAVI_BIT navi, MENU_DRAW_EVENT draw_event, int16_t position)
{
	uiEvent.navi=navi;
	uiEvent.draw_event=draw_event;
	uiEvent.position=position;
	uiEvent.event_flag=1;
}

void LCD_MAIN::update_DrawMainMenu(uint8_t update)
{
	switch(update){
	case 0:
		Update_Display_status();
		break;
	case 1:
#ifdef SENSOR
		Update_Display_measured();
#endif
		break;

	}

//	if (update == 0)
//	{
//		//Update_Icon((TOP_MENU) pSTM32_IO->navi_state.top_title);
//		Update_StatusMessage();
//		//Draw_nick_name();
//	}

}


unsigned int LCD_MAIN::draw7Number(long n, unsigned int xLoc, unsigned int yLoc, char cS, unsigned int fC, unsigned int bC, char nD, uint8_t dash) {
	unsigned int num = abs(n), i, s, t, w, col, h, x, y, si = 0, j = 1, d = 0;
	unsigned int S2 = 5 * cS;                     // width of horizontal segments   5 times the cS
	unsigned int S3 = 2 * cS;                     // thickness of a segment 2 times the cs
	unsigned int S4 = 5 * cS;                     // height of vertical segments 7 times the cS
	unsigned int x1 = cS + 1;                     // starting x location of horizontal segments
	unsigned int x2 = S3 + S2 + 1;                // starting x location of right side segments
	unsigned int y1 = yLoc + x1;                  // starting y location of top side segments
	unsigned int y3 = yLoc + S3 + S4 + 1;         // starting y location of bottom side segments
	// actual x,y locations and direction of all 7 segments
	unsigned int seg[7][2] = {{x1, yLoc}, {x2, y1}, {x2, y3 + x1}, {x1, (2 * y3) - yLoc}, {0, y3 + x1}, {0, y1}, {x1, y3}};
	// segment defintions for all 10 numbers plus blank and minus sign bit mapped as typical segment labels (*,g,f,e,d,c,b,a)
	static const unsigned char nums[12] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67, 0x00, 0x40};
	unsigned char c, cnt, lcnt;
	unsigned int lfC,lbC;

	c = abs(cS);                                  // get character size between 1 and 10 ignoring sign
	if (c>10) c= 10;
	if (c<1) c = 1;

	cnt = abs(nD);                                // get number of digits between 1 and 10 ignoring sign
	lcnt=cnt;
	if (cnt > 10) cnt = 10;
	if (cnt < 1) cnt = 1;

	d = S2+(3*S3)+2+cS;                              // width of one number
	xLoc += (cnt-1) * d;                          // set starting x at last digit location

	while( cnt > 0) {                             // for cnt number of places
		--cnt;
		lfC=fC; lbC=bC;
		if (num > 9) i = num%10;                    // get the last digit
		else if (!cnt && n<0) i = 11;               // show minus sign if 1st position and negative number
		else if (nD < 0 && !num) i = 10;            // show blanks if remaining number is zero
		else i = num;

		num = num/10;                               // trim this digit from the number
		if(dash>0)i=11;
//printf("lcnt[%d] cnt[%d] i[%d]\r\n",lcnt,cnt, i);
//if(wozero){	if(lcnt>1 && cnt==0 && i==0) lfC=lbC;}
		for (j = 0; j < 7; ++j) {                   // draw all seven segments

			if (nums[i] & (1 << j)) col = lfC;         // if segment is On use foreground color
			else col = lbC;                            // else use background color

			if (j==0 || j==3 || j==6) {

				w = S2;                                 // Starting width of segment (side)
				x = xLoc + seg[j][0] + cS;              // starting x location
				y = seg[j][1];                          // starting y location
				t = y + S3;                             // maximum thickness of segment
				h = y + S3/2;                           // half way point thickness of segment


				while (y < h) {                         // until y location = half way
					pIli9488->ILI9488_Draw_Horizontal_Line(x, y, w, col);      // Draw a horizontal segment top
					++y;                                  // move the y position by 1
					--x;                                  // move the x position by -1
					w += 2;                               // make the line wider by 2
				}

				while (y < t) {                         //finish drawing horizontal bottom
					pIli9488->ILI9488_Draw_Horizontal_Line(x, y, w, col);      // Draw Horizonal segment bottom
					++y;                                  // keep moving the y or y draw position until t
					++x;                                  // move the length or height starting position back the other way.
					w -= 2;                               // move the length or height back the other way
				}

			} else {

				w = S4;                                 // Starting height of segment (side)
				x = xLoc + seg[j][0];                   // starting x location
				y = seg[j][1] + cS;                     // starting y location
				t = x + S3;                             // maximum thickness of segment
				h = x + S3/2;                           // half way point thickness of segment

				while (x < h) {                         // until x location = half way
					pIli9488->ILI9488_Draw_Vertical_Line(x, y, w, col);      // Draw a vertical line right side
					++x;                                  // move the x position by 1
					--y;                                  // move the y position by -1
					w += 2;                               // make the line wider by 2
				}
				while (x < t) {
					pIli9488->ILI9488_Draw_Vertical_Line(x, y, w, col);      // Draw a vertical line right side
					++x;                                  // move the x position by 1
					++y;                                  // move the length or height starting position back the other way.
					w -= 2;                               // move the length or height back the other way
				}
			}
		}
		xLoc -=d;                                   // move to next digit position
	}
	return (d*nD);
}

uint16_t LCD_MAIN::draw_humidity_2FND(int n, uint16_t x, uint16_t y, uint16_t cs, uint16_t fc,  uint16_t bc){
	uint16_t xpos, ypos;
	n= n/10;
	if(n<0)n=0;
	else if(n>99)n=99;
	xpos=x+(7*cs);
	ypos=y+(7*cs);
	draw7Number(n, xpos, y, cs, fc, bc, 2, 0);
	xpos=x+(7*cs)+(11*cs*2);
	return xpos;
}

uint16_t LCD_MAIN::draw_3FND(int n, int x, uint16_t y, uint16_t cs, uint16_t fc,  uint16_t bc, uint8_t dash){
	//n=-17;
	uint8_t digit;
	uint8_t adj_cs=cs/2;
	uint16_t xpos, ypos,len;
	uint8_t minus_flag=(n<0)?1:0;
	int dot_pos0,dot_pos1;
	dot_pos0=x+(18*cs);
	dot_pos1=x+(30*cs);
	n=abs(n);
	xpos=x+(7*cs);
	ypos=y+(7*cs);
	digit=(n>99)?3:2;
//clear+++
	len=draw7Number(888, xpos, y, cs, bc, bc, 3, dash);
	if(!minus_flag) pIli9488->ILI9488_Draw_HLineW(x, ypos, 5*cs, 2*cs, bc);
	pIli9488->Draw_Filled_Circle( dot_pos0, y+(15*cs), cs, bc);
	pIli9488->Draw_Filled_Circle( dot_pos1, y+(15*cs), cs, bc);
//clear---
	digit=(n>99)?3:2;
	len=draw7Number(n, xpos, y, cs, fc, bc, digit, dash);
	xpos+=len;
	if(digit==2)pIli9488->Draw_Filled_Circle( dot_pos0, y+(15*cs), cs, fc);
	else pIli9488->Draw_Filled_Circle( dot_pos1, y+(15*cs), cs, fc);
	if(minus_flag)	{
		pIli9488->ILI9488_Draw_HLineW(x, ypos, 5*cs, 2*cs, fc);
	}
	return xpos;
}

uint16_t LCD_MAIN::draw_3FND_dd(int n, int x, uint16_t y, uint16_t cs, uint16_t fc,  uint16_t bc, uint8_t dash){
	//n=-17;
	uint8_t digit;
	//uint8_t adj_cs=cs/2;
	uint16_t xpos, ypos,len;
	uint8_t minus_flag=(n<0)?1:0;
	int dot_pos0,dot_pos1;
	dot_pos0=x+(18*cs)+3;
	dot_pos1=x+(30*cs)+3;
	n=abs(n);
	xpos=x+(7*cs);
	ypos=y+(7*cs);
	digit=(n>99)?3:2;
//clear+++
	len=draw7Number(888, xpos, y, cs, bc, bc, 3, dash);
	if(!minus_flag) pIli9488->ILI9488_Draw_HLineW(x, ypos, 5*cs, 2*cs, bc);
	pIli9488->Draw_Filled_Circle( dot_pos0, y+(15*cs), cs, bc);
	pIli9488->Draw_Filled_Circle( dot_pos1, y+(15*cs), cs, bc);
//clear---
	digit=(n>99)?3:2;
	len=draw7Number(n, xpos, y, cs, fc, bc, digit, dash);
	xpos+=len;
	if(digit==2)pIli9488->Draw_Filled_Circle( dot_pos0, y+(15*cs), cs, fc);
	else pIli9488->Draw_Filled_Circle( dot_pos1, y+(15*cs), cs, fc);
	if(minus_flag)	{
		pIli9488->ILI9488_Draw_HLineW(x, ypos, 5*cs, 2*cs, fc);
	}
	return xpos;
}


uint16_t LCD_MAIN::draw_4FND(int n1, int n2, uint16_t x, uint16_t y, uint16_t cs, uint16_t fc,  uint16_t bc){
	uint16_t xpos,len;
	len=draw7Number(n1, x, y, cs, fc, bc, 2,1);
	xpos=x+len;
	pIli9488->Draw_Filled_Circle( xpos+(1*cs), y+(4*cs), cs, fc);
	pIli9488->Draw_Filled_Circle( xpos+(1*cs), y+(11*cs), cs, fc);
	len=draw7Number(n2,xpos+(4*cs), y, cs,fc,bc,2,0);
	xpos+=len;
	return xpos;
}

void LCD_MAIN::Navi_Draw_Sub(uint8_t sub)
{
	int i, sub_menu_cnt;
	sub_menu_cnt = UI_Info.sub_menu_cnt[sub];
//draw sub title++
	pIli9488->ILI9488_Draw_Filled_Rectangle_Coord(0, 0, 480, 320, TFT_WHITE);
//draw sub title--
//	pSTM32_IO->on_item_drawing = 1;
//	pSTM32_IO->cancel_key = 0;
	for (i = 0; i < sub_menu_cnt; i++)
	{
		//printf("Navi_Draw_Sub[%d]\r\n",i);
		if (i == 0) Navi_Draw_Item(sub, 0, 1, 0);
		else Navi_Draw_Item(sub, i, 0, 0);
	}
//	pSTM32_IO->on_item_drawing = 0;
}
uint16_t LCD_MAIN::Navi_Item_Ypos(uint8_t item)
{
	uint16_t item_pos;
	uint16_t ypos = 0;
	item_pos = item;
	ypos = ITEM_START_Y + (item_pos * ITEM_HEIGHT);
	return ypos;
}


ITEM_DISPLAY_STRING LCD_MAIN::mk_Item_string(uint8_t sub, int posA, int posB, int posC){
	ITEM_DISPLAY_STRING dis_str;
	sprintf(dis_str.A_str, "%s",UI_Info.item_num_arry[sub].str[posA]);
	sprintf(dis_str.B_str, "%s",UI_Info.item_num_arry[sub].str[posB]);
	sprintf(dis_str.C_str, "%s",UI_Info.item_num_arry[sub].str[posC]);
	return dis_str;
}



void LCD_MAIN::Navi_Draw_Item( uint8_t sub, uint8_t item, uint8_t selected,	uint8_t renewal)
{
	ITEM_DISPLAY_STRING dis_str;
	//uint8_t sub = 0;//navi.sub;
	uint16_t select_color=TFT_BLUE_SKY;//ITEM_SELECTED_COLOR

	int xpos = ITEM_START_X;
	int sub_menu_cnt = 9;//UI_Info.sub_menu_cnt[sub];
	int A_pos = (item > 0) ? item - 1 : sub_menu_cnt - 1; //
	int B_pos = item;
	int C_pos = ((item + 1) >= sub_menu_cnt) ? 0 : item + 1; //

	int A_Ypos = ITEM_START_Y + (A_pos * ITEM_HEIGHT);
	int B_Ypos = ITEM_START_Y + (B_pos * ITEM_HEIGHT);
	int C_Ypos = ITEM_START_Y + (C_pos * ITEM_HEIGHT);

	printf("11111>>>Navi_Draw_Item sub[%d] item[%d] \r\n",sub, item);

	//dis_str = Navi_MENU(sub, item);
	dis_str=Navi_MENU_updn_make_string(sub, item);

	if (selected)
	{
		if (renewal)
		{
			//이전거 지우기
			pIli9488->draw16_string(xpos, A_Ypos, TFT_BLACK, ITEM_BG_COLOR,(char*) dis_str.A_str, 2, 0);
			pIli9488->draw16_string(SET_VALUE_REF_X, A_Ypos, TFT_BLACK,ITEM_BG_COLOR, (char*) dis_str.A_val, 2, 0);
			pIli9488->draw16_string(xpos, C_Ypos, TFT_BLACK, ITEM_BG_COLOR,(char*) dis_str.C_str, 2, 0);
			pIli9488->draw16_string(SET_VALUE_REF_X, C_Ypos, TFT_BLACK,ITEM_BG_COLOR, (char*) dis_str.C_val, 2, 0);
		}
		//현재거 쓰기
		pIli9488->draw16_string(xpos, B_Ypos, select_color,ITEM_BG_COLOR, (char*) dis_str.B_str, 2, 0);
		pIli9488->draw16_string(SET_VALUE_REF_X, B_Ypos, select_color,	ITEM_BG_COLOR, (char*) dis_str.B_val, 2, 0);//[1/6]
	}
	else
	{
		pIli9488->draw16_string(xpos, B_Ypos, TFT_BLACK, ITEM_BG_COLOR,(char*) dis_str.B_str,2, 0);
		pIli9488->draw16_string(SET_VALUE_REF_X, B_Ypos, TFT_BLACK,ITEM_BG_COLOR, (char*) dis_str.B_val,2, 0);
	}
//DrawFontTTF16Asc
}
