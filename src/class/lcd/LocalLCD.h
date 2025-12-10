#ifndef LOCAL_LCD_H
#define LOCAL_LCD_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

class LocalLCD {
public:
    LocalLCD();
    bool begin();
    bool isInitialized() { return (tft != nullptr); }
    
    void clear();
    void setBrightness(uint8_t brightness);
    void displayOn();
    void displayOff();
    
    void printText(const char* text, uint16_t x, uint16_t y, uint16_t color = 0xFFFF);
    void printTextCentered(const char* text, uint16_t y, uint16_t color = 0xFFFF);
    void setTextSize(uint8_t size);
    
    void showVehicleSpeed(uint8_t speed);
    void showVehicleBattery(uint8_t percentage);
    void showMotorTemp(int16_t temp);
    void showMotorCurrent(uint16_t current);
    void showFetTemp(int16_t temp);
    
    void drawMainScreen();
    
    enum Color {
        BLACK = 0x0000,
        WHITE = 0xFFFF,
        RED = 0xF800,
        GREEN = 0x07E0,
        BLUE = 0x001F,
        YELLOW = 0xFFE0,
        CYAN = 0x07FF,
        MAGENTA = 0xF81F,
        GRAY = 0x8410
    };
    
private:
    Adafruit_ST7789* tft;
    uint8_t currentTextSize;
    
    static const uint8_t TFT_CS = 5;
    static const uint8_t TFT_DC = 4;
    static const uint8_t TFT_RST = 15;
    static const uint8_t TFT_MOSI = 23;
    static const uint8_t TFT_SCLK = 18;
    
    static const uint16_t SCREEN_WIDTH = 240;
    static const uint16_t SCREEN_HEIGHT = 320;
};

#endif
