#include "ap.h"

extern struct netif gnetif;




void StartDefaultTask(void *argument)
{
    MX_LWIP_Init();
    apInit();
    cliPrintf("CLI> ");
    for (;;) {
        apMain();
    }
}

void dhtSystemTask(void *argument)
{
    for (;;) {
        dhtMain();
    }
}





void tcpClientSystemTask(void *argument)
{
    struct netconn *conn;
    err_t err;
    ip_addr_t server_addr;

    // 1. 서버 IP 주소 설정 (PC의 IP 주소를 적으세요)
    IP_ADDR4(&server_addr, 10, 10, 16, 9);

    while (1) {
        // 2. 새로운 TCP 연결 생성
        conn = netconn_new(NETCONN_TCP);

        if (conn != NULL) {
            // 3. 서버에 접속 시도 (포트 5000)
            err = netconn_connect(conn, &server_addr, 5000);

            if (err == ERR_OK) {
                printf("서버 접속 성공!\n");

                // 4. 접속 성공 메시지 전송
                char *msg = "$DATA,BOARD,READY#";
                netconn_write(conn, msg, strlen(msg), NETCONN_COPY);

                // 5. 서버로부터 데이터 수신 대기 루프
                struct netbuf *buf;
                void *data;
                uint16_t len;
                while (netconn_recv(conn, &buf) == ERR_OK) {
                    netbuf_data(buf, &data, &len);
                    cliPrintf("Server Message: %.*s\r\n", len, (char*)data);
                    netbuf_delete(buf);
                }
            }

            // 연결이 끊기면 정리하고 재접속 시도
            netconn_close(conn);
            netconn_delete(conn);
        }

        osDelay(5000); // 접속 실패 시 5초 후 재시도
    }
}




void apInit()
{
    hwInit();
}

void apMain()
{
    cliMain();
    osDelay(10);
}
