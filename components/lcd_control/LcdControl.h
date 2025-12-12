#ifndef LCD_CONTROL_H
#define LCD_CONTROL_H

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

class LcdControl {
public:
    // LCD 크기
    static constexpr uint16_t LCD_WIDTH = 240;
    static constexpr uint16_t LCD_HEIGHT = 320;
    
    // 색상 (RGB565)
    enum Color : uint16_t {
        BLACK = 0x0000,
        WHITE = 0xFFFF,
        RED = 0xF800,
        GREEN = 0x07E0,
        BLUE = 0x001F,
        YELLOW = 0xFFE0,
        CYAN = 0x07FF,
        MAGENTA = 0xF81F,
        GRAY = 0x8410,
        ORANGE = 0xFD20
    };
    
    // 차량 데이터
    struct VehicleData {
        uint8_t speed;
        uint8_t battery;
        int16_t motor_temp;
        uint16_t motor_current;
        int16_t fet_temp;
        char mode[16];
        bool connected;
        bool data_changed;
    };
    
    // 생성자/소멸자
    LcdControl(spi_host_device_t spi_host = SPI2_HOST,
               gpio_num_t cs_pin = GPIO_NUM_5,
               gpio_num_t dc_pin = GPIO_NUM_4,
               gpio_num_t rst_pin = GPIO_NUM_15,
               gpio_num_t bl_pin = GPIO_NUM_13);
    ~LcdControl();
    
    // 초기화
    esp_err_t begin();
    void end();
    
    // 통합 초기화 (초기 화면 표시 포함)
    esp_err_t initialize(const char* initial_mode, bool connection_status);
    bool isInitialized() const { return initialized_; }
    
    // 기본 제어
    void clear(Color color = BLACK);
    void setBrightness(uint8_t brightness); // 0-255
    void displayOn();
    void displayOff();
    
    // 그리기 함수
    void drawPixel(uint16_t x, uint16_t y, Color color);
    void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, Color color);
    void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, Color color);
    void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, Color color);
    void drawCircle(uint16_t x, uint16_t y, uint16_t r, Color color);
    void fillCircle(uint16_t x, uint16_t y, uint16_t r, Color color);
    
    // 텍스트 (간단한 구현)
    void printText(const char* text, uint16_t x, uint16_t y, Color color = WHITE, uint8_t size = 2);
    void printTextCentered(const char* text, uint16_t y, Color color = WHITE, uint8_t size = 2);
    
    // 한글 지원 텍스트
    void printKoreanText(const char* text, uint16_t x, uint16_t y, Color fg_color, Color bg_color = BLACK, uint8_t size = 1, bool no_bg = false);
    
    // UI 화면
    void drawMainScreen();
    void updateVehicleDisplay(const VehicleData& data);
    
    // UI Task 관리
    esp_err_t startUITask(uint32_t stack_size = 4096, UBaseType_t priority = 3);
    void stopUITask();
    bool isUITaskRunning() const { return ui_task_running_; }
    
    // 데이터 업데이트 (thread-safe)
    void updateVehicleData(uint8_t speed, uint8_t battery, int16_t motor_temp,
                          uint16_t motor_current, int16_t fet_temp);
    void updateControlMode(const char* mode);
    void updateConnectionStatus(bool connected);
    
    // UI 렌더링 (외부 태스크에서 호출 가능)
    void renderUI();
    
private:
    // SPI 설정
    spi_host_device_t spi_host_;
    spi_device_handle_t spi_handle_;
    gpio_num_t cs_pin_;
    gpio_num_t dc_pin_;
    gpio_num_t rst_pin_;
    gpio_num_t bl_pin_;
    
    // 상태
    bool initialized_;
    uint8_t brightness_;
    
    // UI Task
    TaskHandle_t ui_task_handle_;
    bool ui_task_running_;
    VehicleData vehicle_data_;
    SemaphoreHandle_t data_mutex_;
    
    // SPI 전송
    void spiWrite(const uint8_t* data, size_t len);
    void writeCommand(uint8_t cmd);
    void writeData(uint8_t data);
    void writeData16(uint16_t data);
    
    // 초기화 시퀀스
    void initSequence();
    void setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    
    // 내부 렌더링 함수
    void drawStatusBar();
    void drawVehicleInfo();
    
    // 한글 폰트 렌더링
    void draw16English(uint8_t ch, uint16_t x, uint16_t y, Color fg, Color bg, uint8_t size, bool no_bg);
    void draw16Korean(uint16_t hangeul, uint16_t x, uint16_t y, Color fg, Color bg, uint8_t size, bool no_bg);
    
    // Task 래퍼
    static void uiTaskWrapper(void* parameter);
    
    // 로그 태그
    static constexpr const char* TAG = "LcdControl";
};

#endif // LCD_CONTROL_H
