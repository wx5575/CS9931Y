#ifndef __MODBUS_H
#define __MODBUS_H

#include "zwd414b.h"




typedef enum {
    MODBUS_BUSY,
    MODBUS_EMPTY,
}MODBUS_STATUS;

typedef enum {
    REFRESH,
    N_REFRESH,
}REFRESH_STATUS;

/*上位机发送的指令中固定的字节的位置*/
enum {
    TARGET_ADDR_BYTE ,
    CMD_BYTE ,

    MEM_ADDR_BYTE_HIGH,
    MEM_ADDR_BYTE_LOW,

    MEM_NUM_HIGH,
    MEM_NUM_LOW,

    PARAM_NUM,

    PARAM_BASE,

    QUEUE_STR_SIZE = 15,
}; 

#define     TEST_CRC_HIGH          PARAM_NUM
#define     TEST_CRC_LOW           ( TEST_CRC_HIGH +1 )



typedef struct {
    uint8_t target_addr;
    uint8_t cmd;
    uint16_t memory_addr;
    uint16_t memory_num;
    uint8_t param_num;
    uint8_t param_ch[100];  


}ModBus_Receive_Inf;

#define     QUEUE_LEN          (16)
#define     SEND_CMD_LEN      (16)


//typedef struct {
//    char Send_Queue[QUEUE_SIZE][SEND_DATA_SIZE];
//    uint8_t queue_rear;
//    uint8_t queue_front;
//
//}Send_Data_Msg;




typedef struct {                    
    uint8_t Send_Queue[QUEUE_LEN][SEND_CMD_LEN];
    uint8_t queue_front;
    uint8_t queue_rear; 
    MODBUS_STATUS ModBus_Status;                
}SEND_QUEUE_MSG;

extern SEND_QUEUE_MSG send_msg;

extern void ModBus_Init(void);

//extern Send_Data_Msg send_data_msg;

extern MODBUS_STATUS ModBus_Status;
extern ErrorStatus ModBus_Receive_data_Dispose(uint8_t data_num, uint8_t * receive_data, ModBus_Receive_Inf * ModBus_Receive_data);
extern void ModBus_send_data(void);

#endif

