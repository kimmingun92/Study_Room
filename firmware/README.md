# firmware/

STM32F429ZIT6 보드 펌웨어. STM32CubeMX로 생성한 뼈대(`Core/`, `Drivers/`,
`Middlewares/`, `LWIP/`) 위에 실제 애플리케이션 코드(`Project/`)를 얹은 구조다.
FreeRTOS 6개 태스크로 센서 읽기·액추에이터 제어·TCP 통신·CLI를 병렬 처리한다.


## 폴더 구조

```
firmware/
├── Core/               STM32CubeMX 자동 생성 초기화 코드
│   ├── Src/main.c, freertos.c, gpio.c, i2c.c, spi.c, tim.c, usart.c
│   └── Inc/ (대응 헤더)
├── Project/            직접 작성한 애플리케이션 코드 
│   ├── ap/             태스크 진입점 (ap.c)
│   ├── bsp/             delay/millis 등 보드 지원 함수
│   ├── common/           공통 정의, 로그 매크로
│   ├── network/           TCP 클라이언트, 프로토콜 ID 정의
│   └── hw/driver/         센서·액추에이터 드라이버 (아래 표)
├── Drivers/             ST HAL, CMSIS (서드파티, 자동 생성)
├── Middlewares/          FreeRTOS 커널, LwIP 스택 (서드파티)
├── LWIP/                LwIP 애플리케이션 설정 (고정 IP 등)
├── cmake/                STM32CubeMX가 생성한 CMake 빌드 파일
├── CMakeLists.txt        VSCode+CMake로 빌드할 때 사용 (Project/*.c 포함)
├── *.ioc                 STM32CubeMX 설정 파일 (페리페럴/클럭/FreeRTOS 설정)
└── *.ld, startup_*.s     링커 스크립트, 시작 어셈블리
```

## Project/hw/driver — 센서·액추에이터 드라이버

| 파일 | 대상 | 비고 |
|---|---|---|
| `led.c/h` | RGB LED × 3 | 소프트웨어 PWM (9채널), yellow/white/warm/off |
| `servo.c/h` | 서보모터 × 4 | 좌석 3개 문 + 전체 문, 600~1500us 펄스 |
| `dht.c/h` | DHT11 온습도 | 인터럽트 기반 엣지 타이밍으로 직접 디코딩 |
| `motor_r300.c/h` | R300 팬 모터 | 온습도 구간별 자동 풍량 제어 (4단계) |
| `my_rfid.c/h` + `my_spi.c/h` | RC522 RFID | 레지스터 직접 제어 (라이브러리 없이), UID→방 매칭 |
| `thermal.c/h` | MLX90640 열화상 (32×24) | I2C, EEPROM 보정계수 기반 온도 계산 |
| `oled.c/h` | SSD1306 OLED (128×64) | 5×7 비트맵 폰트 직접 구현, 열화상 시각화 |
| `ir_receiver.c/h` | IR 리모컨 수신 | NEC 프로토콜, 타이머+인터럽트로 비트 디코딩 |
| `ir_sensor.c/h` | IR 근접 센서 | 감지 시 문 자동 개방 (5초 뒤 자동 닫힘) |
| `i2c_mutex.c/h` | I2C1 버스 보호 | OLED/Thermal 동시 접근 방지용 뮤텍스 (정의만 있고 호출부 미연결 — 루트 README 트러블슈팅 참고) |
| `uart.c/h` | UART3 | 메시지 큐 기반 non-blocking 수신 |
| `cli.c/h` | 시리얼 CLI | 명령 히스토리, 방향키 지원 (`door`, `led`, `dht`, `rfid`, `motor`, `th` 등) |
| `log.c/h` + `log_def.h` | 로그 매크로 | 레벨별(FATAL~VERBOSE) ANSI 컬러 출력 |
| `pwm.c/h` | 소프트웨어 PWM | GPIO 핀 다중화용 공용 PWM 태스크 |

## 통신 설정

- 보드 고정 IP: `10.10.16.200` (`LWIP/App/lwip.c`)
- 서버 접속 주소: `10.10.16.9:5000` (`Project/network/tcp.c`)
- 프로토콜: 서버에서 받은 `@ID:TYPE:VALUE` 메시지를 `tcp.c`의 `parseProtocol()`이
  파싱해서 LED/팬/문 제어 함수로 분기 (ID 정의는 `network/protocol_id.h`)

## 빌드 방법

두 가지 빌드 체계가 같이 들어있다. 둘 중 편한 쪽으로 빌드하면 된다.

### STM32CubeIDE

`firmware/` 폴더를 STM32CubeIDE에서 **Import > Existing Projects into Workspace**로
열면 `.project`/`.cproject`를 그대로 인식한다.

### VSCode + CMake

```bash
cd firmware
cmake --preset Debug   # CMakePresets.json 참고
cmake --build --preset Debug
```

`CMakeLists.txt`가 `Project/*.c`를 glob으로 포함하므로, `Project/` 폴더에 파일을
추가해도 CMakeLists.txt를 따로 수정할 필요는 없다.

## FreeRTOS 태스크 우선순위

| 태스크 | 우선순위 | 이유 |
|---|---|---|
| DHT | High | 타이밍에 민감한 1-Wire 디코딩, CPU 선점 필요 |
| RFID | High | SPI 스캔 지연 시 카드 인식 놓침 |
| OLED | Mid | |
| Thermal | Mid | |
| LED / Servo | Low | 지연에 상대적으로 둔감 |

메모리 부족을 막기 위해 FreeRTOS 힙 크기를 늘리는 대신, 비중이 적은 태스크의
스택 크기를 줄이는 방향으로 조정했다 (자세한 배경은 루트 README 트러블슈팅 참고).
