#ifndef TYPEDEF_H
#define TYPEDEF_H

#define REV_NUM 10

#define ADC_SENSITIVITY 0.1f

#define LIFT_KEY_HOLD_TIMEOUT 20
#define STOP_KEY_HOLD_TIMEOUT 20
#define CAN_ALIVE_TIMEOUT 20


#define ZIGBEE_MY_ID 0xCC

#define BATTERY24_100 25.40f
#define BATTERY24_095 25.25f
#define BATTERY24_090 25.16f
#define BATTERY24_085 25.04f
#define BATTERY24_080 24.92f
#define BATTERY24_075 24.80f
#define BATTERY24_070 24.72f
#define BATTERY24_065 24.64f
#define BATTERY24_060 24.56f
#define BATTERY24_055 24.48f
#define BATTERY24_050 24.40f
#define BATTERY24_045 24.32f
#define BATTERY24_040 24.24f
#define BATTERY24_035 24.16f
#define BATTERY24_030 24.08f
#define BATTERY24_025 24.00f
#define BATTERY24_020 23.96f
#define BATTERY24_015 23.92f
#define BATTERY24_010 23.88f
#define BATTERY24_005 23.84f
#define BATTERY24_000 23.80f


/*
LED 표시 레벨
8칸 13.3v 이상 100%
7칸 12.9v 이상 90%
6칸 12.5v 이상 75%
5칸 12.2v 이상 60%
4칸 11.9v 이상 50%
3칸 11.6v 이상 40%
2칸 11.3v 이상 25%
1칸 11.1v 이상 10%
출처: https://itrooms.tistory.com/821
*/
 //
#define BATTERY12_100 13.3f
#define BATTERY12_090 12.9f
#define BATTERY12_075 12.5f
#define BATTERY12_060 12.2f
#define BATTERY12_050 11.9f
#define BATTERY12_040 11.6f
#define BATTERY12_025 11.3f
#define BATTERY12_010 11.1f
#define BATTERY12_000 10.8f

#define BATT_VOLTAGE 24

#define TROTTLE_MAX   1024
#define LIMIT_MAX     1024
#define LEAP_TIME_MAX 100 //9.9sec 10sec
#define BATTERY_CHECK_TIME_OUT 20 //20sec

#include "stm32f1xx_hal.h"
//#include <ctime>
#include <string>
#include <cstdarg>
#include <vector>
//#include <queue>

using namespace std;

#define ZIGBEE_REMAIN 3

typedef struct {
	uint8_t  soc;
	uint8_t  step;
}BATTERY_SOC;
//-----------------------------

typedef struct {
	int pwm_value;
	uint8_t  pwm_dir;
	uint8_t  pwm_run;
	uint8_t  pwm_en;
}PWM_PWM;

typedef struct {
	PWM_PWM pwm1;
	PWM_PWM pwm2;
}PWM_DATA;

typedef struct {
  uint8_t TOGGLE_A:2;
  uint8_t TOGGLE_B:2;
  uint8_t TOGGLE_C:2;
  uint8_t TOGGLE_D:2;
}BIT_PORT_INPUT;

typedef union _PORT_INPUT
{
  BIT_PORT_INPUT b;
  uint8_t u8;
}PORT_INPUT;


typedef struct _RELAY
{
  uint8_t MC1:1;
  uint8_t MC2:1;
  uint8_t BRAKE:1;
  uint8_t LLIGHT:1;
  uint8_t RLIGHT:1;
  uint8_t FAN1:1;
  uint8_t FAN2:1;
}RELAY;

typedef struct {
  int16_t adc1; //2byte
  int16_t adc2; //2byte
  BIT_PORT_INPUT remote_group_sw1;//1byte
  BIT_PORT_INPUT remote_group_sw2;//1byte
  RELAY remote_relay;
  uint8_t x7;
}STRUCT_TX_DATA;


//------------MAIN-----------

