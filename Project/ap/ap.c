#include "ap.h"

extern struct netif gnetif;

void StartDefaultTask(void *argument){
    apInit();
    while(1){
        apMain();
    }
}

void apInit(){
    uartInit();
    uartPrintf(0, "System Booting... Smart Study Room Project\r\n");
}

void apMain(){
    uint8_t ip_assigned = 0;

    // 1. DHCP IP 할당 확인 루프
    uartPrintf(0, "Waiting for DHCP to assign IP address...\r\n");

    while (1) {
        // 현재 링크 상태 확인 (네트워크 선이 꽂혔는지 여부)
        if (netif_is_link_up(&gnetif)) {
            uartPrintf(0, "Physical Link: UP\r\n");
        } else {
            uartPrintf(0, "Physical Link: DOWN - Check Cable!\r\n");
        }

        if (gnetif.ip_addr.addr != 0) {
            uint8_t *ip = (uint8_t *)&gnetif.ip_addr.addr;
            
            uartPrintf(0, "\r\n====================================\r\n");
            uartPrintf(0, "  Ethernet Connected!\r\n");
            uartPrintf(0, "  IP Address : %d.%d.%d.%d\r\n", ip[0], ip[1], ip[2], ip[3]);
            uartPrintf(0, "====================================\r\n\r\n");
            
            ip_assigned = 1;
            break;
        }

        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
        osDelay(1000); // 1초 간격으로 체크
    }

    // 2. IP 할당 후 실제 서비스 로직
    while (1) {
        // 여기서 센서 읽기나 TCP 통신 처리를 시작합니다.
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7); // Blue LED (정상 작동 중)
        
        /* * 예: if (is_event_occurred) { 
         * send_data_to_server(); 
         * } 
         */
        
        osDelay(1000);
    }
}