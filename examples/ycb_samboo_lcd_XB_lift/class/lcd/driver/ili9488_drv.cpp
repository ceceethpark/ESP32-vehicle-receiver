//-----------------------------------
//	ILI9488 Driver library for STM32
//-----------------------------------
#include "extern.h"
#include "ili9488_drv.h"
//#include "spi.h"
#include "gpio.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
//#include "./font/5x5_font.h"
//#include "./font/godic_24pt.h"
#include "./img/img1_80x80.h"
#include "./font/kssm_font.h"
#include "./font/english.h"
//#include "./font/digital48ft.h"
//#include "./font/score16ft.h"
#include "./lcd_io_gpio8.c"
#include <cstring>

//extern SPI_HandleTypeDef hspi1;
//extern SPI1_FLASH *pFlashClass;

#define  LCD_REVERSE16   0
//=======================================================================================================
/* Interface mode
   - 0: SPI mode (the lcd does not work in 16bit/pixel mode in spi, so you have to write 24bit/pixel)
   - 1: paralell mode */
#define  ILI9488_INTERFACE        1

//===============================================
/* Orientation:
   - 0: 320x480 micro-sd in the top (portrait)
   - 1: 480x320 micro-sd in the left (landscape)
   - 2: 320x480 micro-sd in the bottom (portrait)
   - 3: 480x320 micro-sd in the right (landscape)
*/
#define  ILI9488_ORIENTATION       3

//===============================================
/* Color mode
   - 0: RGB565 (R:bit15..11, G:bit10..5, B:bit4..0) (default)
   - 1: BRG565 (B:bit15..11, G:bit10..5, R:bit4..0)
*/
#define  ILI9488_COLORMODE         1

//=====================================================
/* Touchscreen
   - 0: touchscreen disabled (default)
   - 1: touchscreen enabled
*/
#define  ILI9488_TOUCH             0

/* ILI9488 Size (physical resolution in default orientation) */
#define  ILI9488_LCD_PIXEL_WIDTH   320
#define  ILI9488_LCD_PIXEL_HEIGHT  480

//------------------------------------------
#define ILI9488_NOP           0x00
#define ILI9488_SWRESET       0x01
#define ILI9488_RDDID         0x04
#define ILI9488_RDDST         0x09

#define ILI9488_SLPIN         0x10
#define ILI9488_SLPOUT        0x11
#define ILI9488_PTLON         0x12
#define ILI9488_NORON         0x13

#define ILI9488_RDMODE        0x0A
#define ILI9488_RDMADCTL      0x0B
#define ILI9488_RDPIXFMT      0x0C
#define ILI9488_RDIMGFMT      0x0D
#define ILI9488_RDSELFDIAG    0x0F

#define ILI9488_INVOFF        0x20
#define ILI9488_INVON         0x21
#define ILI9488_GAMMASET      0x26
#define ILI9488_DISPOFF       0x28
#define ILI9488_DISPON        0x29

#define ILI9488_CASET         0x2A
#define ILI9488_PASET         0x2B
#define ILI9488_RAMWR         0x2C
#define ILI9488_RAMRD         0x2E

#define ILI9488_PTLAR         0x30
#define ILI9488_VSCRDEF       0x33
#define ILI9488_MADCTL        0x36
#define ILI9488_VSCRSADD      0x37
#define ILI9488_PIXFMT        0x3A
#define ILI9488_RAMWRCONT     0x3C
#define ILI9488_RAMRDCONT     0x3E

#define ILI9488_IMCTR         0xB0
#define ILI9488_FRMCTR1       0xB1
#define ILI9488_FRMCTR2       0xB2
#define ILI9488_FRMCTR3       0xB3
#define ILI9488_INVCTR        0xB4
#define ILI9488_DFUNCTR       0xB6

#define ILI9488_PWCTR1        0xC0
#define ILI9488_PWCTR2        0xC1
#define ILI9488_PWCTR3        0xC2
#define ILI9488_PWCTR4        0xC3
#define ILI9488_PWCTR5        0xC4
#define ILI9488_VMCTR1        0xC5
#define ILI9488_VMCTR2        0xC7

#define ILI9488_RDID1         0xDA
#define ILI9488_RDID2         0xDB
#define ILI9488_RDID3         0xDC
#define ILI9488_RDID4         0xDD

#define ILI9488_GMCTRP1       0xE0
#define ILI9488_GMCTRN1       0xE1
#define ILI9488_IMGFUNCT      0xE9

#define ILI9488_ADJCTR3       0xF7

#define ILI9488_MAD_RGB       0x00
#define ILI9488_MAD_BGR       0x08

#define ILI9488_MAD_VERTICAL  0x20
#define ILI9488_MAD_X_LEFT    0x00
#define ILI9488_MAD_X_RIGHT   0x40
#define ILI9488_MAD_Y_UP      0x80
#define ILI9488_MAD_Y_DOWN    0x00

#if ILI9488_COLORMODE == 0
#define ILI9488_MAD_COLORMODE    ILI9488_MAD_RGB
#else
#define ILI9488_MAD_COLORMODE    ILI9488_MAD_BGR
#endif

#define LCD_ORIENTATION  ILI9488_ORIENTATION


