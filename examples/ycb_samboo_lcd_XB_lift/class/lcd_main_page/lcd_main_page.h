#ifndef LCD_MAIN_PAGE_H
#define LCD_MAIN_PAGE_H

#define Get_Unique8(x) ((x >= 0 && x < 12) ? (*(uint8_t *) (UID_BASE + (x))) : 0)
#define Get_Unique16(x) ((x >= 0 && x < 6) ? (*(uint16_t *) (UID_BASE + 2 * (x))) : 0)
#define Get_Unique32(x) ((x >= 0 && x < 3) ? (*(uint32_t *) (UID_BASE + 4 * (x))) : 0)


//GRAPH SHAPE
#define GRAPH_BG_COLOR TFT_WHITE
#define GRAPH_FG_COLOR TFT_BLACK
#define GRAPH_TMP_COLOR TFT_RED
#define GRAPH_O3_COLOR  TFT_BLUE
#define GRAPH_HUMIDITY_COLOR  TFT_ORANGE

#define GS_WIDTH   432
#define GS_HEIGHT  168
#define GS_X_GAP   GS_WIDTH/48   //9 48개 포인터
#define GS_X_START 24   //24
#define GS_Y_START 50   //24
//#define GS_Y_GAP   GS_HEIGHT/3   //54

#define GS_DISPLAY_MAX_TEMP 13   //13도
#define GS_DISPLAY_MAX_O3   900   //900ppb (0.9ppm)

//--------LED POSITION -------
#define ICON_START_Y  24
#define ICON_START_X  336
#define ICON_BETWIN_Y 36

#define MENU_LED_X 430
#define MENU_LED_RAD 16

#define OP_LED_X 40
#define OP_LED_Y 306
#define OP_LED_RAD 10
#define OP_LED_ON_COLOR  TFT_RED
#define OP_LED_OFF_COLOR TFT_GREENYELLOW

//-------MENU POSITION--------
#define ITEM_START_X 20
#define ITEM_TITLE_Y 12
#define ITEM_START_Y 20 //타이틀에서 이격
#define ITEM_HEIGHT 33

//--------TOP_MENU----------
#define TOP_MEASURE_POS_X  100//130
#define TOP_UINT_POS_X 250

#define VALUE_BOX_A TOP_MEASURE_POS_X
#define VALUE_BOX_B TOP_UINT_POS_X-10
#define VALUE_BOX_W VALUE_BOX_B-VALUE_BOX_A
#define VALUE_BOX_H 40

#define TOP_TITLE_X 20//36
#define TOP_MEASURE_POS_Y  20//40
#define TOP_MEASURE_POS_Y1 80//100
#define TOP_MEASURE_POS_Y2 140
#define TOP_SELJUNG_POS_X  TOP_MEASURE_POS_X
#define TOP_SELJUNG_POS_Y  234
#define TOP_SELJUNG_POS_Y1 268

//#define TOP_MENU_START_X  34
#define TOP_MENU_CURRENT_Y 84
#define TOP_MENU_SELJUNG_Y 214
#define TOP_MENU_CENTER_HLINE_X 38
#define TOP_MENU_CENTER_HLINE_Y 168
#define TOP_MENU_CENTER_HLINE_LENGTH 260

//---------------------------
#define PERIOD_BEGIN_X TOP_MENU_START_X
#define PERIOD_BEGIN_Y TOP_MEASURE_POS_Y+70
//#define PERIOD_LENGTH 260

//-------ALARM STRING-------
#define ALARM_STRING_X  TOP_MENU_START_X
#define ALARM_STRING_Y  280
#define ALARM_STRING_NORMAL_COLOR    TFT_WHITE

//#define BAR_COLOR1 TFT_BLACK
#define MAIN_X 0
#define MAIN_Y 0
#define MAIN_W 480 //320
#define MAIN_H 260

