#ifndef __LC__H__
#define __LC__H__


/*包含的头文件*/
#include <stdint.h>
#include "stm32f4xx.h"


/*宏定义*/
#define   GBT12133                (0)
#define   GB7000_1                (1)
#define   I_200uA_2mA_20mA        (2)
#define   GB4943_GBT12113         (3)
#define   UL1563                  (4)
#define   AUX_VOL_Change_NL       (5)
#define   Signal_Dault            (6)
#define   M_VOL_Change_NL         (7)

#define   RELAY_4094_ON			  (1)
#define   RELAY_4094_OFF		  (0)

#define   I_FILES                 (8)
#define   I_AC_DC_AC              (9)
#define   GB9706_1                (10)
#define   LC_NY                   (11)
#define   ULN544NP                (12)
#define   MD_HI_GND               (13)
#define   FIG4_FIG5               (14)
#define   ULN544P                 (15)

#define   LC_OUT9                 (16)
#define   LC_OUT10                (17)
#define   LC_OUT11                (18)
#define   LC_OUT12                (19)
#define   SELV_AC_DC_AC           (20)
#define   S3_CONTROL              (21)
#define   G_Change                (22)
#define   FIG3_FIG4               (23)

#define   SELV_PEAK_DISCHARGE     (24)
#define   PEAK_DISCHARGE          (25)
#define   CD4051_A                (26)
#define   CD4051_B                (27)
#define   D15_CD4051_B            (28)
#define   D15_CD4051_A            (29)

#define   LC_OUT4                 (32)
#define   LC_OUT3                 (33)
#define   LC_OUT2                 (34)
#define   LC_OUT1                 (35)
#define   LC_OUT5                 (36)
#define   LC_OUT6                 (37)
#define   LC_OUT7                 (38)
#define   LC_OUT8                 (39)


#define    VOLTAGE_RMS_CMD        (0x18)		//读电压RMS命令
#define    CURRENT_RMS_CMD        (0x16)		//读电流RMS命令
#define    TRUE_POWER_CMD         (0x14)		//读有功功率

/*数据类型定义*/
typedef  struct{
	uint16_t  DA_value;
	uint16_t  AD_value;
	uint32_t  Voltage;
}LC_VOLTAGE_FACTER;



extern void InitCS5460A(void);

extern void HAL_CS5460Init(void);

extern uint8_t ReadRealCurrent(float *accessAddr);

extern uint32_t ReadCurrentRmsValue(uint32_t *value);

extern void HAL_CS5460Start(void);

extern uint32_t HAL_CS5460Read(uint8_t command);

extern void LC_Init(void);

extern void LC_4051_D1_SELECT(uint8_t channel);

extern void LC_4051_D15_SELECT(uint8_t channel);

extern void LC_Relay_Control(uint8_t index,uint8_t On_or_off,uint8_t Is_Updata);

extern uint16_t  D16_Mcp3202_Read(uint8_t channel); 

extern uint16_t  D3_Mcp3202_Read(uint8_t channel); 

extern uint32_t LC_Get_Assist_Voltage(void);

extern void LC_Assit_Voltage_Set(uint32_t dst_voltage,uint16_t out_rate);

extern void LC_Main_Voltage_Set(uint32_t dst_voltage,uint16_t out_rate);

extern void CAL_LC_Main_Voltage_Set(uint32_t dst_voltage,uint16_t out_rate);

extern void LC_Assit_ADValue_Set(uint32_t ADValue,uint16_t out_rate);

extern void LC_Main_ADValue_Set(uint32_t ADValue,uint16_t out_rate);

extern uint32_t LC_Get_Main_Voltage(void);

extern uint32_t get_lc_main_current(uint32_t *cur_value);

extern void LC_Main_Output_Enable(void);

extern void LC_Main_Output_Disable(void);

extern void LC_Assit_Output_Enable(void);

extern void LC_Assit_Output_Disable(void);

extern uint32_t LC_Get_Current(uint8_t net,uint8_t curdetection,uint8_t curgear);
extern uint32_t count_lc_current(uint16_t value, uint8_t net,uint8_t curdetection,uint8_t curgear);
extern uint32_t LC_Get_Selv_Voltage(void);

#endif
