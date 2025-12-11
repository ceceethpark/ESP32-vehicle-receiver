//-----------------------------------
//	ILI9488 Driver library for STM32
//-----------------------------------
#ifndef ILI9488_DRV_H
#define ILI9488_DRV_H
#include "extern.h"
#include "stm32f1xx_hal.h"
#include "typedef.h"

//#define ILI9341_SCREEN_HEIGHT 240 
//#define ILI9341_SCREEN_WIDTH 	320

#define ILI9488_SCREEN_HEIGHT 320 
#define ILI9488_SCREEN_WIDTH 480

//SPI INSTANCE
//#define HSPI_INSTANCE	&hspi2

////CHIP SELECT PIN AND PORT, STANDARD GPIO
//#define LCD_CS_PORT	SPI2_CS_GPIO_Port
//#define LCD_CS_PIN	SPI2_CS_Pin
//
////DATA COMMAND PIN AND PORT, STANDARD GPIO
//#define LCD_DC_PORT	LCD_DC_GPIO_Port
//#define LCD_DC_PIN	LCD_DC_Pin
//
////RESET PIN AND PORT, STANDARD GPIO
//#define	LCD_RST_PORT	pLCD_RESET_GPIO_Port
//#define	LCD_RST_PIN	pLCD_RESET_Pin

//500
#define HORIZONTAL_IMAGE 0
#define VERTICAL_IMAGE	1

#define BURST_MAX_SIZE 	600 
#define SCREEN_VERTICAL_1		0
#define SCREEN_HORIZONTAL_1		1
#define SCREEN_VERTICAL_2		2
#define SCREEN_HORIZONTAL_2		3
//#define tft_color(color) ( (uint16_t)((color >> 8) | (color << 8)) )
#define _swap(a, b) { int16_t t = a; a = b; b = t; }

#define TFT_BLACK       0x0000      /*   0,   0,   0 */
#define TFT_NAVY        0x000F      /*   0,   0, 128 */
#define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define TFT_MAROON      0x7800      /* 128,   0,   0 */
#define TFT_PURPLE      0x780F      /* 128,   0, 128 */
#define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
#define TFT_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define TFT_BLUE        0x001F      /*   0,   0, 255 */
#define TFT_GREEN       0x07E0      /*   0, 255,   0 */
#define TFT_CYAN        0x07FF      /*   0, 255, 255 */
#define TFT_RED         0xF800      /* 255,   0,   0 */
#define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
#define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
#define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
#define TFT_ORANGE      0xFDA0      /* 255, 180,   0 */
#define TFT_GREENYELLOW 0xB7E0      /* 180, 255,   0 */
#define TFT_GRAY 		0x8410
#define TFT_BLUE_SKY    0x051D      /*0, 160, 233 */
#define TFT_WHITE_GRAY  0xE73D      /*231, 231, 233 */
#define TFT_PINK        0xF81F
#define MINJI_GRAY  0x8c51
#define MINJI_RED   0xfa49
#define MINJI_GREEN 0x0ea0

#define MAIN_BG_COLOR    	TFT_WHITE //TFT_WHITE //0xFD20
#define MEASURE_DISP_COLOR  TFT_BLACK
#define SELJUNG_DISP_COLOR  TFT_BLUE
#define NICK_NAME_COLOR     TFT_GREEN //MAIN_BG_COLOR
#define MEASURE_DISP_COLOR  TFT_BLACK
#define LEVEL_CLEAR_COLOR   TFT_DARKGREY
#define LEVEL_NORMAL_COLOR    MINJI_GREEN
#define LEVEL_ABNORMAL_COLOR  TFT_RED

typedef struct
{
  uint16_t info[2];
}FONT_CHAR_INFO;

typedef struct
{
  uint16_t CharHeight;
  uint16_t StartChar;
  uint16_t EndChar;
  uint16_t WidthPixcel;
  const FONT_CHAR_INFO *Descriptors;
  const uint8_t *Bitmaps;
}FONT_INFO;


extern uint16_t LCD_HEIGHT;
extern uint16_t LCD_WIDTH;



