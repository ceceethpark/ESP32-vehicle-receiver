/*
 * ESP32 WROOM 수신기 예제 (ESP-NOW)
 * 
 * 이 코드는 리모컨에서 전송된 신호를 수신하는 예제입니다.
 * 실제 프로젝트에 맞게 수정하여 사용하세요.
 * 
 * 사용법:
 * 1. 이 코드를 별도의 ESP32에 업로드
 * 2. 시리얼 모니터에서 MAC 주소 확인
 * 3. 리모컨 코드에 이 MAC 주소 입력
 */

#include <esp_now.h>
#include <WiFi.h>

// 수신할 데이터 구조체 (송신기와 동일해야 함)
typedef struct struct_message {
  uint8_t buttonId;
  uint8_t buttonState;
  uint32_t timestamp;
} struct_message;

struct_message incomingData;

// LED 제어 핀들 (예제)
#define LED_1 12
#define LED_2 13
#define LED_3 14
#define LED_4 27

// 데이터 수신 콜백 함수
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingData, incomingData, sizeof(incomingData));
  
  Serial.print("수신 데이터: ");
  Serial.print("버튼 ID: ");
  Serial.print(((struct_message*)incomingData)->buttonId);
  Serial.print(" | 상태: ");
  Serial.print(((struct_message*)incomingData)->buttonState);
  Serial.print(" | 시간: ");
  Serial.println(((struct_message*)incomingData)->timestamp);
  
  // 버튼에 따른 동작 수행
  handleButtonPress(((struct_message*)incomingData)->buttonId);
}

void setup() {
  Serial.begin(115200);
  
  // LED 핀 설정
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);
  
  // WiFi를 Station 모드로 설정
  WiFi.mode(WIFI_STA);
  
  // MAC 주소 출력 (리모컨 설정에 필요)
  Serial.print("수신기 MAC 주소: ");
  Serial.println(WiFi.macAddress());
  Serial.println("이 주소를 리모컨 코드에 입력하세요!");
  
  // ESP-NOW 초기화
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW 초기화 실패");
    return;
  }
  
  Serial.println("ESP-NOW 초기화 성공");
  
  // 수신 콜백 등록
  esp_now_register_recv_cb(OnDataRecv);
  
  Serial.println("수신기 준비 완료");
}

void loop() {
  // ESP-NOW는 인터럽트 기반으로 동작하므로
  // loop에서는 다른 작업 수행 가능
  delay(10);
}

// 버튼 눌림에 따른 동작 처리
void handleButtonPress(uint8_t buttonId) {
  switch(buttonId) {
    case 1:
      digitalWrite(LED_1, !digitalRead(LED_1)); // LED 토글
      Serial.println("LED 1 토글");
      break;
      
    case 2:
      digitalWrite(LED_2, !digitalRead(LED_2));
      Serial.println("LED 2 토글");
      break;
      
    case 3:
      digitalWrite(LED_3, !digitalRead(LED_3));
      Serial.println("LED 3 토글");
      break;
      
    case 4:
      digitalWrite(LED_4, !digitalRead(LED_4));
      Serial.println("LED 4 토글");
      break;
      
    default:
      Serial.println("알 수 없는 버튼");
      break;
  }
}
