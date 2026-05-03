#ifndef __NETWORK_PROTOCOL_ID_H_
#define __NETWORK_PROTOCOL_ID_H_

typedef enum {
    /* 0~9: 시스템 및 네트워크 상태 (디버깅용) */
    ID_SYS_HEARTBEAT    = 0,    // 보드 동작 상태 (1: Alive)
    ID_SYS_UPTIME       = 1,    // 부팅 후 경과 시간 (초 단위)
    ID_NET_IP_ADDR      = 2,    // 현재 보드 IP (10.10.16.200 확인용)
    ID_NET_CONN_STATE   = 3,    // 서버 연결 상태 (0: Disconnected, 1: Connected)

    /* 10~49: 환경 센서 데이터 (보드 -> 서버) */
    ID_ENV_TEMP         = 10,
    ID_ENV_HUMI         = 11,
    ID_ENV_IR_SENSOR    = 12,

    /* 50~69: 액추에이터 제어 및 피드백 (서버 -> 보드) */
    ID_OUT_LED_COLOR    = 50,
    ID_OUT_FAN_SPEED    = 51,
    ID_OUT_DOOR0_STATE  = 52,
    ID_OUT_DOOR1_STATE  = 53,
    ID_OUT_DOOR2_STATE  = 54,
    ID_OUT_DOOR3_STATE  = 55,

    /* 100+: 시스템 알람 및 에러 (LogView에 강조할 내용) */
    ID_ALARM_DHT_ERROR  = 100,  // DHT 센서 읽기 실패
    ID_ALARM_NET_LOST   = 101,  // 네트워크 끊김 감지
    ID_ALARM_FIRE       = 102   // 화재 감지 (고온 등) 등급
} SensorID;

typedef enum {
    TYPE_UINT8   = 0,  // 1바이트 정수
    TYPE_INT32   = 1,  // 4바이트 정수
    TYPE_FLOAT   = 2,  // 4바이트 실수
    TYPE_BOOL    = 3   // 1바이트 논리
} DataType;

#endif