class ILI9488_DRV
{
    private:
	  int _xchar;
	  int _ychar;
	  int _width = 480;
	  int _height = 320;
	  uint8_t _nobg;
	  uint8_t _times;
	  uint16_t fg_color;
	  uint16_t bg_color;
      void draw16_english(uint8_t ch);
      void draw16_korean(uint16_t hangeul);
    public:
	 // uint8_t Ignore_Draw=0;
      ILI9488_DRV();
      virtual ~ILI9488_DRV();
      uint16_t rgb888torgb565(uint8_t red,  uint8_t green, uint8_t blue);
      void LCD_BACK_LIGHT(uint8_t on);
      void ILI9488_SPI_Init(void);
      void ILI9488_SPI_Send(unsigned char SPI_Data);
      void ILI9488_WriteData_16Bit(uint16_t Data);
      void ILI9488_Set_Address(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2);
      void ILI9488_Reset(void);
      void ILI9488_Enable(void);
      void ILI9488_DRV_Init(void);
//-------------- ++
      void ILI9488_Init(void);
      void ILI9488_Set_Rotation(uint8_t Rotation);
      void ILI9488_Fill_Screen(uint16_t color);
      void ILI9488_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t color);
      void ILI9488_Draw_Rectangle(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t color);
      void ILI9488_Draw_Filled_Circle(uint16_t X, uint16_t Y, uint16_t Radius, uint16_t color);
      void ILI9488_Draw_Filled_Rectangle_Coord(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, uint16_t color);
      //void ILI9488_Draw_Filled_Rectangle_Coord(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, color_t color);
      void ILI9488_DrawBitmap(uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp);
      void ILI9488_DrawRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint8_t *pdata);
      void ILI9488_Draw_Pixel(uint16_t Xpos, uint16_t Ypos, uint16_t color);
      void ILI9488_Draw_VLineW(uint16_t x, uint16_t y, uint16_t length, uint16_t w, uint16_t color);
      void ILI9488_Draw_HLineW(uint16_t x, uint16_t y, uint16_t length, uint16_t w, uint16_t color);
      void ILI9488_Draw_Image(const char* Image_Array, uint8_t Orientation, uint16_t posX, uint16_t posY, uint16_t imgX, uint16_t imgY);
      void ILI9488_DrawRGBImage_Flash(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint32_t eprom_add);
      void ILI9488_Draw_RGB565(const char* Image_Array, uint16_t X, uint16_t Y, uint16_t width, uint16_t line);
      void ILI9488_DrawEEPROM_Icon(uint16_t Xpos, uint16_t Ypos, uint8_t id);
      void ILI9488_Draw_Horizontal_Line(uint16_t X, uint16_t Y, uint16_t Width, uint16_t color);
      void ILI9488_Draw_Vertical_Line(uint16_t X, uint16_t Y, uint16_t Height, uint16_t color);
      void ILI9488_Draw_Colour_Burst(uint16_t color, uint32_t Size);
      void ILI9488_Draw_Colour(uint16_t color);
      void Draw_Hollow_Rectangle_Coord(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, uint16_t color);
      void TFT_fillRoundRect(int16_t x, int16_t y, uint16_t w,	uint16_t h, uint16_t r, uint16_t color);
      void Draw_Hollow_Circle(uint16_t X, uint16_t Y, uint16_t Radius, uint16_t color);
      void Draw_Hollow_Circle_Width(uint16_t X, uint16_t Y, uint16_t Radius, uint16_t width, uint16_t color);
      void Draw_Filled_Circle(uint16_t X, uint16_t Y, uint16_t Radius, uint16_t color);
      void DrawLine( int x0, int y0, int x1, int y1, uint16_t color);
      uint16_t GetWidthTTF_Digital16(uint8_t *s);
      void fillCircleHelper(int16_t x0, int16_t y0, int16_t r,	uint8_t cornername, int16_t delta, int16_t color);
      void LCD_Test(void);
      //int DrawFontTTF_Digital40Num(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s, uint8_t mode);
      //int DrawFontTTF_Digital40Nsz(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s, uint8_t mode);
      int draw16_string(int x, int y, uint16_t colorfore, uint16_t colorback, char *str, uint8_t timesX, uint8_t mode);
      int draw16_length(char *str, uint8_t timesX);
};

#endif

