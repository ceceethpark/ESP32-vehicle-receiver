# ESP32 리모컨 UI 다이어그램

## 📱 메인 화면 레이아웃

```mermaid
graph TB
    subgraph "240x320 LCD Display (세로형)"
        A[헤더<br/>리모컨<br/>0-42px]
        B[5버튼 영역<br/>50-150px]
        C[차량 상태<br/>180-220px]
        D[푸터<br/>ESP-NOW + CAN<br/>223-240px]
    end
    
    style A fill:#006666,stroke:#00ffff,color:#fff
    style B fill:#000066,stroke:#0000ff,color:#fff
    style C fill:#006600,stroke:#00ff00,color:#fff
    style D fill:#333,stroke:#888,color:#fff
```

## 🎮 5버튼 레이아웃

```mermaid
graph TD
    subgraph "Button Layout"
        UP["UP<br/>140,50<br/>40x30"]
        LEFT["LEFT<br/>90,85<br/>40x30"]
        SEL["SELECT<br/>140,85<br/>40x30"]
        RIGHT["RIGHT<br/>190,85<br/>40x30"]
        DOWN["DOWN<br/>140,120<br/>40x30"]
    end
    
    UP -.-> SEL
    LEFT -.-> SEL
    SEL -.-> RIGHT
    SEL -.-> DOWN
    
    style UP fill:#0000ff,stroke:#00ffff,color:#fff
    style LEFT fill:#0000ff,stroke:#00ffff,color:#fff
    style SEL fill:#0000ff,stroke:#00ffff,color:#fff
    style RIGHT fill:#0000ff,stroke:#00ffff,color:#fff
    style DOWN fill:#0000ff,stroke:#00ffff,color:#fff
```

## 🔄 상태 전환 다이어그램

```mermaid
stateDiagram-v2
    [*] --> 부팅
    부팅 --> 초기화
    초기화 --> 메인화면
    
    메인화면 --> 운행중 : 버튼입력
    운행중 --> 메인화면 : 정지
    
    메인화면 --> 설정모드 : SEL+LT+RT<br/>1초
    설정모드 --> 메인화면 : SEL 3초<br/>또는 타임아웃
    
    메인화면 --> 연결끊김 : ESP-NOW<br/>연결 실패
    연결끊김 --> 메인화면 : 재연결
```

## 🎨 색상 팔레트

```mermaid
graph LR
    subgraph "UI Colors"
        BLACK["BLACK<br/>0x0000<br/>배경"]
        CYAN["CYAN<br/>0x07FF<br/>제목/테두리"]
        BLUE["BLUE<br/>0x001F<br/>버튼 기본"]
        GREEN["GREEN<br/>0x07E0<br/>버튼 눌림/전진"]
        YELLOW["YELLOW<br/>0xFFE0<br/>정지/경고"]
        RED["RED<br/>0xF800<br/>후진/에러"]
        WHITE["WHITE<br/>0xFFFF<br/>텍스트"]
        GRAY["GRAY<br/>0x8410<br/>비활성"]
    end
    
    style BLACK fill:#000,stroke:#888,color:#fff
    style CYAN fill:#00ffff,stroke:#fff,color:#000
    style BLUE fill:#0000ff,stroke:#fff,color:#fff
    style GREEN fill:#00ff00,stroke:#fff,color:#000
    style YELLOW fill:#ffff00,stroke:#000,color:#000
    style RED fill:#ff0000,stroke:#fff,color:#fff
    style WHITE fill:#fff,stroke:#000,color:#000
    style GRAY fill:#888,stroke:#fff,color:#fff
```

## 📊 차량 정보 영역

```mermaid
graph TD
    subgraph "Vehicle Status Area 180-220px"
        A["속도: 25 km/h<br/>10,180"]
        B["배터리: ████▒▒ 85%<br/>170,180"]
        C["연결: 연결됨<br/>10,200"]
        D["신호: -45 dBm<br/>170,200"]
    end
    
    style A fill:#006600,stroke:#00ff00,color:#fff
    style B fill:#006600,stroke:#00ff00,color:#fff
    style C fill:#006600,stroke:#00ff00,color:#fff
    style D fill:#006600,stroke:#00ff00,color:#fff
```

## 🔀 데이터 흐름

```mermaid
flowchart LR
    A[5버튼 입력] --> B[RemoteButton]
    B --> C{버튼 종류}
    
    C -->|일반 버튼| D[ESP-NOW 전송]
    C -->|SEL+LT+RT| E[설정모드 진입]
    
    D --> F[YbCar]
    F --> G[차량 제어]
    G --> H[응답 데이터]
    H --> I[LCD 업데이트]
    
    E --> J[CAN 통신]
    J --> K[설정 읽기/쓰기]
    K --> L[0x5B0-0x5B8]
    L --> M[설정 UI]
    
    style A fill:#0000ff,color:#fff
    style D fill:#ff6600,color:#fff
    style E fill:#9900ff,color:#fff
    style I fill:#00ffff,color:#000
    style J fill:#ff0066,color:#fff
```

## 🖥️ 화면 구성 상세

