#pragma once

#ifndef EXTERN_H_
#define EXTERN_H_


#include "main.h"
//#include "typedef.h"
#include <string.h>
#include "stdio.h"
#include "main.h"
#include "adc.h"
//#include "tim.h"
#include "can.h"
#include "usart.h"
#include "iwdg.h"

#include "../../class/lcd/driver/ili9488_drv.h"
#include "../../class/data_class/dataclass.h"
#include "../../class/tja1050/tja1050.h"
//#include "../../class/rs485/rs485.h"
#include "../../class/lcd_main_page/lcd_main_page.h"
#include "../../class/zigbee/zigbee.h"
#include "../../class/stm32_io/stm32_io.h"
//#include "../../class/bigint/BigInt.h"

/////////////////////////////////////
#define xDBG_BOARD
#define xDBG_EVENT
#define xDBG_ONE_SEC
#define xDBG_MODEM
#define xDBG_DATACLASS
#define xDBG_TEMP_ALARM
#define xDBG_UPDATE
#define xDBG_TEST_MODE
#define xDBG_ADC
#define xSENSOR


#define MINUTE  60
#define SECOND  1


#define SWAPBYTE_US(X) ((((X) & 0xFF00)>>8) | (((X) & 0x00FF)<<8))
#define BUILD_UINT16(loByte, hiByte) \
          ((int)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))
#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)

#define _ClearBit(Data, loc)   ((Data) &= ~(0x1<<(loc)))    // 한 bit Clear
#define _SetBit(Data, loc)     ((Data) |= (0x01 << (loc)))  // 한 bit Set
#define _InvertBit(Data, loc)  ((Data) ^= (0x1 << (loc)))   // 한 bit 반전
#define _CheckBit(Data, loc)   ((Data) & (0x01 << (loc)))   // 비트 검사
#define ON 1
#define OFF 0

#define RET_OK 1
#define RET_ERR 0
//10K Thermist
#define OHM_LIMIT_LO 0     //0R  OHM
#define OHM_LIMIT_HI 40000 //40K OHM

extern int voltage_table[6];

extern ILI9488_DRV *pIli9488;
//extern RS485 *pRS485;
//extern SPI1_FLASH *pFlashClass;
extern STM32_IO *pSTM32_IO;
extern LCD_MAIN *pLCD_MAIN;
extern data_class *pDataClass;
extern tja1050 *pCAN;

extern zigbee *pZIGBEE;
extern uint8_t draw_UI_flag;
extern uint16_t gBG_COLOR;

extern const unsigned char IMG_dd240x62[];
extern const unsigned char IMG_COW140X140[];
extern const unsigned char IMG_ycbrain_270x62[];
extern const unsigned char samboo_230x53[];

#endif