typedef union _BIT_SYSTEM_FLAG {
    struct {
    	uint8_t canReady:1;
    	uint8_t zbReady:1;
    	uint8_t bisang:1;
    	uint8_t cruise:1;
    	uint8_t leap:1;
    	uint8_t motor_dir1:1;
    	uint8_t motor_dir2:1;
    	uint8_t SaveEEPROM:1;
    };
    uint8_t u8;
} BIT_SYSTEM_FLAG;


typedef struct _BIT_FLAG
{
  uint8_t flg_emegency:1;
  uint8_t trot_zero1:1;
  uint8_t trot_zero2:1;
  uint8_t stop1:1;
  uint8_t stop2:1;
  uint8_t x5:1;
  uint8_t x6:1;
  uint8_t x7:1;
}BIT_FLAG;

typedef union _FLAG_STATE
{
  BIT_FLAG b;
  uint8_t u8;
}FLAG_STATE;

typedef struct {
  int16_t pwm1;
  int16_t pwm2;
  uint8_t fet_temp;
  uint8_t motor_temp;
  FLAG_STATE state;
  PORT_INPUT in_port;
}RX_CAN_DATA;

typedef struct {
  PORT_INPUT quad_sw1;
  PORT_INPUT quad_sw2;
  FLAG_STATE m1_state_flg;
  FLAG_STATE m2_state_flg;
  int16_t cpu_temp;
  uint16_t battery_voltage;
  uint16_t InstantaneousVoltage;
  uint16_t charger_voltage;
  uint8_t  fet_temp;
  uint8_t  m2_fet_temp;
  uint8_t  motor_temp;
  uint8_t  m2_motor_temp;
  int16_t  co2;
  int16_t  protect_current;
  int16_t  protect_temperature;
  int8_t rssi;
  RELAY relay;
  FLAG_STATE flag;
}MAIN_MAIN;


//-------------------------------

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
}color_t;

typedef struct {
  uint16_t crc;
  char build_date[28];
}SYSTEM_INFORMATION;


// I2C acknowledge
typedef enum{
  ACK                      = 0,
  NO_ACK                   = 1,
}etI2cAck;

typedef enum{
  LOW                      = 0,
  HIGH                     = 1,
}etI2cLevel;

// Error codes
typedef enum{
  ACK_ERROR                = 0x01,
  TIME_OUT_ERROR           = 0x02,
  CHECKSUM_ERROR           = 0x04,
  UNIT_ERROR               = 0x08
}etError;

/////////////////

enum ENUM_KIND_SWITCH{
	SW_A=0,
	SW_B,
	SW_C,
	SW_D,
	SW_1,
};

enum ENUM_JUHANG_STATE{
	cART_STOP=0,
	cART_JENJIN,
	cART_HUJIN,
	cART_JENJIN_LEFT_TURN,
	cART_JENJIN_RIGHT_TURN,
	cART_HUJIN_LEFT_TURN,
	cART_HUJIN_RIGHT_TURN,
	cART_NONE_SPIN,
	cART_LEFT_SPIN,
	cART_RIGHT_SPIN,
	cART_BISANG,
};

enum ENUM_FUNCTION_STATE{
	cART_FUNCTION_NONE=0,
	cART_CRUISE_ON,
	cART_LEAP_ON,
	fct_LIFT_UP,
	fct_LIFT_DN,
};

typedef struct
{
    uint8_t jenhujin :2;
    uint8_t stop_lr :2;
    uint8_t lift1_ud :2;
    uint8_t lift2_ud :2;
}bTOGGLE;

typedef struct
{
    uint8_t x0 :1;
    uint8_t x1 :1;
    uint8_t x2 :1;
    uint8_t x3 :1;
    uint8_t x4 :1;
    uint8_t leap :1;
    uint8_t running :1;
    uint8_t emergency :1;
}bBUTTON;

typedef union _MY_TOGGLE
{
  bTOGGLE b;
  uint8_t u8;
}MY_TOGGLE;

typedef union _MY_BUTTON
{
  bBUTTON b;
  uint8_t u8;
}MY_BUTTON;

typedef struct
{
	int16_t orgspeed;
	int16_t speed1;
    int16_t speed2;
    int16_t limit;
    MY_TOGGLE toggle;
    MY_BUTTON btn;
    uint8_t checksum;
    uint8_t etx;
}VCU_SERVICE_DATA_UNIT;

