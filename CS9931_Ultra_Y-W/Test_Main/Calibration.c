#include "Calibration.h"

#include "USART.h"
#include "Sched.h"
#include "stdio.h"
#include "Output_Control.h"
#include "spi_flash.h"
#include "AD_DA.h"
#include "Relay.h"
#include "Relay_Change.h"

#if defined(__cplusplus)

    extern "C" {     /* Make sure we have C-declarations in C++ programs */

#endif

/**************************************************************************
 *                           函数内联关键字宏定义 -- 编译器相关
***************************************************************************/ 

#define     INLINE                                   __inline 

uint16_t AC_facter_index = 0;
uint16_t DC_facter_index = 0;
			
uint8_t AC_Calibration(uint8_t data)
{
	uint16_t j;
	uint32_t value = 0,facter = 1;
	static uint8_t buffer[10] = {0};
	static uint8_t i = 0;
	if(data == 'S'){
		uint8_t *p = (uint8_t *)AC_facter;
		printf("开始校准\r\n");
		AC_facter_index = 0;
		AC_SetVoltage(1,60);
		for(j=0;j<sizeof(AC_facter);j++){
			
			*(p+j) = 0;
		}
		DAC_SetValue(W_VREF,AC_CALIBRATE_POINT[AC_facter_index]);
		GUI_DispDecAt(AC_CALIBRATE_POINT[AC_facter_index],200,100,4);
		return 0;
	}
	if(data == 'Q'){printf("停止校准\r\n");AC_SetVoltage(0,60);AC_facter_index=0;return 0;}
	if(data == 'n'){
		if(AC_facter[AC_facter_index].Voltage!=0)AC_facter_index++;
		DAC_SetValue(W_VREF,AC_CALIBRATE_POINT[AC_facter_index]);
		GUI_DispDecAt(AC_CALIBRATE_POINT[AC_facter_index],200,100,4);
		printf("          \r");
		return 0;
	}
	if(data >= '0' && data <= '9'){
		Usart2_PutChar(data);
		buffer[i++] = data - '0';
		return 0;
	}
	if(data == 0x0D)              //回车
	{
		if(i==0 || i>9){i=0;return 0;}			
		j = i;
		for(;i>0;i--){
			value += buffer[i-1] * facter;
			facter *= 10;
		}
		/********************
		* 添加数据处理函数  *
		********************/
		
		GUI_DispDecAt(value,400,300,j);
		GUI_DispString("V      ");
		printf("\r          \r");
		
		
		AC_facter[AC_facter_index].DA_value = AC_CALIBRATE_POINT[AC_facter_index];	
		AC_facter[AC_facter_index].Voltage = value;
		for(j=0;j<10;j++)AC_facter[AC_facter_index].AD_value = Read_AD_Value(W_V_AD_IN);
		
		if(AC_facter_index >= 39){
//			AC_facter[39].Voltage = 6069;
			flash_unlock();
			sf_WriteBuffer((uint8_t *)AC_facter,4096,sizeof(AC_facter));
			flash_lock();
			AC_SetVoltage(0,60);
			printf("校准完成\r\n");
			return 1;
		}else{return 0;}
	}
	return 0;
}


uint8_t DC_Calibration(uint8_t data)
{
	uint16_t j;
	uint32_t value = 0,facter = 1;
	static uint8_t buffer[10] = {0};
	static uint8_t i = 0;
	if(data == 'S'){
		uint8_t *p = (uint8_t *)DC_facter;
		printf("开始校准\r\n");
		DC_facter_index = 0;
		DC_SetVoltage(1);
		for(j=0;j<sizeof(DC_facter);j++){
			
			*(p+j) = 0;
		}
		DAC_SetValue(W_VREF,DC_CALIBRATE_POINT[DC_facter_index]);
		GUI_DispDecAt(DC_CALIBRATE_POINT[DC_facter_index],200,100,4);
		return 0;
	}
	if(data == 'Q'){printf("停止校准\r\n");DC_SetVoltage(0);DC_facter_index=0;return 0;}
	if(data == 'n'){
		if(DC_facter[DC_facter_index].Voltage!=0)DC_facter_index++;
		DAC_SetValue(W_VREF,DC_CALIBRATE_POINT[DC_facter_index]);
		GUI_DispDecAt(DC_CALIBRATE_POINT[DC_facter_index],200,100,4);
		printf("          \r");
		return 0;
	}
	if(data >= '0' && data <= '9'){
		Usart2_PutChar(data);
		buffer[i++] = data - '0';
		return 0;
	}
	if(data == 0x0D)              //回车
	{
		if(i==0 || i>9){i=0;return 0;}			
		j = i;
		for(;i>0;i--){
			value += buffer[i-1] * facter;
			facter *= 10;
		}
		/********************
		* 添加数据处理函数  *
		********************/
		
		GUI_DispDecAt(value,400,300,j);
		GUI_DispString("V      ");
		printf("\r          \r");
		
		
		DC_facter[DC_facter_index].DA_value = DC_CALIBRATE_POINT[DC_facter_index];	
		DC_facter[DC_facter_index].Voltage = value;
		for(j=0;j<10;j++)DC_facter[DC_facter_index].AD_value = Read_AD_Value(W_V_AD_IN);
		
		if(DC_facter_index >= 39){
//			AC_facter[39].Voltage = 6069;
			flash_unlock();
			sf_WriteBuffer((uint8_t *)DC_facter,8192,sizeof(DC_facter));
			flash_lock();
			DC_SetVoltage(0);
			printf("校准完成\r\n");
			return 1;
		}else{return 0;}
	}
	return 0;
}

uint8_t  const * const Current_Relay_Info[] = {
	"DC_100mA",
	"DC_20mA ",
	"DC_2mA  ",
	"DC_200uA",
	"DC_20uA ",
	"DC_2uA "
};

uint8_t AC_Current_Calibration(uint8_t data)
{
	uint16_t j;
	static uint8_t flag = 0;
	uint32_t value = 0,facter = 1;
	static uint8_t buffer[10] = {0};
	static uint8_t i = 0;
	if(data == 'S'){
		uint8_t *p = (uint8_t *)AC_Current_facter;
		
		if(flag == 0){
			flag = 1;
			printf("开始校准\r\n");
			AC_facter_index = 0;
			GUI_DispStringAt((const char *)Current_Relay_Info[AC_facter_index],300,0);
			Sampling_Relay_State_CHange(AC_facter_index);
		
			Delay_ms(10);
			for(j=0;j<sizeof(AC_Current_facter);j++){
				*(p+j) = 0;
			}
			printf("请先确认设备档位后按\"S\"继续\r\n");
			return 0;
		}
		
		AC_SetVoltage(500,60);
		flag = 0;
		return 0;
	}
	if(data == 'Q'){printf("停止校准\r\n");AC_SetVoltage(0,60);AC_facter_index=0;return 0;}
	if(data == 'n'){
		if(AC_Current_facter[AC_facter_index].Current!=0){
			AC_facter_index++;
			AC_SetVoltage(0,60);
			Sampling_Relay_State_CHange(AC_facter_index);
			GUI_DispStringAt((const char *)Current_Relay_Info[AC_facter_index],300,0);
			Delay_ms(10);
			printf("          \r");
		}
		if(flag == 0){
			flag = 1;
			printf("请先确认设备档位后按\"n\"继续\r\n");
			return 0;
		}
		AC_SetVoltage(500,60);
		flag = 0;
		return 0;
	}
	if(data >= '0' && data <= '9'){
		Usart2_PutChar(data);
		buffer[i++] = data - '0';
		return 0;
	}
	if(data == 0x0D)              //回车
	{
		if(i==0 || i>9){i=0;return 0;}			
		j = i;
		for(;i>0;i--){
			value += buffer[i-1] * facter;
			facter *= 10;
		}
		/********************
		* 添加数据处理函数  *
		********************/
		
		GUI_DispDecAt(value,400,300,j);
		GUI_DispString("mA      ");
		printf("\r          \r");
		
		
		AC_Current_facter[AC_facter_index].Current = value;	
		for(j=0;j<10;j++)AC_facter[AC_facter_index].AD_value = Read_AD_Value(W_I_AD_IN);
		
		if(AC_facter_index >= 5){
//			AC_facter[39].Voltage = 6069;
			flash_unlock();
			sf_WriteBuffer((uint8_t *)AC_Current_facter,4096*3,sizeof(AC_Current_facter));
			flash_lock();
			AC_SetVoltage(0,60);
			printf("校准完成\r\n");
			return 1;
		}else{return 0;}
	}
	return 0;
}

