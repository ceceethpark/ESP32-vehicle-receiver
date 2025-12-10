#include "LocalLCD.h"

LocalLCD::LocalLCD() {
    currentTextSize = 1;
    tft = nullptr;
}

bool LocalLCD::begin() {
    SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
    
    tft = new Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
    
    if (!tft) {
        return false;
    }
    
    tft->init(SCREEN_WIDTH, SCREEN_HEIGHT);
    tft->setRotation(0);
    tft->fillScreen(BLACK);
    
    printf("LCD initialized\r\n");
    return true;
}

void LocalLCD::clear() {
    if (!tft) return;
    tft->fillScreen(BLACK);
}

void LocalLCD::setBrightness(uint8_t brightness) {
}

void LocalLCD::displayOn() {
    if (!tft) return;
    tft->enableDisplay(true);
}

void LocalLCD::displayOff() {
    if (!tft) return;
    tft->enableDisplay(false);
}

void LocalLCD::printText(const char* text, uint16_t x, uint16_t y, uint16_t color) {
    if (!tft) return;
    tft->setCursor(x, y);
    tft->setTextColor(color);
    tft->setTextSize(currentTextSize);
    tft->print(text);
}

void LocalLCD::printTextCentered(const char* text, uint16_t y, uint16_t color) {
    if (!tft) return;
    int16_t x1, y1;
    uint16_t w, h;
    tft->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    uint16_t x = (SCREEN_WIDTH - w) / 2;
    printText(text, x, y, color);
}

void LocalLCD::setTextSize(uint8_t size) {
    currentTextSize = size;
}

void LocalLCD::showVehicleSpeed(uint8_t speed) {
    if (!tft) return;
    char buf[20];
    sprintf(buf, "Speed: %d km/h  ", speed);
    printText(buf, 10, 70, WHITE);
}

void LocalLCD::showVehicleBattery(uint8_t percentage) {
    if (!tft) return;
    char buf[20];
    sprintf(buf, "Battery: %d%%  ", percentage);
    printText(buf, 10, 100, GREEN);
}

void LocalLCD::showMotorTemp(int16_t temp) {
    if (!tft) return;
    char buf[20];
    sprintf(buf, "M.Temp: %dC  ", temp);
    printText(buf, 10, 130, CYAN);
}

void LocalLCD::showMotorCurrent(uint16_t current) {
    if (!tft) return;
    char buf[20];
    sprintf(buf, "Current: %dmA  ", current);
    printText(buf, 10, 160, YELLOW);
}

void LocalLCD::showFetTemp(int16_t temp) {
    if (!tft) return;
    char buf[20];
    sprintf(buf, "FET: %dC  ", temp);
    printText(buf, 10, 190, MAGENTA);
}

void LocalLCD::drawMainScreen() {
    if (!tft) return;
    clear();
    printTextCentered("Vehicle Receiver", 10, CYAN);
    tft->drawFastHLine(0, 40, SCREEN_WIDTH, GRAY);
}
