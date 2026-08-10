#include "tcp.h"

extern bool isAutoMotor;

static void parseProtocol(void *data, uint16_t len)
{
    // 안전한 문자열 처리를 위해 복사본 생성 및 널 종료 문자 처리
    char msg[64];
    if (len >= sizeof(msg))
        len = sizeof(msg) - 1;
    memcpy(msg, data, len);
    msg[len] = '\0';

    // 시작 문(@) 확인
    if (msg[0] != '@')
        return;

    // 데이터 분리 (@ID:TYPE:VALUE)
    // strtok는 원본 문자열을 수정하므로 복사본(msg) 사용
    char *ptr = msg + 1; // '@' 건너뛰기
    char *str_id = strtok(ptr, ":");
    char *str_type = strtok(NULL, ":");
    char *str_val = strtok(NULL, "\n");

    if (!str_id || !str_type || !str_val)
        return;

    // 숫자 데이터 변환
    int id = atoi(str_id);
    int type = atoi(str_type);

    // ID에 따른 분기 처리
    switch ((SensorID)id) {
    case ID_OUT_LED1_COLOR:
        if (type == TYPE_UINT8) {
            uint8_t color = (uint8_t)atoi(str_val);
            setLedColor(0, color);
        }
        break;

    case ID_OUT_LED2_COLOR:
        if (type == TYPE_UINT8) {
            uint8_t color = (uint8_t)atoi(str_val);
            setLedColor(1, color);
        }
        break;

    case ID_OUT_LED3_COLOR:
        if (type == TYPE_UINT8) {
            uint8_t color = (uint8_t)atoi(str_val);
            setLedColor(2, color);
        }
        break;

    case ID_OUT_FAN_SPEED:
        if (type == TYPE_UINT8) {
            uint8_t speed = (uint8_t)atoi(str_val);
            isAutoMotor = false;
            switch(speed){
            case 1:
                motorR300SetPulse(0, 3500);
                break;
            case 2:
                motorR300SetPulse(0, 5000);
                break;
            case 3:
                motorR300SetPulse(0, 7500);
                break;
            case 4:
                motorR300SetPulse(0, 9999);
                break;
            }
            motorR300Set(true);
        }
        break;

    case ID_OUT_DOOR0_STATE:
    case ID_OUT_DOOR1_STATE:
    case ID_OUT_DOOR2_STATE:
    case ID_OUT_DOOR3_STATE:
        if (type == TYPE_BOOL) {
            bool open = (atoi(str_val) != 0);
            int doorIdx = id - ID_OUT_DOOR0_STATE; // ID 값을 이용해 인덱스 계산
            changeDoorState(doorIdx, open);
        }
        break;

    case ID_SYS_HEARTBEAT:
    case ID_SYS_UPTIME:
    case ID_NET_IP_ADDR:
    case ID_NET_CONN_STATE:
        break;

    default:
        LOG_WRN("[TCP] 알 수 없는 ID 수신: % d", id);
        break;
    }
}

void tcpMain()
{
    struct netconn *conn = NULL;
    err_t err;
    ip_addr_t server_addr;

    IP_ADDR4(&server_addr, 10, 10, 16, 9);

    for (;;) {
        LOG_INF("[TCP] 서버 접속 시도 중... (10.10.16.9:5000)");

        conn = netconn_new(NETCONN_TCP);
        if (conn != NULL) {
            // 수신 타임아웃 설정 (예: 2초 동안 데이터 없으면 타임아웃 발생)
            netconn_set_recvtimeout(conn, 2000);

            err = netconn_connect(conn, &server_addr, 5000);

            if (err == ERR_OK) {
                LOG_INF("[TCP] 서버 접속 성공!");

                char *msg = "@BOARD:READY\n";
                netconn_write(conn, msg, strlen(msg), NETCONN_COPY);

                struct netbuf *buf;
                err_t recv_err;

                // 무한 루프로 변경하여 연결을 계속 유지
                while (1) {
                    recv_err = netconn_recv(conn, &buf);

                    if (recv_err == ERR_OK) {
                        // 데이터를 정상적으로 받았을 때
                        void *data;
                        uint16_t len;
                        netbuf_data(buf, &data, &len);
                        cliPrintf("Server Message: %.*s\r\n", len, (char *)data);
                        parseProtocol(data, len);
                        netbuf_delete(buf);
                    } else if (recv_err == ERR_TIMEOUT) {
                        // 연결은 살아있는 상태이므로 아무것도 하지 않고 다시 대기
                        continue;
                    } else {
                        // ERR_CLSD (연결 종료) 등 진짜 에러가 발생하면 루프를 나감
                        LOG_ERR("[TCP] 연결 끊김 또는 수신 에러 발생 (Code: %d)", (int)recv_err);
                        break;
                    }
                    osDelay(10);
                }
            } else {
                LOG_ERR("[TCP] 접속 실패! (에러 코드: %d)", (int)err);
            }

            // 루프를 빠져나오면 자원을 정리
            netconn_close(conn);
            netconn_delete(conn);
            conn = NULL;
        }

        LOG_INF("[TCP] 5초 후 재시도합니다...");
        osDelay(5000);
    }
}