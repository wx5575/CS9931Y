#ifndef __jj98_H__
#define __jj98_H__

#include "stm32f4xx.h"
/************************注意***************************/
// 1、 使用串口程序前需先在上位机上运行随机附送的光
// 盘中的 sscomv20.exe 程序，安装串口调试器。
// 2、 电压、频率可以由上位机控制下位机，并且下位机
// 可以发送测量数据给上位机；电流、功率只是下位
// 机发送数据给上位机用。
// 3、 只有设备处于停止状态，下位机才能接收上位机发
// 送的电压、频率设定值；只有设备处于启动状态，
// 上位机才能接收下位机发送的电压、频率、电流、
// 功率的实测值
/*******************************************************/
#pragma pack (push,1)   /*指定1字节对齐*/  
typedef struct{
	uint8_t direction; //通讯的方向 0x11 上位机给下位机发送 0x10 下位机给上位机发送
	union{
		uint16_t value;//数据
		uint8_t buf[2];//分解数据为高低字节
	}val;
	union{
		uint8_t control;//控制字节
		struct{
			uint8_t rw:1;//上位机读写控制位
			uint8_t decs_num:2;//小数点个数控制位
			uint8_t vol:1;//1 表示数据是电压
			uint8_t freq:1;//1 表示数据是频率
			uint8_t cur:1;//1 表示数据是电流
			uint8_t pw:1;//1 表示数据是功率
			uint8_t ctrl:1;//控制变频电源的启动和停止
		}bits;
	}ctrl;
}jj98_FRAME;
#pragma pack (pop)      /*还原默认字节对齐*/  

/*****************************/
#define DIR_MASTER_W	0x11	//写 上位机==>>从机(变频电源)
#define DIR_MASTER_R	0x10	//读 上位机==>>从机(变频电源)

/*****************************/
#define ERR_BYTE_1 	0XAA
#define ERR_BYTE_2 	0X55
#define ERR_BYTE_3 	0XFF
#define ERR_BYTE_4 	0X00
/*****************************/
#define CTRL_MASTER_READ	1	//实测值
#define CTRL_MASTER_WRITE	0	//设定值

#define CTRL_DECS_NUM_0		0	//无小数位
#define CTRL_DECS_NUM_1		1	//一位小数位
#define CTRL_DECS_NUM_2		2	//两位小数位
#define CTRL_DECS_NUM_3		3	//三位小数位

#define CTRL_SLAVE_START	1	//启动设备
#define CTRL_SLAVE_STOP		0	//停止设备，上位机送给下位机数据，并可以记忆
/*****************************/

extern void jj98_stop_test(void);
extern void jj98_start_test(void);
extern void jj98_set_vol(uint16_t vol, uint8_t decs);
extern void jj98_get_vol(uint8_t decs);
extern void jj98_set_freq(uint16_t freq, uint8_t decs);
extern void jj98_send_frame_to_slove(void);
extern void jj98_comm_analysis(uint8_t data);

#endif //__jj98_H__
