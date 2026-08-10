# qt_app/

Qt6 기반 서버·클라이언트. 서버가 좌석 입/퇴실을 관리하며 보드와 클라이언트 사이의
메시지를 중계하고, 클라이언트는 좌석을 선택해 입실한 뒤 LED·팬·문을 제어하는 UI를
제공한다.

이 폴더는 원래 별도 저장소(`github.com/Pjumo/StudyRoom_QT`)였던 걸 커밋 히스토리
그대로 가져온 것이다.

## 폴더 구조

```
qt_app/
├── StudyRoom_server/
│   ├── main.cpp            진입점
│   ├── mainserver.h/.cpp    서버 로직 (좌석 관리, 메시지 중계)
│   ├── mainserver.ui        로그 뷰 UI
│   └── CMakeLists.txt
└── StudyRoom_client/
    ├── main.cpp            진입점
    ├── mainclient.h/.cpp    클라이언트 로직 (좌석 선택, 제어 UI)
    ├── mainclient.ui        좌석 선택/제어 화면 UI
    └── CMakeLists.txt
```

## StudyRoom_server — 함수 역할

| 함수 | 역할 |
|---|---|
| `onNewConnection()` | 접속 IP가 보드 고정 IP(`10.10.16.200`)인지로 보드/클라이언트 분기 |
| `onReadyRead()` | 보드는 즉시 처리, 클라이언트는 `\n` 기준 버퍼링 후 파싱 |
| `processClientMessage()` | `ENTER:n` / `EXIT:n` / `@ID:TYPE:VALUE` 처리 |
| `processBoardMessage()` | 보드 센서 데이터를 **입실 중인 좌석의 클라이언트에게만** 포워딩 |
| `sendUsedSeatsList()` | 특정 소켓에 현재 사용 중 좌석 목록 전송 |
| `broadcastUsedSeats()` | 전체 클라이언트에 좌석 목록 브로드캐스트 |
| `onDisconnected()` | 보드/클라이언트 정리, 비정상 종료 시 자동 퇴실 처리 |

좌석 중복 입실 방지(`m_seatMap`), 좌석 번호 유효성 검사(1~3), 보드 미연결 시 제어
명령 무시 등의 방어 로직이 `processClientMessage()`에 들어있다.

## StudyRoom_client — 주요 동작

- `QStackedWidget`으로 로그인 페이지(좌석 선택)와 메인 페이지(제어) 전환
- 좌석 3개 버튼 → 선택/사용중 스타일 실시간 반영 (`USED_SEATS:` 메시지 수신 시 갱신)
- LED 4색(off/yellow/white/warm), 팬 세기 슬라이더, 문 열기/닫기 → `sendCommand()`로
  `@ID:TYPE:VALUE` 형식 전송
- 입실 경과 시간 1초마다 갱신 표시

## 빌드 방법

Qt6 (Core, Widgets, Network) + CMake 필요.

```bash
# 서버
cd qt_app/StudyRoom_server
cmake -B build && cmake --build build

# 클라이언트
cd qt_app/StudyRoom_client
cmake -B build && cmake --build build
```

또는 Qt Creator에서 각 폴더의 `CMakeLists.txt`를 프로젝트로 열어서 빌드해도 된다.

## 실행 순서

서버를 먼저 실행해야 한다 (포트 5000 리스닝 시작).

```bash
./StudyRoom_server        # 포트 5000 대기
./StudyRoom_client         # 좌석 수만큼 여러 개 실행 가능
```

서버 IP/포트는 `mainserver.cpp`에서 `listen(QHostAddress::Any, 5000)`으로 고정,
클라이언트 접속 주소는 `mainclient.cpp`에서 `connectToHost("10.10.16.9", 5000)`으로
하드코딩돼 있다 — 다른 네트워크에서 쓰려면 이 부분을 수정해야 한다.