//GRAPHIC BAR+++
#define GRAPHIC_BAR_X 48//72
#define GRAPHIC_BAR_Y TOP_SELJUNG_POS_Y+14
#define GRAPHIC_BAR_TOTAL_H 14
#define GRAPHIC_BAR_PAINT_H 8
#define GRAPHIC_BAR_WIDTH 64
//GRAPHIC BAR---
//--------SIDE------
#define SIDE_BG_COLOR   TFT_BLACK
#define SIDE_X 320
#define SIDE_Y 150 //0
#define SIDE_W 480-SIDE_X
#define SIDE_H 110 //260

//-------FOOTER--------------
#define FOOTER_BG_COLOR 0x31C9
#define FOOTER_ROUND_COLOR 0xFD20
#define FOOTER_X 0
#define FOOTER_Y 260
#define FOOTER_W 480
#define FOOTER_H 320-MAIN_H

//--------BAR-------
#define BAR_X TOP_TITLE_X
#define BAR_Y 140
#define BAR_W 480-TOP_TITLE_X-TOP_TITLE_X//-20 //290+20
#define BAR_H 4
#define BAR_COLOR MINJI_GRAY//TFT_WHITE //MINJI_RED//MINJI_GRAY//TFT_ORANGE

//------SELSUNG-----
#define SJ_TEXT_COLOR MINJI_RED//TFT_MAGENTA//TFT_NAVY//
#define MEASURE_TEXT_COLOR TFT_BLACK//MINJI_GRAY
//
#define ITEM_SELECTED_COLOR TFT_RED
#define ITEM_BG_COLOR 		 TFT_WHITE

#define SET_VALUE_REF_X  320

#define MAXIUM_TEMP_DIFF      50
#define MAX_COMP_DELAY        240
#define MAX_EVA_FAN_DELAY     60
#define MAX_TEMP_ALARM_JISOK  120
#define MAX_HI_TEMP_ALARM     50
#define MAX_LO_TEMP_ALARM    -20

#define MAX_DEF_PERIODE 480
#define MAX_DEF_TIME    30
#define MAX_JAESANG_PF  300
#define MAX_EVA_FAN_DELAY_AF_DEF  600
#define MAX_DEF_AFT_COMP_DELAY  600

#define MAX_O3_GEN_DELAY  240
#define MAX_O3_FAN_DELAY  240
#define MAX_O3_DIFF  100
#define MAX_O3_GEN_PERIODE  120
#define MAX_O3_GEN_TIME  120

#define MAX_DATE_YEAR   2050
#define MAX_DATE_MONTH  12
#define MAX_DATE_DAY    31
#define MAX_TIME_HOUR   24
#define MAX_TIME_MINUTE 60
#define MAX_TEMP_KEEP   60

#define MAX_CONTROL_TEMP_H  500
#define MAX_CONTROL_TEMP_L -500
#define MAX_CONTROL_OZEON_H 20*60//500
#define MAX_CONTROL_OZEON_L 0//10
#define MAX_CONTROL_WIND_H 90
#define MAX_CONTROL_WIND_L 10

enum FLASH_ICON
{
	IMG_on_r_comp = 0,
	IMG_on_r_defrost,
	IMG_on_r_evafan,
	IMG_on_r_ozone,
	IMG_on_r_ozonefan,
	IMG_on_r_alarm,
	IMG_on_g_comp,
	IMG_on_g_defrost,
	IMG_on_g_evafan,
	IMG_on_g_ozone,
	IMG_on_g_ozonefan,
	IMG_on_g_alarm,
	IMG_off_comp, //12
	IMG_off_defrost,
	IMG_off_evafan,
	IMG_off_ozone,
	IMG_off_ozonefan,
	IMG_off_alarm,
};

typedef struct
{
	NAVI_BIT navi;
	MENU_DRAW_EVENT draw_event;
	int16_t position;
	uint8_t event_flag;
}UI_EVENT;