```mermaid
graph TB
    subgraph "LCD 240x320 (세로형)"
        subgraph "Header 0-42"
            H1["리모컨<br/>32x32 한글<br/>CYAN"]
        end
        
        subgraph "Button Area 50-150"
            B1["UP 140,50"]
            B2["LEFT 90,85"]
            B3["SEL 140,85"]
            B4["RIGHT 190,85"]
            B5["DOWN 140,120"]
            B6["설정모드: SEL+LT+RT 1초<br/>155"]
        end
        
        subgraph "Status Area 180-220"
            S1["속도 + 방향"]
            S2["배터리 진행바"]
            S3["연결상태"]
            S4["RSSI 신호"]
        end
        
        subgraph "Footer 223-240"
            F1["ESP-NOW + CAN 500k"]
        end
    end
    
    H1 --> B1
    B6 --> S1
    S4 --> F1
    
    style H1 fill:#006666,stroke:#00ffff,color:#fff
    style B1 fill:#0000ff,color:#fff
    style B2 fill:#0000ff,color:#fff
    style B3 fill:#0000ff,color:#fff
    style B4 fill:#0000ff,color:#fff
    style B5 fill:#0000ff,color:#fff
    style B6 fill:#333,color:#fff
    style S1 fill:#006600,color:#fff
    style S2 fill:#006600,color:#fff
    style S3 fill:#006600,color:#fff
    style S4 fill:#006600,color:#fff
    style F1 fill:#333,color:#fff
```

## 🎯 설정모드 화면 구성

```mermaid
flowchart TD
    A[설정모드 진입] --> B[CAN 0x700<br/>GET_CONFIG]
    B --> C{응답 수신?}
    C -->|성공| D[0x5B0-0x5B7<br/>64바이트 수신]
    C -->|실패| E[타임아웃<br/>메인화면 복귀]
    
    D --> F[설정 화면 표시]
    F --> G{버튼 입력}
    
    G -->|UP/DN| H[항목 선택]
    G -->|LT/RT| I[값 변경]
    G -->|SEL 짧게| J[CAN 0x701<br/>PUT_CMD]
    G -->|SEL 3초| K[종료 확인]
    
    H --> F
    I --> F
    J --> L[CAN 0x708<br/>SAVE_CMD]
    L --> M[저장 완료<br/>메시지]
    M --> F
    
    K --> N[메인화면 복귀]
    
    style A fill:#9900ff,color:#fff
    style D fill:#ff6600,color:#fff
    style F fill:#0066ff,color:#fff
    style J fill:#ff0066,color:#fff
    style L fill:#ff0066,color:#fff
    style N fill:#006666,color:#fff
```

## 📡 통신 프로토콜

```mermaid
sequenceDiagram
    participant B as Button
    participant R as Remote
    participant E as ESP-NOW
    participant C as CAN
    participant V as Vehicle
    
    Note over R: 일반 운행 모드
    B->>R: 버튼 입력
    R->>E: 제어 명령
    E->>V: 차량 제어
    V->>E: 상태 데이터
    E->>R: 속도/배터리/방향
    R->>R: LCD 업데이트
    
    Note over R: 설정 모드 진입
    B->>R: SEL+LT+RT 1초
    R->>C: 0x700 GET_CONFIG
    C->>V: 설정 요청
    V->>C: 0x5B0-0x5B7 (64byte)
    C->>R: 설정 데이터
    R->>R: 설정 화면 표시
    
    B->>R: 값 변경
    R->>C: 0x701 PUT_CMD
    B->>R: SEL (저장)
    R->>C: 0x708 SAVE_CMD
    C->>V: 저장 실행
    V->>C: 완료 응답
    C->>R: 저장 완료
```

## 🔧 함수 호출 구조

```mermaid
graph TD
    A[setup] --> B[lcd.begin]
    A --> C[lcd.drawMainScreen]
    A --> D[button.begin]
    A --> E[can.begin]
    A --> F[espnow.begin]
    
    G[loop] --> H{버튼 체크}
    H -->|눌림| I[lcd.showButtonStatus]
    H -->|뗌| J[버튼 처리]
    
    J --> K{버튼 종류}
    K -->|일반| L[espnow.sendControl]
    K -->|설정 콤보| M[can.requestConfig]
    
    G --> N[espnow.receive]
    N --> O{데이터 수신}
    O -->|속도| P[lcd.showVehicleSpeed]
    O -->|배터리| Q[lcd.showBatteryLevel]
    O -->|방향| R[lcd.showVehicleDirection]
    O -->|RSSI| S[lcd.showRSSI]
    
    M --> T[설정 UI 표시]
    T --> U[설정값 변경]
    U --> V[can.saveConfig]
    
    style A fill:#ff6600,color:#fff
    style G fill:#ff6600,color:#fff
    style C fill:#0066ff,color:#fff
    style I fill:#0066ff,color:#fff
    style L fill:#9900ff,color:#fff
    style M fill:#ff0066,color:#fff
```

---

## 📷 다이어그램 렌더링 방법

### VS Code에서 보기
1. **Markdown Preview Mermaid Support** 확장 설치
2. `Ctrl+Shift+V`로 미리보기

### GitHub에서 보기
- GitHub는 Mermaid를 자동으로 렌더링합니다

### 온라인 에디터
- https://mermaid.live/ 에서 편집/내보내기 가능
- PNG, SVG, PDF로 변환 가능

### 이미지로 변환
```bash
# Mermaid CLI 설치
npm install -g @mermaid-js/mermaid-cli

# PNG로 변환
mmdc -i docs/ui-diagram.md -o docs/ui-diagram.png
```