/* the drawing directions of the 4 orientations */
#if ILI9488_INTERFACE == 0 /* SPI interface */
#define ILI9488_SETCURSOR(x, y)            {LCD_IO_WriteCmd8(ILI9488_CASET); LCD_IO_WriteData16_to_2x8(x); LCD_IO_WriteData16_to_2x8(x); LCD_IO_WriteCmd8(ILI9488_PASET); LCD_IO_WriteData16_to_2x8(y); LCD_IO_WriteData16_to_2x8(y);}
#if (LCD_ORIENTATION == 0)
#define ILI9488_MAX_X                      (ILI9488_LCD_PIXEL_WIDTH - 1)
#define ILI9488_MAX_Y                      (ILI9488_LCD_PIXEL_HEIGHT - 1)
#define ILI9488_MAD_DATA_RIGHT_THEN_UP     ILI9488_MAD_COLORMODE | ILI9488_MAD_X_RIGHT | ILI9488_MAD_Y_UP
#define ILI9488_MAD_DATA_RIGHT_THEN_DOWN   ILI9488_MAD_COLORMODE | ILI9488_MAD_X_RIGHT | ILI9488_MAD_Y_DOWN
#elif (LCD_ORIENTATION == 1)
#define ILI9488_MAX_X                      (ILI9488_LCD_PIXEL_HEIGHT - 1)
#define ILI9488_MAX_Y                      (ILI9488_LCD_PIXEL_WIDTH - 1)
#define ILI9488_MAD_DATA_RIGHT_THEN_UP     ILI9488_MAD_COLORMODE | ILI9488_MAD_X_RIGHT | ILI9488_MAD_Y_DOWN | ILI9488_MAD_VERTICAL
#define ILI9488_MAD_DATA_RIGHT_THEN_DOWN   ILI9488_MAD_COLORMODE | ILI9488_MAD_X_LEFT  | ILI9488_MAD_Y_DOWN | ILI9488_MAD_VERTICAL
#elif (LCD_ORIENTATION == 2)
#define ILI9488_MAX_X                      (ILI9488_LCD_PIXEL_WIDTH - 1)
#define ILI9488_MAX_Y                      (ILI9488_LCD_PIXEL_HEIGHT - 1)
#define ILI9488_MAD_DATA_RIGHT_THEN_UP     ILI9488_MAD_COLORMODE | ILI9488_MAD_X_LEFT  | ILI9488_MAD_Y_DOWN
#define ILI9488_MAD_DATA_RIGHT_THEN_DOWN   ILI9488_MAD_COLORMODE | ILI9488_MAD_X_LEFT  | ILI9488_MAD_Y_UP
#else
#define ILI9488_MAX_X                      (ILI9488_LCD_PIXEL_HEIGHT - 1)
#define ILI9488_MAX_Y                      (ILI9488_LCD_PIXEL_WIDTH - 1)
#define ILI9488_MAD_DATA_RIGHT_THEN_UP     ILI9488_MAD_COLORMODE | ILI9488_MAD_X_LEFT  | ILI9488_MAD_Y_UP   | ILI9488_MAD_VERTICAL
#define ILI9488_MAD_DATA_RIGHT_THEN_DOWN   ILI9488_MAD_COLORMODE | ILI9488_MAD_X_RIGHT | ILI9488_MAD_Y_UP   | ILI9488_MAD_VERTICAL
#endif
#elif ILI9488_INTERFACE == 1 /* paralell interface */
#if (LCD_ORIENTATION == 0)
#define ILI9488_MAX_X                      (ILI9488_LCD_PIXEL_WIDTH - 1)
#define ILI9488_MAX_Y                      (ILI9488_LCD_PIXEL_HEIGHT - 1)
#define ILI9488_MAD_DATA_RIGHT_THEN_UP     ILI9488_MAD_COLORMODE | ILI9488_MAD_X_RIGHT | ILI9488_MAD_Y_UP
#define ILI9488_MAD_DATA_RIGHT_THEN_DOWN   ILI9488_MAD_COLORMODE | ILI9488_MAD_X_RIGHT | ILI9488_MAD_Y_DOWN
//++
#define ILI9488_MAD_DATA_LEFT_THEN_DOWN   ILI9488_MAD_COLORMODE | ILI9488_MAD_X_LEFT | ILI9488_MAD_Y_DOWN
//--
#define ILI9488_SETCURSOR(x, y)            {LCD_IO_WriteCmd8(ILI9488_CASET); LCD_IO_WriteData16_to_2x8(ILI9488_MAX_X - x); LCD_IO_WriteData16_to_2x8(ILI9488_MAX_X - x); LCD_IO_WriteCmd8(ILI9488_PASET); LCD_IO_WriteData16_to_2x8(y); LCD_IO_WriteData16_to_2x8(y);}
#elif (LCD_ORIENTATION == 1)
#define ILI9488_MAX_X                      (ILI9488_LCD_PIXEL_HEIGHT - 1)
#define ILI9488_MAX_Y                      (ILI9488_LCD_PIXEL_WIDTH - 1)
#define ILI9488_MAD_DATA_RIGHT_THEN_UP     ILI9488_MAD_COLORMODE | ILI9488_MAD_X_RIGHT | ILI9488_MAD_Y_DOWN | ILI9488_MAD_VERTICAL
#define ILI9488_MAD_DATA_RIGHT_THEN_DOWN   ILI9488_MAD_COLORMODE | ILI9488_MAD_X_LEFT  | ILI9488_MAD_Y_DOWN | ILI9488_MAD_VERTICAL
#define ILI9488_SETCURSOR(x, y)            {LCD_IO_WriteCmd8(ILI9488_CASET); LCD_IO_WriteData16_to_2x8(x); LCD_IO_WriteData16_to_2x8(x); LCD_IO_WriteCmd8(ILI9488_PASET); LCD_IO_WriteData16_to_2x8(y); LCD_IO_WriteData16_to_2x8(y);}
#elif (LCD_ORIENTATION == 2)
#define ILI9488_MAX_X                      (ILI9488_LCD_PIXEL_WIDTH - 1)
#define ILI9488_MAX_Y                      (ILI9488_LCD_PIXEL_HEIGHT - 1)
#define ILI9488_MAD_DATA_RIGHT_THEN_UP     ILI9488_MAD_COLORMODE | ILI9488_MAD_X_LEFT  | ILI9488_MAD_Y_DOWN
#define ILI9488_MAD_DATA_RIGHT_THEN_DOWN   ILI9488_MAD_COLORMODE | ILI9488_MAD_X_LEFT  | ILI9488_MAD_Y_UP
#define ILI9488_SETCURSOR(x, y)            {LCD_IO_WriteCmd8(ILI9488_CASET); LCD_IO_WriteData16_to_2x8(x); LCD_IO_WriteData16_to_2x8(x); LCD_IO_WriteCmd8(ILI9488_PASET); LCD_IO_WriteData16_to_2x8(ILI9488_MAX_Y - y); LCD_IO_WriteData16_to_2x8(ILI9488_MAX_Y - y);}
#else
#define ILI9488_MAX_X                      (ILI9488_LCD_PIXEL_HEIGHT - 1)
#define ILI9488_MAX_Y                      (ILI9488_LCD_PIXEL_WIDTH - 1)
#define ILI9488_MAD_DATA_RIGHT_THEN_UP     ILI9488_MAD_COLORMODE | ILI9488_MAD_X_LEFT  | ILI9488_MAD_Y_UP   | ILI9488_MAD_VERTICAL
#define ILI9488_MAD_DATA_RIGHT_THEN_DOWN   ILI9488_MAD_COLORMODE | ILI9488_MAD_X_RIGHT | ILI9488_MAD_Y_UP   | ILI9488_MAD_VERTICAL
//#define ILI9488_SETCURSOR(x, y)            {LCD_IO_WriteCmd8(ILI9488_CASET); LCD_IO_WriteData16_to_2x8(ILI9488_MAX_X - x); LCD_IO_WriteData16_to_2x8(ILI9488_MAX_X - x); LCD_IO_WriteCmd8(ILI9488_PASET); LCD_IO_WriteData16_to_2x8(ILI9488_MAX_Y - y); LCD_IO_WriteData16_to_2x8(ILI9488_MAX_Y - y);}
#define ILI9488_SETCURSOR(x, y)            {LCD_IO_WriteCmd8(ILI9488_CASET); LCD_IO_WriteData16_to_2x8(x); LCD_IO_WriteData16_to_2x8(x); LCD_IO_WriteCmd8(ILI9488_PASET); LCD_IO_WriteData16_to_2x8(y); LCD_IO_WriteData16_to_2x8(y);}

#endif
#endif

#define  LCD_IO_WriteData16_to_2x8(dt)    {LCD_IO_WriteData8((dt) >> 8); LCD_IO_WriteData8(dt); }


#define ILI9488_LCD_INITIALIZED    0x01
#define ILI9488_IO_INITIALIZED     0x02
static  uint8_t   Is_ili9488_Initialized = 0;

#if      ILI9488_MULTITASK_MUTEX == 1 && ILI9488_TOUCH == 1 && ILI9488_INTERFACE == 1
//#if      ILI9488_INTERFACE == 1
volatile uint8_t io_lcd_busy = 0;
volatile uint8_t io_ts_busy = 0;
#define  ILI9488_LCDMUTEX_PUSH()    while(io_ts_busy); io_lcd_busy++;
#define  ILI9488_LCDMUTEX_POP()     io_lcd_busy--
#else
#define  ILI9488_LCDMUTEX_PUSH()
#define  ILI9488_LCDMUTEX_POP()
#endif

/* Global Variables ------------------------------------------------------------------*/
extern TIM_HandleTypeDef htim1;

ILI9488_DRV *pIli9488;

//uint16_t LCD_WIDTH = 480;
//uint16_t LCD_HEIGHT = 320;

void ili9488_SetCursor(uint16_t Xpos, uint16_t Ypos);
void ili9488_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
void ili9488_Init(void);

void ili9488_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
  //ILI9488_LCDMUTEX_PUSH();
  ILI9488_SETCURSOR(Xpos, Ypos);
  //ILI9488_LCDMUTEX_POP();
}

/* The SPI mode not capable the 16bpp mode -> convert to 24bpp */
#if ILI9488_INTERFACE == 0
extern inline void ili9488_write16to24(uint16_t RGBCode);
inline void ili9488_write16to24(uint16_t RGBCode)
{
  LCD_IO_WriteData8((RGBCode & 0xF800) >> 8);
  LCD_IO_WriteData8((RGBCode & 0x07E0) >> 3);
  LCD_IO_WriteData8((RGBCode & 0x001F) << 3);
}
#endif



uint16_t ILI9488_DRV::rgb888torgb565(uint8_t red,  uint8_t green, uint8_t blue)
{
   // uint8_t red   = rgb888.r;
   // uint8_t green = rgb888.g;
   // uint8_t blue  = rgb888Pixel[2];

    uint16_t b = (blue >> 3) & 0x1f;
    uint16_t g = ((green >> 2) & 0x3f) << 5;
    uint16_t r = ((red >> 3) & 0x1f) << 11;

    return (uint16_t) (r | g | b);
}


void ILI9488_DRV::LCD_BACK_LIGHT(uint8_t on)
{
	LCD_IO_Bl_OnOff(on);
}