class LCD_MAIN
{
private:
	BATTERY_SOC batt_soc;
	//BATTERY_SOC battery_step_calcuration(uint16_t value);
	BATTERY_SOC nabsan_battery_step_calcuration(uint16_t value);
	BATTERY_SOC battery_step_calcuration(uint16_t volt_inx,uint16_t value);
	uint8_t bar_step_calcuration(uint16_t value);
	uint16_t BattVoltage, exBattVoltage;
	uint8_t Update_StatusMessage_cnt = 0;
	uint8_t b1_cnt,b2_cnt, b3_cnt, b4_cnt, b5_cnt;
	uint8_t  calcu_percent(int value);

	void Update_Display_status();
	void Update_Display_Set_UnderLine();
	void Footer_Message(char *msg, int x, int y, int width, uint16_t color);
	//void Update_StateMark();
	ITEM_DISPLAY_STRING Navi_MENU_updn_make_string(uint8_t sub, uint8_t item);
	void Navi_Draw_Item_ValueUpdate(NAVI_BIT navi, int16_t position);
	void Update_StatusMessage();
	void Update_Wheel_icon(uint8_t toggle_sw, uint8_t stop);
	void Draw_graphic_battery_bar(uint8_t b1_cnt);
	void Draw_graphic_JUHANG_bar(uint8_t b2_cnt, uint8_t b3_cnt, uint8_t jenhujin, uint8_t stop);
	void Draw_graphic_dd_bar(uint8_t b4_cnt, uint8_t b5_cnt);
	void Draw_Home();
	//void Draw_memu_system_information();
	void Draw_TopMenu_seljung();
	void Draw_TopMenu_Form();
	void Draw_JenHujin_toggle(uint8_t toggle);
	unsigned int draw7Number(long n, unsigned int xLoc, unsigned int yLoc, char cS, unsigned int fC, unsigned int bC, char nD, uint8_t dash);
	uint16_t draw_humidity_2FND(int n, uint16_t x, uint16_t y, uint16_t cs, uint16_t fc,  uint16_t bc);
	uint16_t draw_3FND(int n, int x, uint16_t y, uint16_t cs, uint16_t fc,  uint16_t bc, uint8_t dash);
	uint16_t draw_3FND_dd(int n, int x, uint16_t y, uint16_t cs, uint16_t fc,  uint16_t bc, uint8_t dash);
	uint16_t draw_4FND(int n1, int n2, uint16_t x, uint16_t y, uint16_t cs, uint16_t fc,  uint16_t bc);
	ITEM_DISPLAY_STRING mk_Item_string(uint8_t sub, int posA, int posB, int posC);
	//ITEM_DISPLAY_STRING Navi_MENU(uint8_t sub,uint8_t item);
	void Navi_Draw_Item( uint8_t sub, uint8_t item, uint8_t selected,	uint8_t renewal);
	void Navi_Draw_Sub(uint8_t sub);
	void NAVI_Draw_TopMode(NAVI_BIT navi);
	uint16_t Navi_Item_Ypos(uint8_t item);
	//char* mk_on_off_str(uint8_t sub, uint8_t item_str_pos, uint8_t on);
	//char* mk_ok_err_str(uint8_t sub, uint8_t item_str_pos, uint8_t err);
	//char* mk_ok_mode_str(uint8_t sub, uint8_t item_str_pos, uint8_t mode);
	void Draw_BattOutline(uint16_t xpos, uint16_t ypos);
	void Draw_HBattOutline(uint16_t xpos, uint16_t ypos);
	void Draw_Information(uint16_t xpos, uint16_t ypos);

public:
	uint8_t DRAW_MUTEX = 0;
	uint8_t change_background = 0;
	uint16_t color_dd_blue;
	UI_EVENT uiEvent;

	LCD_MAIN();
	virtual ~LCD_MAIN();
	void initPage(void);
	void update_DrawMainMenu(uint8_t update);
	void NAVI_ChangeMode(NAVI_BIT navi, MENU_DRAW_EVENT draw_event, int16_t position);
	void Draw_Key_Event();

};

#endif
