#include "ModBus.h"

//    const uint8_t QUEUE_LEN = 16;
//    const uint8_t SEND_CMD_LEN = 16;

extern TEST_CMD test[5];

static const uint8_t auchCRCHi[];
static const uint8_t auchCRCLo[];
extern uint8_t receive_timeout_counter;

SEND_QUEUE_MSG send_msg;

#define     RECEIVE_PARAM_BYTE_NUM          MEM_ADDR_BYTE_HIGH 
#define     RECEIVE_PARAM_BYTE_CH           (RECEIVE_PARAM_BYTE_NUM + 1)   

MODBUS_STATUS ModBus_Status = MODBUS_EMPTY;


static ErrorStatus check_crc_ok(uint8_t data_num, uint8_t * receive_data);
static ErrorStatus check_target_addr_ok(uint8_t * receive_data);
static void Read_receive_cmd(uint8_t * receive_data, ModBus_Receive_Inf * ModBus_Receive_data);
static void Read_receive_memory_addr(uint8_t * receive_data, ModBus_Receive_Inf * ModBus_Receive_data);
static void Read_receive_memory_num(uint8_t * receive_data, ModBus_Receive_Inf * ModBus_Receive_data);
static void Read_receive_param_num(uint8_t * receive_data, ModBus_Receive_Inf * ModBus_Receive_data);
static void Read_receive_param_ch(uint8_t * receive_data, ModBus_Receive_Inf * ModBus_Receive_data);

void ModBus_Init(void)
{
    send_msg.queue_front = 0;
    send_msg.queue_rear = 0;
    send_msg.ModBus_Status = MODBUS_EMPTY;
}

ErrorStatus ModBus_Receive_data_Dispose(uint8_t data_num, uint8_t * receive_data, ModBus_Receive_Inf * ModBus_Receive_data)
{
    ErrorStatus status; 
        

    if(check_target_addr_ok(receive_data) == SUCCESS) {
        if(check_crc_ok(data_num , receive_data) == SUCCESS) {
            Read_receive_cmd(receive_data , ModBus_Receive_data); 
            
            /*
            根据指令内容，区分接收的数据的格式，
            测试格式的时候读取的数据内容为数据的字节个数和数据
            设置格式的时候读取的数据内容为寄存器的地址和寄存器的个数            
            */            
            if(ModBus_Receive_data->cmd == CMD_TEST) {
                Read_receive_param_num(receive_data , ModBus_Receive_data);  
                Read_receive_param_ch(receive_data , ModBus_Receive_data);
            }
            else if(ModBus_Receive_data->cmd == CMD_SET) {
                Read_receive_memory_addr(receive_data , ModBus_Receive_data);
                Read_receive_memory_num(receive_data , ModBus_Receive_data);
            }
        }
        else {
            status = ERROR;  
        }       
    }
    else {
        status = ERROR; 
    }
    
    return status;
}

/*
    校验设备地址是否是接收到的数据
*/
static ErrorStatus check_target_addr_ok(uint8_t * receive_data)
{
    ErrorStatus status;
    
    if(receive_data[TARGET_ADDR_BYTE] == this_target_address) {
        status = SUCCESS;  
    }  
    else {
        status = ERROR;  
    }

    return status;
}

// static uint16_t CrcCal(uint8_t receive_data , uint16_t crc_poly, uint16_t crc_data)
// {
//     uint8_t i;
//     receive_data *= 2;

//     for(i = 8; i > 0; i--) {
//         receive_data /= 2;
//         if((receive_data ^ crc_data) & 1 ) {
//             crc_data = (crc_data / 2) ^ crc_poly;       
//         }
//         else {
//             crc_data /= 2; 
//         }
//     }

//     return crc_data;
// }

