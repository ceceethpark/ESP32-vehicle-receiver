#include <Arduino.h>
#include "class/led/LocalLED.h"
#include "class/button/LocalButton.h"
#include "class/ybcar/YbCar.h"
#include "class/espnow/RecvESPNow.h"
#include "class/cancom/CanComm.h"
#include "class/lcd/LocalLCD.h"
#include "class/ybcarDoctor/YbCarDoctor.h"

// Global objects
LocalLED led;
LocalButton localButton;
YbCar car;
RecvESPNow espNow;
CanComm canComm;
LocalLCD lcd;
YbCarDoctor carDoctor;

// Control mode
enum ControlMode {
    MODE_REMOTE = 0,
    MODE_LOCAL = 1
};

ControlMode currentMode = MODE_REMOTE;
unsigned long lastRemoteTime = 0;
const unsigned long REMOTE_TIMEOUT = 3000; // 3 seconds

// Remote transmitter MAC address (will be set when first message received)
uint8_t remoteMac[6] = {0};
bool remoteMacSet = false;

// Function declarations
void onDataReceived(const uint8_t* mac, const uint8_t* data, int len);
void handleButtonCommand(uint8_t buttonId, uint8_t state);
void sendVehicleStatus();
void updateDisplay();
void processLocalControl();
void checkRemoteTimeout();

void setup() {
    Serial.begin(115200);
    printf("\r\n\r\n=== Vehicle Receiver Starting ===\r\n");
    
    // Initialize LED
    printf("Initializing LED...\r\n");
    led.begin();
    led.showInfo();
    
    // Initialize local button
    printf("Initializing local buttons...\r\n");
    localButton.begin(&car);
    
    // Initialize ESP-NOW
    printf("Initializing ESP-NOW...\r\n");
    if (!espNow.begin()) {
        printf("ESP-NOW init failed!\r\n");
        led.showError();
        while(1) delay(1000);
    }
    espNow.setReceiveCallback(onDataReceived);
    
    // Initialize CAN communication
    printf("Initializing CAN...\r\n");
    if (!canComm.begin()) {
        printf("CAN init failed!\r\n");
        led.showError();
        while(1) delay(1000);
    }
    
    // Initialize LCD
    printf("Initializing LCD...\r\n");
    if (!lcd.begin()) {
        printf("LCD init failed!\r\n");
        led.showWarning();
    } else {
        lcd.drawMainScreen();
        lcd.printText("Ready", 10, 50, LocalLCD::GREEN);
    }
    
    // Initialize car
    printf("Initializing car...\r\n");
    car.emergencyStop();
    
    printf("=== System Ready ===\r\n");
    led.showSuccess();
}

void loop() {
    // Check remote timeout
    checkRemoteTimeout();
    
    // Process local control if in local mode
    if (currentMode == MODE_LOCAL) {
        processLocalControl();
    }
    
    // Update display every 500ms
    static unsigned long lastDisplayUpdate = 0;
    if (millis() - lastDisplayUpdate > 500) {
        updateDisplay();
        lastDisplayUpdate = millis();
    }
    
    // Send status to remote every 100ms
    static unsigned long lastStatusSend = 0;
    if (millis() - lastStatusSend > 100) {
        sendVehicleStatus();
        lastStatusSend = millis();
    }
    
    delay(10);
}

void onDataReceived(const uint8_t* mac, const uint8_t* data, int len) {
    if (len != sizeof(struct_message)) {
        return;
    }
    
    // Store remote MAC address on first message
    if (!remoteMacSet) {
        memcpy(remoteMac, mac, 6);
        remoteMacSet = true;
        printf("Remote MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n", 
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    
    // Update remote time and switch to remote mode
    lastRemoteTime = millis();
    if (currentMode != MODE_REMOTE) {
        currentMode = MODE_REMOTE;
        printf("Switched to REMOTE mode\r\n");
        led.showInfo();
    }
    
    // Process button command
    struct_message* msg = (struct_message*)data;
    handleButtonCommand(msg->buttonId, msg->buttonState);
}

void handleButtonCommand(uint8_t buttonId, uint8_t state) {
    printf("Button: %d, State: %d\r\n", buttonId, state);
    
    // Emergency stop has priority
    if (buttonId == 7 && state == 1) { // Emergency button pressed
        car.emergencyStop();
        canComm.sendMotorCommand(0, 0);
        led.showError();
        return;
    }
    
    // Other buttons
    switch(buttonId) {
        case 1: // Forward
            if (state == 1) {
                car.moveForward(50);
                canComm.sendMotorCommand(50, 1);
            } else {
                car.stop();
                canComm.sendMotorCommand(0, 0);
            }
            break;
            
        case 2: // Backward
            if (state == 1) {
                car.moveBackward(50);
                canComm.sendMotorCommand(50, 2);
            } else {
                car.stop();
                canComm.sendMotorCommand(0, 0);
            }
            break;
            
        case 3: // Left
            if (state == 1) {
                car.turnLeft(50);
                canComm.sendMotorCommand(50, 3);
            } else {
                car.stop();
                canComm.sendMotorCommand(0, 0);
            }
            break;
            
        case 4: // Right
            if (state == 1) {
                car.turnRight(50);
                canComm.sendMotorCommand(50, 4);
            } else {
                car.stop();
                canComm.sendMotorCommand(0, 0);
            }
            break;
            
        case 5: // Lift up
            car.liftUp();
            canComm.sendLiftCommand(1);
            break;
            
        case 6: // Lift down
            car.liftDown();
            canComm.sendLiftCommand(2);
            break;
            
        case 8: // Cast up
            car.castUp();
            canComm.sendCastCommand(1);
            break;
            
        case 9: // Cast down
            car.castDown();
            canComm.sendCastCommand(2);
            break;
    }
}

void sendVehicleStatus() {
    if (!remoteMacSet) return;
    
    struct_vehicle_status status;
    status.speed = car.getSpeed();
    status.direction = car.getDirection();
    status.batteryLevel = car.getBatteryLevel();
    status.motorTemp = car.getMotorTemp();
    status.motorCurrent = car.getMotorCurrent();
    status.fetTemp = car.getFetTemp();
    
    espNow.send(remoteMac, (uint8_t*)&status, sizeof(status));
}

void updateDisplay() {
    if (!lcd.isInitialized()) return;
    
    lcd.showVehicleSpeed(car.getSpeed());
    lcd.showVehicleBattery(car.getBatteryLevel());
    lcd.showMotorTemp(car.getMotorTemp());
    lcd.showMotorCurrent(car.getMotorCurrent());
    lcd.showFetTemp(car.getFetTemp());
    
    // Show control mode
    const char* modeText = (currentMode == MODE_REMOTE) ? "REMOTE" : "LOCAL";
    lcd.printText(modeText, 10, 220, LocalLCD::YELLOW);
}

void processLocalControl() {
    localButton.update();
}

void checkRemoteTimeout() {
    if (currentMode == MODE_REMOTE) {
        if (millis() - lastRemoteTime > REMOTE_TIMEOUT) {
            currentMode = MODE_LOCAL;
            printf("Remote timeout! Switched to LOCAL mode\r\n");
            led.showWarning();
            car.stop();
            canComm.sendMotorCommand(0, 0);
        }
    }
}
