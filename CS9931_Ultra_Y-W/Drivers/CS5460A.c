
#include "CS5460A.h"

#define SPI_CLK_HIGH()		
#define SPI_CLK_LOW()		
#define SPI_MOSI_HIGH()		
#define SPI_MOSI_LOW()		
#define SPI_MISO_ST()		
#define SPI_CS_EN()		
#define SPI_CS_DIS()	


void send_SPI_BYTE(unsigned char data)
{
	int i = 0;
	
	for(i = 0; i < 8; i++)
	{
		SPI_CLK_HIGH();
		
		if(data & 0x80)
		{
			SPI_MOSI_HIGH();
		}
		else
		{
			SPI_MOSI_LOW();
		}
		
		data <<= 1;
		SPI_CLK_LOW();
	}
}

void Write_CS5460A(unsigned char*buf, unsigned short n)
{
	for(int i = 0; i < n; i++)
	{
		send_SPI_BYTE(buf[i]);
	}
}

void CS5460A_Init(void)
{
	unsigned char buf[5];
	
// 	reset_5460=0; //reset_5460 为 CS5460A 的复位脚
// 	Delay_10MS();
// 	reset_5460=1; //复位 CS5460A
	buf[0]=0xff; //SYNC1
	buf[1]=0xff; //SYNC1
	buf[2]=0xff; //SYNC1
	buf[3]=0xfe; //SYNC0
	Write_CS5460A(buf,4); //写 3 个同步命令 1 之后再写 1 个同步命令 0
	buf[0]=0x40; //写配置寄存器
	buf[1]=0x01; //GI=1，电流通道增益=50
	buf[2]=0x00;
	buf[3]=0x01; //DCLK=MCLK/1
	Read_Memory(&temp,phase_addr,1);
	if(temp==0xA5)
	{
	Read_Memory(&temp,phase_addr+1,1);
	buf5460[1]=temp;
	}
	//假如已经执行过相位补偿，设置相位补偿值，否则设置相位补偿值为 0
	Write_CS5460A(buf,4);
	//EEPROM 保存校准的电流/电压校准值。
	//假如指定地址单元等于 OXA5，则接下来的 3BYTES 即是校准值。
	Load_Rom_To_5460(0x10,0x42);//写直流电流偏置校准寄存器
	Load_Rom_To_5460(0x20,0x46);//写直流电压偏置校准寄存器
	Load_Rom_To_5460(0x30,0x44);//写电流增益校准寄存器
	Load_Rom_To_5460(0x40,0x48);//写电压增益校准寄存器
	Load_Rom_To_5460(0x50,0x60);//写交流电流偏置校准寄存器
	Load_Rom_To_5460(0x60,0x62);//写交流电压偏置校准寄存器
	buf[0]=0x5e;
	buf[1]=0xff;
	buf[2]=0xff;
	buf[3]=0xff;
	Write_CS5460A(buf,4); //清状态寄存器
	buf[0]=0x74;
	buf[1]=0x00;
	buf[2]=0x00;
	buf[3]=0x00;
	Write_CS5460A(buf,4); //写中断屏蔽寄存器，缺省值CS5460A 使用说明
	5
	buf[0]=0x78;
	buf[1]=0x00;
	buf[2]=0x00;
	buf[3]=0x00; //缺省值
	Write_CS5460A(buf,4); //写控制寄存器
	buf[0]=0x4c;
	buf[1]=0x00;
	buf[2]=0x34;
	buf[3]=0x9C;
	Write_CS5460A(buf,4); //写 EOUT 脉冲输出寄存器
	buf[0]=0x4A;
	buf[1]=0x00;
	buf[2]=0x01; //每秒钟计算 10 次， N=400
	buf[3]=0x90;
	Write_CS5460A(buf,4); //写 CYCLE COUNT 寄存器
	Read_CS5460A(0x1e,buf); //读状态寄存器
	Buf[3]=buf[2];
	Buf[2]=buf[1];
	Buf[1]=buf[0];
	Buf[0]=0X5E;
	Write_CS5460A(buf,4); //写状态寄存器
	Buf[0]=0xe8;
	Write_CS5460A(buf,1); //启动 CS5460A
}