// /*获取校验位*/
// static uint16_t GetCheckData(uint8_t data_num , uint8_t * receive_data)
// {
//     uint16_t crc_data = 0xffff;
//     uint8_t i;
//     const uint16_t CHECK_POLY = 0xa001;
//     
//     for(i = 0; i< data_num ; i++) {
//         crc_data = CrcCal(receive_data[i] , CHECK_POLY , crc_data );
//     }  
//       
//     return crc_data ;   
// }





/**************************************************** 
**函数原型：   INT16U ComputeCRC16(uint8_t *, uint8_t)	*
**功    能：   CRC计算函数							*
**入口参数:    databuf:	数据指针					*
*			   length:	数据长度					*
*			   crc:		初始的校验值				*
**返 回 值:     									*
*              CRC16值								*
****************************************************/
uint16_t ComputeCRC16(uint8_t *puchMsg, uint8_t usDataLen)
{
	uint8_t uchCRCHi = 0xFF; 														//初始化高字节
	uint8_t uchCRCLo = 0xFF; 														//初始化低字节
	uint16_t uIndex; 																//把CRC表
	while(usDataLen--){ 														//通过数据缓冲器
		uIndex = uchCRCHi ^ *puchMsg++;											//计算CRC
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
		uchCRCLo = auchCRCLo[uIndex];
	}
	return uchCRCHi << 8 | uchCRCLo;

}









/*
    判断接收到的数据的校验值是否和发送的校验值相等
*/
static ErrorStatus check_crc_ok(uint8_t data_num, uint8_t * receive_data)
{
    ErrorStatus status;
    uint16_t receive_crc ,crc_data ;
    
    receive_crc = ( receive_data[data_num - 2]<< 8  )|(receive_data[data_num - 1]);

   crc_data = ComputeCRC16(receive_data, data_num -2);  

   if(receive_crc == crc_data) {
        status = SUCCESS; 
   }
   else {
        status = ERROR; 
   }
      
   return status; 
}

/*
    读取接收数据的指令码
*/
static void Read_receive_cmd(uint8_t * receive_data, ModBus_Receive_Inf * ModBus_Receive_data)
{
    ModBus_Receive_data->cmd = receive_data[CMD_BYTE];  
}

static void Read_receive_memory_addr(uint8_t * receive_data, ModBus_Receive_Inf * ModBus_Receive_data)
{
    uint16_t memory_addr;
    
    memory_addr = (receive_data[MEM_ADDR_BYTE_HIGH] << 8) | receive_data[MEM_ADDR_BYTE_LOW];  

    ModBus_Receive_data->memory_addr = memory_addr;
}

static void Read_receive_memory_num(uint8_t * receive_data, ModBus_Receive_Inf * ModBus_Receive_data)
{
    uint16_t memory_num;
    
    memory_num = (receive_data[MEM_NUM_HIGH] << 8) | receive_data[MEM_NUM_LOW];  

    ModBus_Receive_data->memory_num = memory_num;
}

static void Read_receive_param_num(uint8_t * receive_data, ModBus_Receive_Inf * ModBus_Receive_data)
{
    ModBus_Receive_data->param_num = receive_data[RECEIVE_PARAM_BYTE_NUM];  
}
static void Read_receive_param_ch(uint8_t * receive_data, ModBus_Receive_Inf * ModBus_Receive_data)
{
    uint8_t i;

    for(i = 0; i< ModBus_Receive_data->param_num; i++) {
        ModBus_Receive_data->param_ch[i] = receive_data[RECEIVE_PARAM_BYTE_CH + i];  
    }
}


