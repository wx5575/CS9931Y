#ifndef __AD_DA__H__
#define __AD_DA__H__


/*包含的头文件*/
#include <stdint.h>
#include "stm32f4xx.h"
#include "CS9931_Config.h"


/* define ---------------------------------------------------------------------*/
#define    VREF_SELF_AD_IN        (0)
#define    W_V_AD_IN              (1)
#define    W_I_AD_IN              (2)
#define    GR_I_AD_IN             (3)
#define    GR_V_AD_IN             (4)


#define   W_VREF                  (0)
#define   GR_VREF                 (0)

#if (LC_TEST_MODE==LC_WY)
#define   LC_VREF                 (0) //使用外部电源
#define   LC_ASSIT_VREF           (0) //2016.9.8 wangxin
#else
#define   LC_VREF                 (0)
#define   LC_ASSIT_VREF           (1) //2016.9.8 wangxin
#endif


#define   Short_VREF              (2)
#define   ARC_VREF                (3)
#define   ARC_4us_VREF            (4)
#define   ARC_10us_VREF           (5)
#define   ARC_20us_VREF           (6)
#define   ARC_40us_VREF           (7)

extern  uint16_t Read_AD_Value(uint8_t ch);
extern  uint16_t Read_AD_Value_Cal(uint8_t ch);
extern  void  AD_DA_Config(void);
extern  void CD4054_DAC_OUTPUT(uint8_t ch);
extern  void VREF_SelfTest_CH_Change(uint8_t ch);
extern  void DAC_SetValue(uint8_t ch,uint16_t value);
extern  uint16_t DAC_GetValue(uint8_t ch);
extern  void VREF_SelfTest_CH_Change(uint8_t ch);
#endif