void ILI9488_DRV::ILI9488_Init(void)
{
  if((Is_ili9488_Initialized & ILI9488_LCD_INITIALIZED) == 0)
  {
    Is_ili9488_Initialized |= ILI9488_LCD_INITIALIZED;
    if((Is_ili9488_Initialized & ILI9488_IO_INITIALIZED) == 0)
      LCD_IO_Init();
    Is_ili9488_Initialized |= ILI9488_IO_INITIALIZED;
  }

  LCD_Delay(105);
  LCD_IO_WriteCmd8(ILI9488_SWRESET);
  LCD_Delay(5);
  // positive gamma control
  LCD_IO_WriteCmd8MultipleData8(ILI9488_GMCTRP1, (uint8_t *)"\x00\x03\x09\x08\x16\x0A\x3F\x78\x4C\x09\x0A\x08\x16\x1A\x0F", 15);
  // negative gamma control
  LCD_IO_WriteCmd8MultipleData8(ILI9488_GMCTRN1, (uint8_t *)"\x00\x16\x19\x03\x0F\x05\x32\x45\x46\x04\x0E\x0D\x35\x37\x0F", 15);
  // Power Control 1 (Vreg1out, Verg2out)
  LCD_IO_WriteCmd8MultipleData8(ILI9488_PWCTR1, (uint8_t *)"\x17\x15", 2);
  LCD_Delay(5);
  // Power Control 2 (VGH,VGL)
  LCD_IO_WriteCmd8(ILI9488_PWCTR2); LCD_IO_WriteData8(0x41);
  LCD_Delay(5);
  // Power Control 3 (Vcom)
  LCD_IO_WriteCmd8MultipleData8(ILI9488_VMCTR1, (uint8_t *)"\x00\x12\x80", 3);
  LCD_Delay(5);
  #if ILI9488_INTERFACE == 0
  LCD_IO_WriteCmd8(ILI9488_PIXFMT); LCD_IO_WriteData8(0x66); // Interface Pixel Format (24 bit)
  #if LCD_SPI_MODE != 2
  // LCD_IO_WriteCmd8(0xFB); LCD_IO_WriteData8(0x80);
  LCD_IO_WriteCmd8(ILI9488_IMCTR); LCD_IO_WriteData8(0x80); // Interface Mode Control (SDO NOT USE)
  #else
  LCD_IO_WriteCmd8(ILI9488_IMCTR); LCD_IO_WriteData8(0x00); // Interface Mode Control (SDO USE)
  #endif
  #elif ILI9488_INTERFACE == 1
  LCD_IO_WriteCmd8(ILI9488_PIXFMT); LCD_IO_WriteData8(0x55); // Interface Pixel Format (16 bit)
  #endif
  LCD_IO_WriteCmd8(ILI9488_FRMCTR1); LCD_IO_WriteData8(0xA0); // Frame rate (60Hz)
  LCD_IO_WriteCmd8(ILI9488_INVCTR); LCD_IO_WriteData8(0x02); // Display Inversion Control (2-dot)
  LCD_IO_WriteCmd8MultipleData8(ILI9488_DFUNCTR, (uint8_t *)"\x02\x02", 2); // Display Function Control RGB/MCU Interface Control
  LCD_IO_WriteCmd8(ILI9488_IMGFUNCT); LCD_IO_WriteData8(0x00); // Set Image Functio (Disable 24 bit data)
  LCD_IO_WriteCmd8MultipleData8(ILI9488_ADJCTR3, (uint8_t *)"\xA9\x51\x2C\x82", 4); // Adjust Control (D7 stream, loose)
  LCD_Delay(5);
  LCD_IO_WriteCmd8(ILI9488_SLPOUT);      // Exit Sleep
  LCD_Delay(120);
  LCD_IO_WriteCmd8(ILI9488_DISPON);      // Display on
  LCD_Delay(5);
  LCD_IO_WriteCmd8(ILI9488_MADCTL); LCD_IO_WriteData8(ILI9488_MAD_DATA_RIGHT_THEN_DOWN);
  //LCD_IO_WriteCmd8(ILI9488_MADCTL); LCD_IO_WriteData8(ILI9488_MAD_DATA_RIGHT_THEN_UP);
  LCD_IO_WriteCmd8(ILI9488_INVON);
  //ILI9488_Set_Rotation(3);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Get the LCD pixel Width.
  * @param  None
  * @retval The Lcd Pixel Width
  */
uint16_t ili9488_GetLcdPixelWidth(void)
{
  return ILI9488_MAX_X + 1;
}

//-----------------------------------------------------------------------------
/**
  * @brief  Get the LCD pixel Height.
  * @param  None
  * @retval The Lcd Pixel Height
  */
uint16_t ili9488_GetLcdPixelHeight(void)
{
  return ILI9488_MAX_Y + 1;
}


ILI9488_DRV::ILI9488_DRV() {
	//memcpy(&modem_n502l,0,sizeof(modem_n502l));
}

ILI9488_DRV::~ILI9488_DRV() {
}

/*HARDWARE RESET*/
void ILI9488_DRV::ILI9488_Reset(void) {
	LCD_RST_OFF;                          /* RST = 1 */
	LCD_Delay(20);
	LCD_RST_ON;                           /* RST = 0 */
	LCD_Delay(1);
	LCD_RST_OFF;                          /* RST = 1 */

}

/*Initialize LCD display*/
void ILI9488_DRV::ILI9488_DRV_Init(void) {
	ILI9488_Init();
}

void ILI9488_DRV::ILI9488_WriteData_16Bit(uint16_t Data) {
	LCD_IO_WriteData8(Data >> 8);
	LCD_IO_WriteData8(Data);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Sets a display window
  * @param  Xpos:   specifies the X bottom left position.
  * @param  Ypos:   specifies the Y bottom left position.
  * @param  Height: display window height.
  * @param  Width:  display window width.
  * @retval None
  */
void ili9488_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
 // ILI9488_LCDMUTEX_PUSH();
  #if ILI9488_INTERFACE == 0
  yStart = Ypos; yEnd = Ypos + Height - 1;
  LCD_IO_WriteCmd8(ILI9488_CASET); LCD_IO_WriteData16_to_2x8(Xpos); LCD_IO_WriteData16_to_2x8(Xpos + Width - 1);
  LCD_IO_WriteCmd8(ILI9488_PASET); LCD_IO_WriteData16_to_2x8(Ypos); LCD_IO_WriteData16_to_2x8(Ypos + Height - 1);
  #elif ILI9488_INTERFACE == 1
  #if (LCD_ORIENTATION == 0)
  LCD_IO_WriteCmd8(ILI9488_CASET); LCD_IO_WriteData16_to_2x8(ILI9488_LCD_PIXEL_WIDTH - Width - Xpos); LCD_IO_WriteData16_to_2x8(ILI9488_LCD_PIXEL_WIDTH - 1 - Xpos);
  LCD_IO_WriteCmd8(ILI9488_PASET); LCD_IO_WriteData16_to_2x8(Ypos); LCD_IO_WriteData16_to_2x8(Ypos + Height - 1);
  #elif (LCD_ORIENTATION == 1)
  LCD_IO_WriteCmd8(ILI9488_CASET); LCD_IO_WriteData16_to_2x8(Xpos); LCD_IO_WriteData16_to_2x8(Xpos + Width - 1);
  LCD_IO_WriteCmd8(ILI9488_PASET); LCD_IO_WriteData16_to_2x8(Ypos); LCD_IO_WriteData16_to_2x8(Ypos + Height - 1);
  #elif (LCD_ORIENTATION == 2)
  LCD_IO_WriteCmd8(ILI9488_CASET); LCD_IO_WriteData16_to_2x8(Xpos); LCD_IO_WriteData16_to_2x8(Xpos + Width - 1);
  LCD_IO_WriteCmd8(ILI9488_PASET); LCD_IO_WriteData16_to_2x8(ILI9488_LCD_PIXEL_HEIGHT - Height - Ypos); LCD_IO_WriteData16_to_2x8(ILI9488_LCD_PIXEL_HEIGHT - 1 - Ypos);
  #else
  //LCD_IO_WriteCmd8(ILI9488_CASET); LCD_IO_WriteData16_to_2x8(ILI9488_LCD_PIXEL_HEIGHT - Width - Xpos); LCD_IO_WriteData16_to_2x8(ILI9488_LCD_PIXEL_HEIGHT - 1 - Xpos);
 // LCD_IO_WriteCmd8(ILI9488_PASET); LCD_IO_WriteData16_to_2x8(ILI9488_LCD_PIXEL_WIDTH - Height - Ypos); LCD_IO_WriteData16_to_2x8(ILI9488_LCD_PIXEL_WIDTH - 1 - Ypos);
  LCD_IO_WriteCmd8(ILI9488_CASET); LCD_IO_WriteData16_to_2x8(Xpos); LCD_IO_WriteData16_to_2x8(Xpos + Width - 1);
  LCD_IO_WriteCmd8(ILI9488_PASET); LCD_IO_WriteData16_to_2x8(Ypos); LCD_IO_WriteData16_to_2x8(Ypos + Height - 1);
  #endif
  #endif

  //ILI9488_LCDMUTEX_POP();
}

/* Set Address - Location block - to draw into */
void ILI9488_DRV::ILI9488_Set_Address(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2) {
	//printf("ILI9341_Set_Address [%d][%d]-[%d][%d]\r\n",X1,Y1,X2,Y2);
	//ili9488_SetDisplayWindow(X1, Y1, X2, Y2);
	LCD_IO_WriteCmd8(0x2A);
	LCD_IO_WriteData8(X1 >> 8);
	LCD_IO_WriteData8(X1);
	LCD_IO_WriteData8(X2 >> 8);
	LCD_IO_WriteData8(X2);
	LCD_IO_WriteCmd8(0x2B);
	LCD_IO_WriteData8(Y1 >> 8);
	LCD_IO_WriteData8(Y1);
	LCD_IO_WriteData8(Y2 >> 8);
	LCD_IO_WriteData8(Y2);
	//LCD_IO_WriteCmd8(0x2C);
}


//
///*Ser rotation of the screen - changes x0 and y0*/
void ILI9488_DRV::ILI9488_Set_Rotation(uint8_t Rotation) {

uint8_t screen_rotation = Rotation;
//uint16_t LCD_WIDTH;
//uint16_t LCD_HEIGHT;
	LCD_IO_WriteCmd8(ILI9488_MADCTL);
	HAL_Delay(1);
	switch (screen_rotation) {
	case SCREEN_VERTICAL_1:
		LCD_IO_WriteData8(0x40 | 0x08);
		LCD_WIDTH = 320;	//240;
		LCD_HEIGHT = 480;	//320;
		break;
	case SCREEN_HORIZONTAL_1:
		LCD_IO_WriteData8(0x20 | 0x08);
		LCD_WIDTH = 480;	//320;
		LCD_HEIGHT = 320;	//240;
		break;
	case SCREEN_VERTICAL_2:
		LCD_IO_WriteData8(0x80 | 0x08);
		LCD_WIDTH = 320;	//240;
		LCD_HEIGHT = 480;	//320;
		break;
	case SCREEN_HORIZONTAL_2:
		LCD_IO_WriteData8(0x40 | 0x80 | 0x20 | 0x08);
		LCD_WIDTH = 480;	//320;
		LCD_HEIGHT = 320;	//240;
		break;
	default:
		//EXIT IF SCREEN ROTATION NOT VALID!
		break;
	}

}


//INTERNAL FUNCTION OF LIBRARY, USAGE NOT RECOMENDED, USE Draw_Pixel INSTEAD
/*Sends single pixel colour information to LCD*/
void ILI9488_DRV::ILI9488_Draw_Colour(uint16_t color) {
//SENDS COLOUR
//	unsigned char TempBuffer[3];	 // = {color>>8, color};
//	TempBuffer[0] = color.r;
//	TempBuffer[1] = color.g;
//	TempBuffer[2] = color.b;
//
//	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
//	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
////HAL_SPI_Transmit(HSPI_INSTANCE, TempBuffer, 2, 1);
//	HAL_SPI_Transmit(HSPI_INSTANCE, TempBuffer, 3, 1);
//	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

//INTERNAL FUNCTION OF LIBRARY
/*Sends block colour information to LCD*/
void ILI9488_DRV::ILI9488_Draw_Colour_Burst(uint16_t color, uint32_t Size) {
	LCD_IO_WriteCmd8DataFill16(ILI9488_RAMWR, color, Size);
}

//FILL THE ENTIRE SCREEN WITH SELECTED COLOUR (either #define-d ones or custom 16bit)
/*Sets address (entire screen) and Sends Height*Width ammount of colour information to LCD*/

//void ILI9341_Draw_Colour_Burst(color_t color, uint32_t Size)
void ILI9488_DRV::ILI9488_Fill_Screen(uint16_t color) {
	//ili9488_DrawRGBImage_Flash(0, 0, 0, 480, 320);
	ILI9488_FillRect(0,0,480, 320, color);
}

void ILI9488_DRV::ILI9488_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t color)
{
 //ILI9488_LCDMUTEX_PUSH();
 ILI9488_Set_Address(Xpos, Ypos, Xpos+Xsize, Ypos+Ysize);
  #if ILI9488_INTERFACE == 0
  LCD_IO_WriteCmd8(ILI9488_RAMWR);
  uint32_t XYsize = Xsize * Ysize;
  while(XYsize--)
    ili9488_write16to24(RGBCode);
  #elif ILI9488_INTERFACE == 1
  LCD_IO_WriteCmd8DataFill16(ILI9488_RAMWR, color, Xsize * Ysize);
  #endif
 //ILI9488_LCDMUTEX_POP();
}

//DRAW PIXEL AT XY POSITION WITH SELECTED COLOUR
//
//Location is dependant on screen orientation. x0 and y0 locations change with orientations.
//Using pixels to draw big simple structures is not recommended as it is really slow
//Try using either rectangles or lines if possible
//
void ILI9488_DRV::ILI9488_Draw_Pixel(uint16_t Xpos, uint16_t Ypos, uint16_t color) {
//	  ILI9488_LCDMUTEX_PUSH();
	  ILI9488_SETCURSOR(Xpos, Ypos);
	  LCD_IO_WriteCmd8(ILI9488_RAMWR);
	  LCD_IO_WriteData16(color);
//	  ILI9488_LCDMUTEX_POP();
}

//DRAW RECTANGLE OF SET SIZE AND HEIGTH AT X and Y POSITION WITH CUSTOM COLOUR
//
//Rectangle is hollow. X and Y positions mark the upper left corner of rectangle
//As with all other draw calls x0 and y0 locations dependant on screen orientation
//

void ILI9488_DRV::ILI9488_Draw_Rectangle(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t color) {
	  //ILI9488_LCDMUTEX_PUSH();
	  ILI9488_Set_Address(X, Y, X + Width - 1, Y + Height - 1);
	  LCD_IO_WriteCmd8DataFill16(ILI9488_RAMWR, color, Width * Height);
	  //ILI9488_LCDMUTEX_POP();

}

//DRAW LINE FROM X,Y LOCATION to X+Width,Y LOCATION
void ILI9488_DRV::ILI9488_Draw_Horizontal_Line(uint16_t X, uint16_t Y, uint16_t Width, uint16_t color) {

	uint16_t LCD_WIDTH=480;
	uint16_t LCD_HEIGHT=320;

	if ((X >= LCD_WIDTH) || (Y >= LCD_HEIGHT))
		return;
	if ((X + Width - 1) >= LCD_WIDTH)
		Width = LCD_WIDTH - X;
	ILI9488_Set_Address(X, Y, X + Width - 1, Y);
	ILI9488_Draw_Colour_Burst(color, Width);
}

//DRAW LINE FROM X,Y LOCATION to X,Y+Height LOCATION
void ILI9488_DRV::ILI9488_Draw_Vertical_Line(uint16_t X, uint16_t Y, uint16_t Height, uint16_t color) {
	uint16_t LCD_WIDTH=480;
	uint16_t LCD_HEIGHT=320;

	if ((X >= LCD_WIDTH) || (Y >= LCD_HEIGHT))
		return;
	if ((Y + Height - 1) >= LCD_HEIGHT) {
		Height = LCD_HEIGHT - Y;
	}
	ILI9488_Set_Address(X, Y, X, Y + Height - 1);
	ILI9488_Draw_Colour_Burst(color, Height);
}

void ILI9488_DRV::LCD_Test(void) {
	printf("ili9488_test..!!\n");
}

/*Draw hollow circle at X,Y location with specified radius and colour. X and Y represent circles center */
void ILI9488_DRV::Draw_Hollow_Circle_Width(uint16_t X, uint16_t Y, uint16_t Radius, uint16_t width, uint16_t color) {
	int i;
	for(i=0;i<width;i++){
		Draw_Hollow_Circle(X,Y,Radius-i,color);
	}
}

/*Draw hollow circle at X,Y location with specified radius and colour. X and Y represent circles center */
void ILI9488_DRV::Draw_Hollow_Circle(uint16_t X, uint16_t Y, uint16_t Radius,
		uint16_t color) {
	int x = Radius - 1;
	int y = 0;
	int dx = 1;
	int dy = 1;
	int err = dx - (Radius << 1);

	while (x >= y) {
		ILI9488_Draw_Pixel(X + x, Y + y, color);
		ILI9488_Draw_Pixel(X + y, Y + x, color);
		ILI9488_Draw_Pixel(X - y, Y + x, color);
		ILI9488_Draw_Pixel(X - x, Y + y, color);
		ILI9488_Draw_Pixel(X - x, Y - y, color);
		ILI9488_Draw_Pixel(X - y, Y - x, color);
		ILI9488_Draw_Pixel(X + y, Y - x, color);
		ILI9488_Draw_Pixel(X + x, Y - y, color);

		if (err <= 0) {
			y++;
			err += dy;
			dy += 2;
		}
		if (err > 0) {
			x--;
			dx += 2;
			err += (-Radius << 1) + dx;
		}
	}
}

/*Draw filled circle at X,Y location with specified radius and colour. X and Y represent circles center */
void ILI9488_DRV::Draw_Filled_Circle(uint16_t X, uint16_t Y, uint16_t Radius, uint16_t color) {

	int x = Radius;
	int y = 0;
	int xChange = 1 - (Radius << 1);
	int yChange = 0;
	int radiusError = 0;

	while (x >= y) {
		for (int i = X - x; i <= X + x; i++) {
			ILI9488_Draw_Pixel(i, Y + y, color);
			ILI9488_Draw_Pixel(i, Y - y, color);
		}
		for (int i = X - y; i <= X + y; i++) {
			ILI9488_Draw_Pixel(i, Y + x, color);
			ILI9488_Draw_Pixel(i, Y - x, color);
		}

		y++;
		radiusError += yChange;
		yChange += 2;
		if (((radiusError << 1) + xChange) > 0) {
			x--;
			radiusError += xChange;
			xChange += 2;
		}
	}
	//Really slow implementation, will require future overhaul
	//TODO:	https://stackoverflow.com/questions/1201200/fast-algorithm-for-drawing-filled-circles
}

/*Draw a hollow rectangle between positions X0,Y0 and X1,Y1 with specified colour*/
void ILI9488_DRV::Draw_Hollow_Rectangle_Coord(uint16_t X0, uint16_t Y0,
		uint16_t X1, uint16_t Y1, uint16_t color) {
	uint16_t X_length = 0;
	uint16_t Y_length = 0;
	uint8_t Negative_X = 0;
	uint8_t Negative_Y = 0;
	float Calc_Negative = 0;

	Calc_Negative = X1 - X0;
	if (Calc_Negative < 0)
		Negative_X = 1;
	Calc_Negative = 0;

	Calc_Negative = Y1 - Y0;
	if (Calc_Negative < 0)
		Negative_Y = 1;

	//DRAW HORIZONTAL!
	if (!Negative_X) {
		X_length = X1 - X0;
	} else {
		X_length = X0 - X1;
	}
	ILI9488_Draw_Horizontal_Line(X0, Y0, X_length, color);
	ILI9488_Draw_Horizontal_Line(X0, Y1, X_length, color);

	//DRAW VERTICAL!
	if (!Negative_Y) {
		Y_length = Y1 - Y0;
	} else {
		Y_length = Y0 - Y1;
	}
	ILI9488_Draw_Vertical_Line(X0, Y0, Y_length, color);
	ILI9488_Draw_Vertical_Line(X1, Y0, Y_length, color);

	if ((X_length > 0) || (Y_length > 0)) {
		ILI9488_Draw_Pixel(X1, Y1, color);
	}

}

/*Draw a filled rectangle between positions X0,Y0 and X1,Y1 with specified colour*/
void ILI9488_DRV::ILI9488_Draw_Filled_Rectangle_Coord(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, uint16_t color) {
	uint16_t X_length = 0;
	uint16_t Y_length = 0;
	uint8_t Negative_X = 0;
	uint8_t Negative_Y = 0;
	int32_t Calc_Negative = 0;

	uint16_t X0_true = 0;
	uint16_t Y0_true = 0;

	Calc_Negative = X1 - X0;
	if (Calc_Negative < 0)
		Negative_X = 1;
	Calc_Negative = 0;

	Calc_Negative = Y1 - Y0;
	if (Calc_Negative < 0)
		Negative_Y = 1;

	//DRAW HORIZONTAL!
	if (!Negative_X) {
		X_length = X1 - X0;
		X0_true = X0;
	} else {
		X_length = X0 - X1;
		X0_true = X1;
	}

	//DRAW VERTICAL!
	if (!Negative_Y) {
		Y_length = Y1 - Y0;
		Y0_true = Y0;
	} else {
		Y_length = Y0 - Y1;
		Y0_true = Y1;
	}

	ILI9488_Draw_Rectangle(X0_true, Y0_true, X_length, Y_length, color);
}

//void ILI9488_DRV::_drawFastVLine(int16_t x, int16_t y, int16_t h,
//		int16_t color) {
//	ILI9488_Draw_Vertical_Line(x, y, h, color);
//}

//void ILI9488_DRV::_drawFastHLine(int16_t x, int16_t y, int16_t w,
//		int16_t color) {
//	ILI9488_Draw_Horizontal_Line(x, y, w, color);
//}

//void ILI9488_DRV::_fillRect(int16_t x, int16_t y, int16_t w, int16_t h,	int16_t color) {
//	// clipping
//	ILI9488_Draw_Filled_Rectangle_Coord(x, y, x + w, y + h, color);
//}

#if 0
/*Draws a character (fonts imported from fonts.h) at X,Y location with specified font colour, size and Background colour*/
/*See fonts.h implementation of font on what is required for changing to a different font when switching fonts libraries*/
//5x5 font
void ILI9488_DRV::ILI9341_Draw_Char(char Character, uint16_t X, uint16_t Y,	uint16_t color, uint16_t Size, uint16_t Background_Colour) {
	uint8_t function_char;
	uint8_t i, j;

	function_char = Character;
	//if(Size==3){
	//  printf("1-X:%d Y:%d [%c]\r\n",X,Y,function_char);
	//}

	if (function_char < ' ') {
		Character = 0;
	} else {
		function_char -= 32;
	}

	char temp[CHAR_WIDTH];
	for (uint8_t k = 0; k < CHAR_WIDTH; k++) {
		temp[k] = font[function_char][k];
	}
	// Draw pixels
	ILI9488_Draw_Rectangle(X, Y, CHAR_WIDTH * Size, CHAR_HEIGHT * Size, 	Background_Colour);
	for (j = 0; j < CHAR_WIDTH; j++) {
		for (i = 0; i < CHAR_HEIGHT; i++) {
			if (temp[j] & (1 << i)) {
				if (Size == 1) {
					ILI9488_Draw_Pixel(X + j, Y + i, color);
				} else {
					ILI9488_Draw_Rectangle(X + (j * Size), Y + (i * Size), Size,
							Size, color);
				}
			}
		}
	}
}

/*Draws an array of characters (fonts imported from fonts.h) at X,Y location with specified font colour, size and Background colour*/
/*See fonts.h implementation of font on what is required for changing to a different font when switching fonts libraries*/
void ILI9488_DRV::ILI9341_Draw_Text(const char *Text, uint16_t X, uint16_t Y,
		uint16_t color, uint16_t Size, uint16_t Background_Colour) {
	while (*Text) {
		ILI9341_Draw_Char(*Text++, X, Y, color, Size, Background_Colour);
		X += CHAR_WIDTH * Size;
		//printf("ILI9341_Draw_Text X:%d\r\n",X);
	}
}
#endif

void ILI9488_DRV::DrawLine( int x0, int y0, int x1, int y1, uint16_t color)
{

    int x = x1-x0;
    int y = y1-y0;
    int dx = abs(x), sx = x0<x1 ? 1 : -1;
    int dy = -abs(y), sy = y0<y1 ? 1 : -1;
    int err = dx+dy, e2;                                                /* error value e_xy             */
    for (;;){                                                           /* loop                         */
        //setPixel(x0,y0,color);
        ILI9488_Draw_Pixel(x0,y0,color);
        e2 = 2*err;
        if (e2 >= dy) {                                                 /* e_xy+e_x > 0                 */
            if (x0 == x1) break;
            err += dy; x0 += sx;
        }
        if (e2 <= dx) {                                                 /* e_xy+e_y < 0                 */
            if (y0 == y1) break;
            err += dx; y0 += sy;
        }
    }

}



void ILI9488_DRV::ILI9488_Draw_VLineW(uint16_t x, uint16_t y, uint16_t length, uint16_t w, uint16_t color) {
	ILI9488_Draw_Filled_Rectangle_Coord(x, y, x+w, y+length, color);
}
void ILI9488_DRV::ILI9488_Draw_HLineW(uint16_t x, uint16_t y, uint16_t length, uint16_t w, uint16_t color) {
	ILI9488_Draw_Filled_Rectangle_Coord(x, y, x+length, y+w, color);
}

void ILI9488_DRV::ILI9488_Draw_RGB565(const char *Image_Array, uint16_t X,
		uint16_t Y, uint16_t width, uint16_t line) {
//	uint16_t oneLineDataSz, oneLinePushSz;
//	//uint16_t block, push_sz;
//	uint16_t i, ix, iy, rgb565;
//	uint32_t k;
//	color_t color;
//	uint8_t tmp[4];
//	//uint8_t* push_buffer;//[(width*3)+10];
//	//uint8_t* buf;//[(width*3)+10];
//	uint8_t push_buffer[(240 * 3) + 10];
//
//	//uint8_t push_bufferx[480];
//	//memset(push_buffer,0,sizeof(push_buffer));
//	ILI9488_Set_Address(X, Y, X + width - 1, Y + line);	////
//	oneLineDataSz = width * 2;	//byte/pixel 16bit 565RGB
//	oneLinePushSz = width * 3;	//byte/pixel 24bit 888RGB
//
//	//HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
//	//HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
//	k = 0;
//	for (i = 0; i < line; i++) {
//		k = oneLineDataSz * i;
//		for (ix = 0, iy = 0; ix < oneLineDataSz; ix += 2) {
//
//			tmp[0] = Image_Array[k + ix];
//			tmp[1] = Image_Array[k + ix + 1];
//			rgb565 = BUILD_UINT16(tmp[1], tmp[0]);
//			//rgb565=0xf800;
//			color.r = (rgb565 >> 11) << 3;
//			color.g = ((rgb565 >> 5) & 0x3f) << 2;
//			color.b = (rgb565 & 0x1f) << 1;
//			push_buffer[iy + 0] = color.r;
//			push_buffer[iy + 1] = color.g;
//			push_buffer[iy + 2] = color.b;
//			iy += 3;
//			//if(i==0 && ix==0) printf("Draw oneline[%x] push_buffer[%x] \r\n",rgb565, push_buffer[0]);
//			//printf("Draw oneline[%d]\r\n",k);
//		}
//		//printf("Draw oneline oneLinePushSz[%d] iy[%d]\r\n",oneLinePushSz, iy);
//		LCD_IO_WriteCmd8DataFill16();
//		HAL_SPI_Transmit(&hspi2, push_buffer, oneLinePushSz, 20);
//		//free(push_buffer);
//	}
//	//HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

/*Draws a full screen picture from flash. Image converted from RGB .jpeg/other to C array using online converter*/
//USING CONVERTER: http://www.digole.com/tools/PicturetoC_Hex_converter.php
//65K colour (2Bytes / Pixel)
void ILI9488_DRV::ILI9488_Draw_Image(const char *Image_Array,
		uint8_t Orientation, uint16_t posX, uint16_t posY, uint16_t imgX,
		uint16_t imgY) {
//	printf("ILI9341_Draw_Image Orientation[%d]\r\n", Orientation);
//#if 1
//	if (Orientation == SCREEN_HORIZONTAL_1) {
//		ILI9488_Set_Rotation(SCREEN_HORIZONTAL_1);
//		ILI9341_Draw_RGB565(Image_Array, posX, posY, imgX, imgY);
//	} else if (Orientation == SCREEN_HORIZONTAL_2) {
//		ILI9488_Set_Rotation(SCREEN_HORIZONTAL_2);
//		ILI9341_Draw_RGB565(Image_Array, posX, posY, imgX, imgY);
//
//	} else if (Orientation == SCREEN_VERTICAL_2) {
//		ILI9488_Set_Rotation(SCREEN_VERTICAL_2);
//		ILI9341_Draw_RGB565(Image_Array, posX, posY, imgX, imgY);
//	} else if (Orientation == SCREEN_VERTICAL_1) {
//		ILI9488_Set_Rotation(SCREEN_VERTICAL_1);
//		ILI9341_Draw_RGB565(Image_Array, posX, posY, imgX, imgY);
//	}
//#endif
}


//-----------------------------------------------------------------------------
//bitmap data에서 사이즈 가져옴
/**
  * @brief  Displays a 16bit bitmap picture..
  * @param  BmpAddress: Bmp picture address.
  * @param  Xpos:  Bmp X position in the LCD
  * @param  Ypos:  Bmp Y position in the LCD
  * @retval None
  * @brief  Draw direction: right then up
  */
void ILI9488_DRV::ILI9488_DrawBitmap(uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp)
{
  uint32_t index = 0, size = 0;
  /* Read bitmap size */
  Ypos += pbmp[22] + (pbmp[23] << 8) - 1;
  size = *(volatile uint16_t *) (pbmp + 2);
  size |= (*(volatile uint16_t *) (pbmp + 4)) << 16;
  /* Get bitmap data address offset */
  index = *(volatile uint16_t *) (pbmp + 10);
  index |= (*(volatile uint16_t *) (pbmp + 12)) << 16;
  size = (size - index)/2;
  pbmp += index;

  //ILI9488_LCDMUTEX_PUSH();

  #if ILI9488_INTERFACE == 0
  LCD_IO_WriteCmd8(ILI9488_MADCTL); LCD_IO_WriteData8(ILI9488_MAD_DATA_RIGHT_THEN_UP);
  LCD_IO_WriteCmd8(ILI9488_PASET); LCD_IO_WriteData16_to_2x8(ILI9488_MAX_Y - yEnd); LCD_IO_WriteData16_to_2x8(ILI9488_MAX_Y - yStart);
  LCD_IO_WriteCmd8(ILI9488_RAMWR);
  while(size--)
  {
    ili9488_write16to24(*(uint16_t *)pbmp);
    pbmp+= 2;
  }
  LCD_IO_WriteCmd8(ILI9488_MADCTL); LCD_IO_WriteData8(ILI9488_MAD_DATA_RIGHT_THEN_DOWN);
  #elif ILI9488_INTERFACE == 1
  LCD_IO_WriteCmd8(ILI9488_MADCTL); LCD_IO_WriteData8(ILI9488_MAD_DATA_RIGHT_THEN_UP);
  LCD_IO_WriteCmd8MultipleData16(ILI9488_RAMWR, (uint16_t *)pbmp, size);
  LCD_IO_WriteCmd8(ILI9488_MADCTL); LCD_IO_WriteData8(ILI9488_MAD_DATA_RIGHT_THEN_DOWN);
  #endif

  //ILI9488_LCDMUTEX_POP();
}

//-----------------------------------------------------------------------------
/**
  * @brief  Displays 16bit/pixel picture..
  * @param  pdata: picture address.
  * @param  Xpos:  Image X position in the LCD
  * @param  Ypos:  Image Y position in the LCD
  * @param  Xsize: Image X size in the LCD
  * @param  Ysize: Image Y size in the LCD
  * @retval None
  * @brief  Draw direction: right then down
  */
void ILI9488_DRV::ILI9488_DrawRGBImage_Flash(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint32_t eprom_add)
{
/*
  uint32_t size,wSize;
  uint32_t r_address;
  uint32_t eep_offset;
  uint8_t buffer[SUBSECTOR_SIZE]={0,};
  uint16_t color;
  //eep_offset=img_id*0x00000;
  eep_offset=eprom_add;
  size = (Xsize * Ysize)*2;
  int sector_num= (size%SUBSECTOR_SIZE) ? (size/SUBSECTOR_SIZE)+1 : (size/SUBSECTOR_SIZE);

  //ILI9488_LCDMUTEX_PUSH();
  ili9488_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
  LCD_CS_ON;
  LCD_CMD8_WRITE(ILI9488_RAMWR);

  for(int i=0;i<sector_num;i++){
	  r_address=(i*SUBSECTOR_SIZE)+eep_offset;
	 // pFlash->flash_read(&hspi1,r_address, buffer, SUBSECTOR_SIZE);
	  if(size<SUBSECTOR_SIZE)wSize=size;
	  else wSize=SUBSECTOR_SIZE;

	  pFlashClass->BSP_W25Qx_Read(buffer, r_address, wSize);
	  for(int ix=0;ix<wSize;ix++)
	  {
		color=buffer[ix]<<8|buffer[ix+1];
		//LCD_DATA8_WRITE(buffer[ix]);
		//LCD_DATA8_WRITE(buffer[ix+1]);
		//if(color==TFT_WHITE)LCD_IO_WriteData16(TFT_WHITE);
		//else LCD_IO_WriteData16(color);
		LCD_IO_WriteData16(color);
		ix++;
	  }
  }
  LCD_CS_OFF;
  //ILI9488_LCDMUTEX_POP();
*/
}

void ILI9488_DRV::ILI9488_DrawEEPROM_Icon(uint16_t Xpos, uint16_t Ypos, uint8_t id)
{
	ILI9488_DrawRGBImage_Flash(Xpos, Ypos, 26, 26, id*2048);
}



void ILI9488_DRV::ILI9488_DrawRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint8_t *pdata)
{
  uint32_t size;

  size = (Xsize * Ysize)*2;

  //ILI9488_LCDMUTEX_PUSH();
  ili9488_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
  #if ILI9488_INTERFACE == 0
  LCD_IO_WriteCmd8(ILI9488_RAMWR);
  while(size--)
  {
    ili9488_write16to24(*pdata);
    pdata++;
  }
  #elif ILI9488_INTERFACE == 1
 // LCD_IO_WriteCmd8MultipleData16(ILI9488_RAMWR, pdata, size);
  LCD_IO_WriteCmd8MultipleData8(ILI9488_RAMWR, pdata, size);

  #endif
  //ILI9488_LCDMUTEX_POP();
}

#if 0

/*****************************************************************************
 * @name       :void GUI_DrawFontTTF(u8 fntsz, u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode)
 * @function   :Display  TTF Bitmap Fnt 24pt/36pt
 * @parameters :
 fntsz 36pt
 x:the bebinning x coordinate of the Chinese character
 y:the bebinning y coordinate of the Chinese character
 fc:the color value of Chinese character
 bc:the background color of Chinese character
 s:the start address of the Chinese character
 mode:0-no overlying,1-overlying
 * @retvalue   :None
 ******************************************************************************/
void ILI9488_DRV::DrawFontTTF24(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc,
		uint8_t *s, uint8_t mode) {
	uint16_t LCD_WIDTH=480;
	uint16_t LCD_HEIGHT=320;
	uint16_t ix, iy, ixx, j;
	uint16_t k;
	//uint16_t HZnum;
	//uint16_t x0 = x;
	uint16_t xA, xpos, ypos;

	uint16_t charHeightByte, charSpaceByte;
//	uint16_t font_pixcel_height;
	uint8_t drw_ot, drw_in;

	int font_index;
	uint16_t font_pixcel_width;
	uint16_t font_pixcel_offset;
	uint16_t font_byte_width;
//	uint16_t font_byte_height;
	uint16_t bitmap_offset;

	uint8_t byte_data;
	uint8_t StartChar;

	FONT_INFO font_info;

	memcpy(&font_info, &godic_24ptFontInfo, sizeof(FONT_INFO));
	drw_ot = 40;
	drw_in = 38;
	charHeightByte = font_info.CharHeight;
	charSpaceByte = font_info.WidthPixcel;
	StartChar = font_info.StartChar;
	//font_pixcel_height = charHeightByte * 8;
	xA = x;
	for (k = 0; k < 32; k++) {
		if (s[k] < 0x20)
			break;
		font_index = (s[k] - StartChar) + 0;
		font_pixcel_width = godic_24ptDescriptors[font_index].info[0];
		font_pixcel_offset = godic_24ptDescriptors[font_index].info[1];
		if (font_index < 0) {
			font_pixcel_width = 10;
			font_pixcel_offset = 0;
		}
		font_byte_width =
				(font_pixcel_width % 8) ?
						(font_pixcel_width / 8) + 1 :
						(font_pixcel_width / 8) + 0;
		bitmap_offset = font_pixcel_offset;

		for (iy = 0; iy < drw_ot; iy++) {
			//if(Ignore_Draw==1)break;
			ypos = y + iy;
			xpos = xA;
			for (ix = 0; ix < font_byte_width; ix++) {
				byte_data = 0x00;
				if (iy >= drw_in || (font_index < 0))
					byte_data = 0x00;
				else {
					byte_data = font_info.Bitmaps[bitmap_offset++];
				}
				for (ixx = 0; ixx < 8; ixx++) {
					if (mode) {
						if (byte_data & 0x80)
							ILI9488_Draw_Pixel(xpos, ypos, fc);     //point draw
					} else {
						if (byte_data & 0x80)
							ILI9488_Draw_Pixel(xpos, ypos, fc);
						else
							ILI9488_Draw_Pixel(xpos, ypos, bc);
					}
					byte_data = (byte_data << 1);
					xpos++;
				}
			}
		}
		xA = xA + font_byte_width * 8;
		ILI9488_Set_Address(x, y, x + (font_byte_width * 8) - 1,
				y + (font_byte_width * 8) - 1);
	}
	ILI9488_Set_Address(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
}
#endif


void ILI9488_DRV::draw16_english(uint8_t ch) {
	ILI9488_Set_Address(_xchar, _ychar, _xchar+(8*_times-1), _ychar+(16*_times-1));
	LCD_CS_ON;
	LCD_CMD8_WRITE(ILI9488_RAMWR);
	for (int i=0; i<2; i++) {
			for (int j=0; j<8; j++) {
				for(int xx=0;xx<_times;xx++){
					 for (int k=0; k<8; k++) {
						uint8_t data =(english[ch][i*8+k])&(1<<j);
						if (data){
							for(int t=0;t<_times;t++)LCD_DATA16_WRITE(fg_color);
						}
						else {
							if(!_nobg){for(int t=0;t<_times;t++)LCD_DATA16_WRITE(bg_color);}
						}
					 }
				}
			}
	}
	LCD_CS_OFF;
	_xchar+=8*_times;
	if(_xchar>(_width-(8*_times))) { _xchar=0; _ychar+=16*_times; }
}

void ILI9488_DRV::draw16_korean(uint16_t hangeul) {
	uint8_t cho1[22] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 3, 1, 2, 4, 4, 4, 2, 1, 3, 0 };
	uint8_t cho2[22] = { 0, 5, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 6, 6, 7, 7, 7, 6, 6, 7, 5 };
	uint8_t jong[22] = { 0, 0, 2, 0, 2, 1, 2, 1, 2, 3, 0, 2, 1, 3, 3, 1, 2, 1, 3, 3, 1, 1 };

	uint8_t first=0, mid=0, last=0;
	uint8_t firstType=0, midType=0, lastType=0;
	uint8_t i;
	uint16_t pF_temp;
	uint8_t Korean_buffer[32]={0,};

  // 초성(20) : 없음ㄱㄲㄴㄷㄸㄹㅁㅂㅃㅅㅆㅇㅈㅉㅊㅋㅌㅍㅎ
  // 중성(22) : 없음ㅏㅐㅑㅒㅓㅔㅕㅖㅗㅘㅙㅚㅛㅜㅝㅞㅟㅠㅡㅢㅣ
  // 종성(28) : 없음ㄱㄲㄳㄴㄵㄶㄷㄹㄺㄻㄼㄽㄾㄿㅀㅁㅂㅄㅅㅆㅇㅈㅊㅋㅌㅍㅎ
  //              가각갂갃간갅갆갇갈갉갊갋갌갍갎갏감갑값갓갔강갖갗갘같갚갛

  hangeul -= 0xAC00;    // 0xAC00='가' ~  0xD7A3='힣' 11,172글자
  last = hangeul % 28;  // 종성
  hangeul /= 28;
  first = hangeul / 21 + 1;  // 초성
  mid   = hangeul % 21 + 1;  // 중성


  if(last==0){  //받침 없는 경우
    firstType = cho1[mid];
    if(first == 1 || first == 24) midType = 0;
    else midType = 1;
  }
  else{       //받침 있는 경우
    firstType = cho2[mid];
    if(first == 1 || first == 24) midType = 2;
    else midType = 3;
    lastType = jong[mid];
  }

  //초성(0~159)
  pF_temp = firstType*20 + first;
  i=32; while(i--) Korean_buffer[i] = (K_font[pF_temp][i]);

  //중성(160~247)
  pF_temp = 160 + midType*22 + mid;
  i=32; while(i--) Korean_buffer[i] |= (K_font[pF_temp][i]);

  //종성(247~359)
  if(last){
    pF_temp = 248 + lastType*28 + last;
    i=32; while(i--) Korean_buffer[i] |= (K_font[pF_temp][i]);
  }

  // TFT-LCD 출력
	ILI9488_Set_Address(_xchar, _ychar, _xchar+(16*_times-1), _ychar+(16*_times-1));
	LCD_CS_ON;
	LCD_CMD8_WRITE(ILI9488_RAMWR);
	for(int i=0; i<16; i++){
		for(int xx=0;xx<_times;xx++){
			uint16_t data = (Korean_buffer[i]<<8) | Korean_buffer[i+16];
			for(int j=0; j<16; j++){
				if (data&0x8000) {
					for(int t=0;t<_times;t++)  LCD_DATA16_WRITE(fg_color);
				}
				else {
					if(!_nobg){for(int t=0;t<_times;t++)LCD_DATA16_WRITE(bg_color);}
				}
				data<<=1;
			}
		}
	}
	LCD_CS_OFF;
	_xchar+=16*_times;
	if(_xchar>(_width-16*_times)) { _xchar=0; _ychar+=16*_times; }
}

int ILI9488_DRV::draw16_string(int x, int y, uint16_t colorfore, uint16_t colorback, char *str, uint8_t timesX, uint8_t mode) {
	int length=0;
	_xchar = x;
	_ychar = y;
	fg_color = colorfore;
	bg_color = colorback;
	_nobg = mode;
	_times = timesX;
  while (*str) {
    uint8_t ch1 = *str++;
    if (ch1 < 0x80) {
    	draw16_english(ch1);  // ASCII
    	length+=(8*_times);
    }
    else {                              // 한글
		uint8_t ch2 = *str++;
		uint8_t ch3 = *str++;
		uint16_t hangeul = (ch1&0x0F)<<12 | (ch2&0x3F)<<6 | (ch3&0x3F);
		draw16_korean(hangeul);
		length+=(16*_times);
    }
  }
  return length;
}

int ILI9488_DRV::draw16_length(char *str, uint8_t timesX) {
	int length=0;
	  while (*str) {
	    uint8_t ch1 = *str++;
	    if (ch1 < 0x80) length=length+(8*timesX);  // ASCII
	    else {                              // 한글
			//uint8_t ch2 = *str++;
			//uint8_t ch3 = *str++;
	    	length=length+(16*timesX);
	    }
	  }
	  //printf("draw16_length[%d]\r\n",length);
	  return length;
}

uint16_t ILI9488_DRV::GetWidthTTF_Digital16(uint8_t *s) {
#if 0
	uint16_t i, ix, iy, ixx;
	uint16_t k;
	uint16_t xA, xpos;
	uint8_t charSpaceSpace=1;
	uint8_t drw_ot, drw_in;
	int font_index;
	uint16_t font_pixcel_width;
	uint16_t font_pixcel_offset;
	uint16_t font_byte_width;
	uint16_t bitmap_offset;
	uint8_t byte_data;
	uint8_t StartChar;

	FONT_INFO font_info;
	uint8_t des_str[128]={0,};
	char utf_str[4]={0,};
	ix=0;
	for (i = 0; i < 128; i++) {
		if(s[i]<0x80) {
			des_str[ix++]=s[i];
			if(s[i]==0) break;
		}
		else{
			for(iy=0; iy<84; iy++){
				memcpy(utf_str, godic_16pt_kor_table[iy], 3);
				if( utf_str[0]==s[i] && utf_str[1]==s[i+1] && utf_str[2]==s[i+2]){
					des_str[ix++]=iy+0x80;
					i+=2;
					break;
				}
			}
		}
	}
	//kor_font;
	memcpy(&font_info, &score_16ptFontInfo, sizeof(FONT_INFO));
	drw_ot = 24;//8*4=32 폰트바이트
	drw_in = 24;//폰트높이
	StartChar = font_info.StartChar;
	xA = 0;
	for (k = 0; k < 128; k++) {
		if (des_str[k] < 0x20) break;
		font_index = (des_str[k] - StartChar) + 0;
		font_pixcel_width = score_16ptDescriptors[font_index].info[0];
		font_pixcel_offset = score_16ptDescriptors[font_index].info[1];
		if (font_index < 0) {
			font_pixcel_width = 5;
			font_pixcel_offset = 0;
		}
		font_byte_width = (font_pixcel_width % 8) ? (font_pixcel_width / 8) + 1 :	(font_pixcel_width / 8) + 0;
		bitmap_offset = font_pixcel_offset;
		for (iy = 0; iy < drw_ot; iy++) {
			//if(Ignore_Draw==1)break;
			xpos = xA;
			for (ix = 0; ix < font_byte_width; ix++) {
				byte_data = 0x00;
				if (iy >= drw_in || (font_index < 0)) byte_data = 0x00;
				else {
					byte_data = font_info.Bitmaps[bitmap_offset++];
				}
				for (ixx = 0; ixx < 8; ixx++) {
					byte_data = (byte_data << 1);
					xpos++;
				}
			}
		}
		xA = xA + (font_byte_width*8)+charSpaceSpace;
	}
	return xA;
#else
	return 0;
#endif
}

#if 0
int ILI9488_DRV::DrawFontTTF_Digital40Num(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc,	uint8_t *s, uint8_t mode) {
	uint16_t LCD_WIDTH=480;
	uint16_t LCD_HEIGHT=320;
	uint16_t i,k, ix, iy, ixx;
	uint16_t xA, xpos, ypos;
	uint8_t  charSpaceSpace=4;
	uint8_t drw_ot, drw_in;
	int font_index;
	uint16_t font_pixcel_width;
	uint16_t font_pixcel_offset;
	uint16_t font_byte_width;
	uint16_t bitmap_offset;
	uint8_t byte_data;
	uint8_t StartChar;
	FONT_INFO font_info;

	memcpy(&font_info, &digitalDisplay_48ptFontInfo, sizeof(FONT_INFO));//48

	drw_ot = 48;//40 8*7 폰트바이트
	drw_in = 42;//폰트높이//40
	StartChar = font_info.StartChar;
	//font_pixcel_height = charHeightByte * 8;
	xA = x;
	for (k = 0; k < 32; k++) {
		if (s[k] < 0x20) break;
		font_index = (s[k] - StartChar) + 0;
		font_pixcel_width = digitalDisplay_48ptDescriptors[font_index].info[0];
		font_pixcel_offset = digitalDisplay_48ptDescriptors[font_index].info[1];
		//font_index=font_index-1;
		if (font_index < 0) {
			font_pixcel_width = 10;
			font_pixcel_offset = 0;
		}
		font_byte_width = (font_pixcel_width % 8) ?	(font_pixcel_width / 8) + 1 :(font_pixcel_width / 8) + 0;
		bitmap_offset = font_pixcel_offset;
		for (iy = 0; iy < drw_ot; iy++) {
//			if(Ignore_Draw==1)break;
			ypos = y + iy;
			xpos = xA;
			for (ix = 0; ix < font_byte_width; ix++) {
				byte_data = 0x00;
				if (iy >= drw_in || (font_index < 0))
					byte_data = 0x00;
				else {
					byte_data = font_info.Bitmaps[bitmap_offset++];
				}
				for (ixx = 0; ixx < 8; ixx++) {
					if (mode) {
						if (byte_data & 0x80) ILI9488_Draw_Pixel(xpos, ypos, fc);     //point draw
					}
					else {
						if (byte_data & 0x80)ILI9488_Draw_Pixel(xpos, ypos, fc);
						else ILI9488_Draw_Pixel(xpos, ypos, bc);
					}
					byte_data = (byte_data << 1);
					xpos++;
				}
			}
			if (mode==0) for(i=0;i<charSpaceSpace;i++) ILI9488_Draw_Pixel(xpos+(i), ypos, bc);
		}
		xA = xA + (font_byte_width * 8)+charSpaceSpace;//자간 띄우기
		ILI9488_Set_Address(x, y, x + (font_byte_width * 8) - 1, y + (font_byte_width * 8) - 1);
	}
	ILI9488_Set_Address(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
	return xA;
}

int ILI9488_DRV::DrawFontTTF_Digital40Nsz(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc,	uint8_t *s, uint8_t mode) {
	uint16_t LCD_WIDTH=480;
	uint16_t LCD_HEIGHT=320;
	uint16_t i,k, ix, iy, ixx;
	uint16_t xA, xpos, ypos;
	uint8_t  charSpaceSpace=4;
	uint8_t drw_ot, drw_in;
	int font_index;
	uint16_t font_pixcel_width;
	uint16_t font_pixcel_offset;
	uint16_t font_byte_width;
	uint16_t bitmap_offset;
	uint8_t byte_data;
	uint8_t StartChar;
	FONT_INFO font_info;

	memcpy(&font_info, &digitalDisplay_48ptFontInfo, sizeof(FONT_INFO));//48
	drw_ot = 48;//40 8*7 폰트바이트
	drw_in = 42;//폰트높이//40
	StartChar = font_info.StartChar;
	//font_pixcel_height = charHeightByte * 8;
	xA = x;
	for (k = 0; k < 32; k++) {
		if (s[k] < 0x20) break;
		font_index = (s[k] - StartChar) + 0;
		font_pixcel_width = digitalDisplay_48ptDescriptors[font_index].info[0];
		font_pixcel_offset = digitalDisplay_48ptDescriptors[font_index].info[1];
		//font_index=font_index-1;
		if (font_index < 0) {
			font_pixcel_width = 10;
			font_pixcel_offset = 0;
		}
		font_byte_width = (font_pixcel_width % 8) ?	(font_pixcel_width / 8) + 1 :(font_pixcel_width / 8) + 0;
		bitmap_offset = font_pixcel_offset;
		for (iy = 0; iy < drw_ot; iy++) {
//			if(Ignore_Draw==1)break;
			ypos = y + iy;
			xpos = xA;
			for (ix = 0; ix < font_byte_width; ix++) {
				byte_data = 0x00;
				if (iy >= drw_in || (font_index < 0))
					byte_data = 0x00;
				else {
					byte_data = font_info.Bitmaps[bitmap_offset++];
				}
				for (ixx = 0; ixx < 8; ixx++) {
//					if (mode) {
//						if (byte_data & 0x80) ILI9341_Draw_Pixel(xpos, ypos, fc);     //point draw
//					}
//					else {
//						if (byte_data & 0x80)ILI9341_Draw_Pixel(xpos, ypos, fc);
//						else ILI9341_Draw_Pixel(xpos, ypos, bc);
//					}
//					byte_data = (byte_data << 1);
//					xpos++;
				}
			}
			//if (mode==0) for(i=0;i<charSpaceSpace;i++) ILI9341_Draw_Pixel(xpos+(i), ypos, bc);
		}
		xA = xA + (font_byte_width * 8)+charSpaceSpace;//자간 띄우기
		//ILI9488_Set_Address(x, y, x + (font_byte_width * 8) - 1, y + (font_byte_width * 8) - 1);
	}
	//ILI9488_Set_Address(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
	return xA;
}

#endif

void ILI9488_DRV::TFT_fillRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color)
{
	ILI9488_Draw_Filled_Rectangle_Coord(x + r, y, x+r+(w - 2 * r), y+h, color);
	// draw four corners
	fillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
	fillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);
}