/*向ModBus发送队列写入测试指令*/
void ModBus_Write_TestCmd(MEMORY_ADDRESS test_cmd , uint8_t addr_len)
{
    uint16_t crc_data;    

    send_msg.Send_Queue[send_msg.queue_rear][TARGET_ADDR_BYTE] = this_target_address;
    send_msg.Send_Queue[send_msg.queue_rear][CMD_BYTE] = CMD_TEST; 
    send_msg.Send_Queue[send_msg.queue_rear][MEM_ADDR_BYTE_HIGH] = (test_cmd >>8) & 0xff;
    send_msg.Send_Queue[send_msg.queue_rear][MEM_ADDR_BYTE_LOW] = test_cmd & 0xff;

    send_msg.Send_Queue[send_msg.queue_rear][MEM_NUM_HIGH] = (addr_len >>8) & 0xff;
    send_msg.Send_Queue[send_msg.queue_rear][MEM_NUM_LOW] = addr_len & 0xff;

    crc_data = ComputeCRC16(send_msg.Send_Queue[send_msg.queue_rear], MEM_NUM_LOW + 1);

    send_msg.Send_Queue[send_msg.queue_rear][TEST_CRC_LOW] = (crc_data ) & 0xff;
    send_msg.Send_Queue[send_msg.queue_rear][TEST_CRC_HIGH ] = (crc_data >>8) & 0xff;

    send_msg.Send_Queue[send_msg.queue_rear][QUEUE_STR_SIZE] = TEST_CRC_LOW + 1;

    send_msg.queue_rear++;
}

void ModBus_Write_SetCmd(MEMORY_ADDRESS test_cmd , uint8_t addr_len , uint8_t param_len , uint8_t *param)
{
    uint16_t crc_data; 
    uint8_t i;   

    send_msg.Send_Queue[send_msg.queue_rear][TARGET_ADDR_BYTE] = this_target_address;
    send_msg.Send_Queue[send_msg.queue_rear][CMD_BYTE] = CMD_SET; 
    send_msg.Send_Queue[send_msg.queue_rear][MEM_ADDR_BYTE_HIGH] = (test_cmd >>8) & 0xff;
    send_msg.Send_Queue[send_msg.queue_rear][MEM_ADDR_BYTE_LOW] = test_cmd & 0xff;

    send_msg.Send_Queue[send_msg.queue_rear][MEM_NUM_HIGH] = (addr_len >>8) & 0xff;
    send_msg.Send_Queue[send_msg.queue_rear][MEM_NUM_LOW] = addr_len & 0xff;

    send_msg.Send_Queue[send_msg.queue_rear][PARAM_NUM] = param_len;
	
    for(i = 0; i < param_len; i++) {
        send_msg.Send_Queue[send_msg.queue_rear][PARAM_BASE + i] = param[i] ;
    }
	
    crc_data = ComputeCRC16(send_msg.Send_Queue[send_msg.queue_rear], (PARAM_BASE + param_len));
	
    send_msg.Send_Queue[send_msg.queue_rear][PARAM_BASE + param_len+ 1 ] = (crc_data ) & 0xff;
    send_msg.Send_Queue[send_msg.queue_rear][PARAM_BASE + param_len ] = (crc_data >>8) & 0xff;
	
    send_msg.Send_Queue[send_msg.queue_rear][QUEUE_STR_SIZE] = PARAM_BASE + param_len + 2;
	
    send_msg.queue_rear++;
}

/*
当ModBus状态为空闲，并且队列不为空的时候发送队列头结点数据
*/
void ModBus_send_data(void)
{
    if(send_msg.ModBus_Status == MODBUS_EMPTY)
	{
		if(send_msg.queue_rear == send_msg.queue_front)
		{
			send_msg.queue_front = 0;
		}
		
		rt_enter_critical ();
		Usart_SendData(send_msg.Send_Queue[send_msg.queue_front] ,send_msg.Send_Queue[send_msg.queue_front][QUEUE_STR_SIZE] );
		rt_exit_critical ();
		send_msg.ModBus_Status = MODBUS_BUSY;
		receive_timeout_counter = 50;  //开启计数器
    }
}



static const uint8_t auchCRCHi[] = {
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
	0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
	0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
	0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
	0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
	0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40
};
static const uint8_t auchCRCLo[] = {
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
	0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
	0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
	0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
	0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
	0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
	0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
	0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
	0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
	0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
	0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
	0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
	0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
	0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
	0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
	0x40
};

