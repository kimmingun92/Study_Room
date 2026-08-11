# STM32 기반 IoT 통합 독서실 관리 시스템

STM32 보드가 열화상·RFID·온습도·적외선 센서를 읽고 LED·서보·팬을 제어하며,
Qt 서버가 여러 좌석 클라이언트와 보드 사이를 TCP로 중계하는 IoT 관리 시스템.
좌석마다 입실/퇴실, LED 색상, 팬 세기, 문 개폐를 원격으로 제어할 수 있다.

`STM32F429` `FreeRTOS` `LwIP` `Qt6` `TCP/IP` `MLX90640` `RC522`

## 시스템 구성

```
[Client UI]  <--TCP-->  [Central Server]  <--TCP-->  [STM32 Nucleo]  <--I2C/SPI/GPIO/PWM-->  [Hardware Nodes]
Qt 클라이언트           Qt 서버 (좌석 관리,          LwIP/FreeRTOS               MLX90640·RC522·SSD1306
(좌석당 1개)            보드↔클라이언트 중계)         6-Task 아키텍처              Servo×4·DHT·RGB LED×3·
                        10.10.16.9:5000              10.10.16.200                 R300 Fan·IR 센서/리모컨
```

| 구성 요소 | 역할 | 위치 |
|---|---|---|
| STM32 보드 | 센서 읽기, 액추에이터 제어, TCP 클라이언트 | [`firmware/`](firmware/README.md) |
| Qt 서버 | 좌석 입/퇴실 관리, 보드↔클라이언트 메시지 중계 | [`qt_app/StudyRoom_server`](qt_app/README.md) |
| Qt 클라이언트 | 좌석 선택, LED/팬/문 제어 UI | [`qt_app/StudyRoom_client`](qt_app/README.md) |

각 폴더 안의 파일 역할, 빌드 방법은 폴더별 README에 정리했다.

- [`firmware/README.md`](firmware/README.md)
- [`qt_app/README.md`](qt_app/README.md)

## 핵심 설계 — FreeRTOS 6-Task 아키텍처

열화상(I2C1+OLED), RFID(SPI), 환경센서(GPIO: DHT/IR), 통신(Ethernet/LwIP), CLI(UART)를
각각 독립 태스크로 분리해 서로 블로킹 없이 병렬로 동작하도록 구성했다.

| 태스크 | 함수 | 역할 | 사용 버스 |
|---|---|---|---|
| `defaultTask` | `StartDefaultTask` | LwIP 초기화, 하드웨어 초기화, CLI 메인 루프 | UART |
| `dhtTask` | `dhtSystemTask` | DHT11 온습도 측정 + 팬 자동 제어 | GPIO (1-Wire 방식) |
| `tcpTask` | `tcpClientSystemTask` | 서버 TCP 접속, 명령 수신·파싱 | Ethernet (LwIP netconn) |
| `irSensorTask` | `irSensorSystemTask` | IR 근접 센서, 자동 문 열림 | GPIO |
| `rfidTask` | `rfidSystemTask` | RC522 카드 스캔, 도어 UID 매칭 | SPI |
| `thermalTask` | `thermalSystemTask` | MLX90640 프레임 읽기 + OLED 시각화 | I2C1 (OLED와 공유) |

## 통신 프로토콜 — `@ID:TYPE:VALUE`

보드 ↔ 서버는 커스텀 텍스트 프로토콜을, 서버 ↔ 클라이언트는 좌석 관리용 텍스트
프로토콜을 각각 사용한다.

### 보드 ↔ 서버 (`@ID:TYPE:VALUE\n`)

| 구간 | ID 예시 | 방향 | 내용 |
|---|---|---|---|
| 0~3 | `ID_SYS_HEARTBEAT`, `ID_NET_CONN_STATE` | 보드→서버 | 시스템/네트워크 상태 |
| 10~12 | `ID_ENV_TEMP`, `ID_ENV_HUMI`, `ID_ENV_IR_SENSOR` | 보드→서버 | 환경 센서 값 |
| 50~57 | `ID_OUT_LED1_COLOR` ~ `ID_OUT_DOOR3_STATE` | 서버→보드 | LED/팬/문 제어 |
| 100~102 | `ID_ALARM_DHT_ERROR`, `ID_ALARM_FIRE` | 보드→서버 | 알람 |

`TYPE`은 `UINT8`(0) / `INT32`(1) / `FLOAT`(2) / `BOOL`(3).

### 서버 ↔ 클라이언트

| 방향 | 메시지 | 설명 |
|---|---|---|
| 클라이언트→서버 | `ENTER:1`, `EXIT:1` | 입실/퇴실 |
| 클라이언트→서버 | `@50:0:1` | 좌석 제어 명령 (그대로 보드로 포워딩) |
| 서버→클라이언트 | `USED_SEATS:1,2` | 사용 중 좌석 목록 (입실/퇴실 시 전체 브로드캐스트) |
| 서버→클라이언트 | `ENTRY_SUCCESS` / `ENTRY_FAIL` | 입실 결과 |