void ILI9488_DRV::fillCircleHelper(int16_t x0, int16_t y0, int16_t r,
	uint8_t cornername, int16_t delta, int16_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;
	int16_t ylm = x0 - r;

	while (x < y) {
		if (f >= 0) {
			if (cornername & 0x1)
				ILI9488_Draw_Vertical_Line(x0 + y, y0 - x, 2 * x + 1 + delta, color);
			if (cornername & 0x2)
				ILI9488_Draw_Vertical_Line(x0 - y, y0 - x, 2 * x + 1 + delta, color);
			ylm = x0 - y;
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		if ((x0 - x) > ylm) {
			if (cornername & 0x1)
				ILI9488_Draw_Vertical_Line(x0 + x, y0 - y, 2 * y + 1 + delta, color);
			if (cornername & 0x2)
				ILI9488_Draw_Vertical_Line(x0 - x, y0 - y, 2 * y + 1 + delta, color);
		}
	}
}

/*Draw filled circle at X,Y location with specified radius and colour. X and Y represent circles center */
void ILI9488_DRV::ILI9488_Draw_Filled_Circle(uint16_t X, uint16_t Y,uint16_t Radius, uint16_t color) {

	int x = Radius;
	int y = 0;
	int xChange = 1 - (Radius << 1);
	int yChange = 0;
	int radiusError = 0;

	while (x >= y) {
		for (int i = X - x; i <= X + x; i++) {
			ILI9488_Draw_Pixel(i, Y + y, color);
			ILI9488_Draw_Pixel(i, Y - y, color);
		}
		for (int i = X - y; i <= X + y; i++) {
			ILI9488_Draw_Pixel(i, Y + x, color);
			ILI9488_Draw_Pixel(i, Y - x, color);
		}

		y++;
		radiusError += yChange;
		yChange += 2;
		if (((radiusError << 1) + xChange) > 0) {
			x--;
			radiusError += xChange;
			xChange += 2;
		}
	}
	//Really slow implementation, will require future overhaul
	//TODO:	https://stackoverflow.com/questions/1201200/fast-algorithm-for-drawing-filled-circles
}