typedef struct
{
    uint8_t stx;
    uint8_t id;
    int16_t speed;
    int16_t limit;
    uint16_t tbd[3];
    MY_BUTTON btn1;
    MY_BUTTON btn2;
    uint8_t checksum;
    uint8_t etx;
}RCU_DATA_STRUCTURE;

typedef struct
{
    int16_t speed;
    int16_t limit;
    MY_BUTTON btn1;
    MY_BUTTON btn2;
    uint8_t checksum;
    uint8_t etx;
}VCU_CAN_TX_RUN;

typedef struct
{
    int16_t speed;
    int16_t limit;
    MY_BUTTON btn1;
    MY_BUTTON btn2;
    uint8_t checksum;
    uint8_t etx;
}VCU_CAN_TX_SET;

//typedef struct
//{
//    uint8_t stx;
//    uint8_t id;
//    int16_t speed;
//    int16_t limit;
//    uint16_t tbd[3];
//    MY_BUTTON btn1;
//    MY_BUTTON btn2;
//    uint8_t checksum;
//    uint8_t etx;
//}VCU_DATA_STRUCTURE;

typedef struct
{
    uint8_t stx;
    uint8_t id;
    int16_t speed;
    int16_t limit;
    int16_t Vbat;
    int8_t ntc_fet;
    int8_t ntc_mot;
    int8_t dummy1;
    MY_BUTTON dbtn1;
    MY_BUTTON dbtn2;
    uint8_t rssi;
    uint8_t checksum;
    uint8_t etx;
}DCU_DATA_STRUCTURE;
//==========================================================

typedef struct
{
	uint16_t idx;
	uint16_t battery_voltage;
	uint16_t limit_current;
	uint16_t limit_motor_temp;
	uint16_t limit_fet_temp;
	uint16_t alarm_Battery;
	uint16_t cart_type;
	uint16_t motor1_polarity;
	uint16_t motor2_polarity;
	uint16_t tbd;
}SYSTEM_CONF1;

typedef struct
{
	uint16_t idx;
	uint16_t tottle_offset;
	uint16_t tottle_inflec;
	uint16_t foreward;
	uint16_t backward;
	uint16_t accel;
	uint16_t decel;
	uint16_t brake_delay;
	uint16_t brake_rate;
	uint16_t checkSum;
}SYSTEM_CONF2;




typedef struct
{
	char *str[10];
} MENU_ITEM_STRING;
typedef struct
{
	char *str[10];
} MENU_SET_VALUE;

typedef struct
{
	uint16_t idata[10];
}MENU_DATA;

typedef struct
{
	uint8_t total_menu_cnt;
	uint8_t sub_menu_cnt[2];
	const MENU_ITEM_STRING *item_num_arry;
	const MENU_SET_VALUE *value_arry;
} MENU_STRING_INFO;

typedef struct
{
	SYSTEM_CONF1 system_conf1;
	SYSTEM_CONF2 system_conf2;
}CONFIG_TOTAL;

const MENU_ITEM_STRING init_item_str[] =
{
	{ 	(char*) "VCU System",
		(char*) "배터리 전압",
		(char*) "전류 제한",
		(char*) "모터온도 제한",
		(char*) "소자온도 제한",
		(char*) "저전압 설정",
		(char*) "바퀴 Type",
		(char*) "모터1 역회전",
		(char*) "모터2 역회전",
		(char*) ""
	},
	{ 	(char*) "VCU Drive",
		(char*) "Throttle Offset",
		(char*) "Throttle Inflec.",
		(char*) "전진 비율(%)",
		(char*) "후진 비율(%)",
		(char*) "Accel(0-100)",
		(char*) "Decel(0-100)",
		(char*) "브레이크 지연",
		(char*) "회전상대감속",
		(char*) ""
	},
};