서버는 접속한 소켓의 IP가 보드 고정 IP(`10.10.16.200`)인지로 보드와 클라이언트를
구분하고, 보드가 보낸 센서 데이터는 **입실 중인 좌석의 클라이언트에게만** 포워딩한다.

## 트러블슈팅

실제로 막혔던 지점 중 설계에 영향을 준 것들만 추렸다.

### 1. I2C 버스 충돌 (OLED ↔ MLX90640)

두 센서가 같은 `hi2c1` 버스를 쓰는데, MLX90640 프레임 수신(32Hz)과 OLED 갱신이 서로
다른 태스크에서 동시에 버스에 접근하면서 `HAL_BUSY` 오류가 발생했다. 정상 부팅
이후 OLED 화면이 안 뜨거나 온도 값이 간헐적으로만 갱신되는 증상으로 나타났다.

**해결 방향**: FreeRTOS 뮤텍스(`i2cMutexTake`/`Give`)로 두 태스크의 I2C 접근을
직렬화하는 방식으로 설계

### 2. FreeRTOS HardFault

태스크(RFID/DHT/OLED/Thermal/LED/Servo)를 늘릴 때마다 부팅 직후 또는 일정 시간 뒤
HardFault로 시스템이 멈추는 문제가 있었다. 원인은 두 가지였다.

- **힙 오버플로우**: 태스크 수가 늘면서 `configTOTAL_HEAP_SIZE` 초과 → `pvPortMalloc()`
  실패 → HardFault. 전체 힙 크기를 늘리는 대신, 비중이 적은 태스크의 스택 크기를
  줄여서 해결했다.
- **DHT 태스크의 CPU 독점**: DHT 타이밍 측정 중 다른 태스크(OLED/Thermal)가 CPU를
  못 받아 2초 샘플링을 못 채우는 문제. `apMain()`에 `volatile bool dht_read` 플래그를
  둬서, DHT 값을 받아오는 동안 OLED/Thermal 태스크에 `osDelay`로 양보하도록 했다.

### 3. Timer 기반 Timebase 사용 시 HardFault

STM32CubeMX 기본 설정대로 `TIM1`을 HAL 타임베이스로 쓰자 `USART3` 인터럽트가 자주
발생할 때 HardFault가 났다. `TIM1_UP`과 `TIM10` 인터럽트가 같은 벡터
(`TIM1_UP_TIM10_IRQHandler`)를 공유하는 데다, `TIM1`은 Break/Trigger/COM/Capture/Update
등 인터럽트 소스가 많은 Advanced Timer라 우선순위가 높아 USART3와 경쟁 상태(race
condition)가 발생했다. **타임베이스를 `TIM1`에서 범용 타이머 `TIM11`로 바꿔서
해결**했다 (현재 `Core/Src/stm32f4xx_hal_timebase_tim.c`에 반영되어 있다).

### 4. TCP 연결 안정성 설계 (서버 재접속)

고정 5초 간격으로 무조건 재접속을 시도하는 기존 방식은, 서버가 잠깐 죽었다가
살아나면 여러 보드가 동시에 재접속을 시도해 서버에 순간적인 부하가 몰리는 문제와,
서버가 응답 없이 소켓만 물고 있는 half-open 상태를 감지하지 못하는 문제가 있었다.

**개선 설계**: IDLE→CONNECTING→CONNECTED→ERROR→WAITING 5단계 상태 머신 + 지수
백오프(재시도마다 대기시간 2배, 최대 30초) + 랜덤 지터(0~500ms, 여러 보드의 재접속
타이밍 분산) + 주기적 Heartbeat(응답 3회 연속 실패 시 강제 재연결)로 설계

## 실행 방법 (요약)

1. **STM32 보드**: `firmware/`를 STM32CubeIDE 또는 VSCode+CMake로 빌드해 보드에
   플래시. 고정 IP `10.10.16.200`, 서버 접속 주소 `10.10.16.9:5000`으로 설정돼 있다.
2. **Qt 서버**: `qt_app/StudyRoom_server`를 Qt6로 빌드해 먼저 실행 (포트 5000 대기).
3. **Qt 클라이언트**: `qt_app/StudyRoom_client`를 좌석 수만큼 실행, 서버 접속 후
   좌석 선택 → 입실.

자세한 빌드 명령은 [`firmware/README.md`](firmware/README.md),
[`qt_app/README.md`](qt_app/README.md) 참고.

## 개선 가능 사항

- DMA 기반 USART 통신 (인터럽트 방식 대체)
- FreeRTOS Event Group 도입 (센서 데이터 준비 시점에만 태스크가 깨어나는 구조)
- Watchdog Timer 적용 (HardFault 시 자동 리셋)
- RFID UID 해시(SHA-256) 인증 (현재는 평문 UID 비교라 카드 복제에 취약)
- 네트워크 TLS 적용 (현재 TCP 평문 통신)
- I2C 뮤텍스 실제 연결 (위 트러블슈팅 1번)
- TCP 재연결 상태 머신 실제 적용 (위 트러블슈팅 4번)