const MENU_SET_VALUE init_value_str[] =
{
		{ 		(char*) "[1/2]",
				(char*) "[%02dV]",		//
				(char*) "[%03dA]",		//
				(char*) "[%02doC]",		//
				(char*) "[%02doC]",		//
				(char*) "[%02dV]",		//
				(char*) "[%d]",	//0:궤도 1:바퀴
				(char*) "[%d]",	//
				(char*) "[%d]",	//
				(char*) ""		//
		},
		{ 		(char*) "[2/2]",
				(char*) "[%03d]",		//
				(char*) "[%03d]",		//
				(char*) "[%02d%%]",		//
				(char*) "[%02d%%]",		//
				(char*) "[%d]",		//
				(char*) "[%d]",	//
				(char*) "[%03dms]",	//
				(char*) "[%d]",	//
				(char*) ""		//
		},
};

const MENU_DATA init_data[] =
{
	{
		0,	//타이틀
		0,	//BATTERY VOLTAGE
		200,//Limit_CURRENT
		90,//Limit MOTOR Temp
		85,	//Limit FET Temp
		23,	//Low BATTERY
		1,//cart type 0:궤도 1:바퀴
		0,  //MOTOR1 극성
		0, //MOTOR2 극성
		99,
	},
	{
		1,	//타이틀
		300,	//Trottle Offset
		900,	//Trottle Inflec
		100,	//forward
		80,	//backward
		5,	//Accel
		5, //Decel
		300, //Brake Delay(ms)
		1, //Brake Rate
		99
	},
};

typedef struct
{
	char A_str[64];
	char A_val[64];
	char B_str[64];
	char B_val[64];
	char C_str[64];
	char C_val[64];
}ITEM_DISPLAY_STRING;


typedef struct
{
	MENU_DATA data_arry[2];
}MENU_SET_DATA;

const MENU_STRING_INFO UI_Info =
{ 	 				//  total_menu_cnt
	 2, 				//  total_menu_cnt
	{ 9, 9 },
	init_item_str, 	//  item string array
	init_value_str
};

typedef struct
{
	uint8_t ex_pwm1;
	uint8_t ex_pwm2;
	uint16_t BatteryVolt;
	uint8_t fet_temp;
	uint8_t motor_temp;
	uint8_t motor_rpm;
	BIT_SYSTEM_FLAG flg_state;
}CAN_RESPONSE;

typedef enum
{
	NO_PRESS=0,
	KEY_UP,
	KEY_DN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_BISANG,
	KEY_CRUISE,
	KEY_LEAP,
	KEY_SETING,
	KEY_LIFT_UP,
	KEY_LIFT_DN,
	KEY_LEFT_STOP,
	KEY_RIGHT_STOP,
	KEY_ONLY_BEEP,
} eButtonEvent;

enum MENU_DRAW_EVENT {
	KEY_HOME_SETUP_EVENT=0,
	KEY_SET_MODE_SETUP_NEXT_EVENT,//1
	KEY_TOP_MODE_SETUP_NEXT_EVENT,//2
	KEY_TOP_MODE_ITEM_EVENT,//3
	KEY_ITEM_VALUE_UPDN_EVENT,//4
	KEY_TOP_MODE_DN_EVENT,//5
	KEY_SELJUNG_UPDN_EVENT,
	KEY_TOP_MENU_CHANGE_EVENT,
	KEY_SET_MODE_DUMMY_EVENT,
	KEY_CANCEL_EVENT,
};

typedef struct _NAVI_BIT
{
    uint16_t  system_setup: 1; //0:main 1:setup
//    uint16_t  top_title: 4;
    uint16_t  sub: 4;
    uint16_t  item: 4;
 //   uint16_t  depth: 4;
}NAVI_BIT;


inline std::string format(const char* fmt, ...){
    int size = 512;
    char* buffer = 0;
    buffer = new char[size];
    va_list vl;
    va_start(vl, fmt);
    int nsize = vsnprintf(buffer, size, fmt, vl);
    if(size<=nsize){ //fail delete buffer and try again
        delete[] buffer;
        buffer = 0;
        buffer = new char[nsize+1]; //+1 for /0
        nsize = vsnprintf(buffer, size, fmt, vl);
    }
    std::string ret(buffer);
    va_end(vl);
    delete[] buffer;
    return ret;
}

#endif
