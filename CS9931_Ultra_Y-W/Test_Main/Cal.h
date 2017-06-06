#ifndef       __CAL_H__
#define       __CAL_H__

#include      "stdint.h"


#define       LC_ASSIST    (255)


//校准数据类型定义


//耐压的校准系数结构
typedef struct{
	uint16_t Vol_Cal_Point_Num;       //电压有效校准点的个数
	uint16_t Cur_Cal_Point_Num;       //电流有效校准点的个数
	struct{
		uint16_t  DA_value;             //输出的DA值
		uint16_t  AD_value;             //电压读取的AD值		
		uint16_t  Voltage;              //实际的电压值
	}Vol_Cal_Point[40];
	struct{
		uint16_t  AD_value;             //电流读取的AD值
		uint32_t  Current;              //实际的电流值
	}Cur_Cal_Point[10];
	uint16_t crc;
}CW_FACTER;                         

//接地的校准系数结构
typedef  struct{
	uint16_t  GR_Cal_Point_Num;       //接地较准点的个数
	struct{
		uint16_t  I_DA_value;           //电流输出的DA值
		uint16_t  I_AD_value;           //电流读取的AD值
		uint16_t  V_AD_value;           //电压读取的AD值
		uint16_t  I_Value;              //实际的电流值
	}GR_Cal_Point[10];
}GR_FACTER;

//泄漏电压的校准系数结构
typedef  struct{
	uint16_t  LC_Cal_Point_Num;       //泄漏较准点的个数
	struct{
		uint16_t  DA_value;             //输出的DA值
		uint16_t  AD_value;             //电压读取的AD值
		uint32_t  Voltage;              //实际的电压值
	}LC_Cal_Point[20];
}LC_V_FACTER;
typedef  struct{
	uint32_t Current; ///<校准输入电流值
	uint32_t AD_Currrent;///<校准输入电流AD值
}LC_MC_FACTER;


//泄漏电流的校准系数结构
typedef  struct{
	uint16_t  LC_Cal_Point_Num;       //泄漏较准点的个数
	struct{
		uint16_t  AD_value;             //电压读取的AD值
		uint32_t  Current;              //实际的电压值
	}LC_Cal_Point[3][4][3];           //3种网络、4种检波方式、3个档位
}LC_CUR_FACTER;  

//ARC的校准系数结构
typedef  struct{
	uint16_t  ACW_ARC_Base;           //ACW的ARC较准点
	uint16_t  DCW_ARC_Base;           //DCW的ARC较准点
}ARC_FACTER;  

//SELV的校准系数结构
typedef  struct{
	uint16_t  LC_Cal_Point_Num;       //泄漏较准点的个数
	struct{
		uint16_t  AD_value;             //电压读取的AD值
		uint32_t  Voltage;              //实际的电压值
	}LC_Cal_Point[10];        
}SELV_FACTER;

//IR的校准系数结构
typedef  struct{
	struct{
		uint16_t  AD_value[10];             //电流读取的AD值
		uint32_t  Voltage[10];              //实际的电压值
	}Cal_Point[10];       
}IR_FACTER;


/*存贮结构体数据类型定义*/
typedef struct
{
	CW_FACTER      ACW_Facter;        //交流校准系数
	CW_FACTER      DCW_Facter;        //直流校准系数
	GR_FACTER      GR_Facter;         //接地校准系数
	LC_V_FACTER    LC_M_Facter;       //泄漏主电压校准系数
	LC_V_FACTER    LC_A_Facter;       //泄漏辅助电压校准系数
	LC_CUR_FACTER  LC_C_Facter;       //泄漏电流校准系数
	ARC_FACTER     ARC_Facter;        //ARC校准系数
	SELV_FACTER    SELV_Facter;       //SELF电压校准系数
	IR_FACTER      IR_Facter;       //IR的电流校准系数
	LC_MC_FACTER    LC_MC_Facter;//泄漏主电流校准系数
}ALL_FACTER;


//全局共享变量申明
extern  ALL_FACTER  Global_Cal_Facter;




extern   uint8_t  ACW_Cal_Facter_Refresh(uint8_t item, uint8_t cal, uint32_t val);
extern   uint8_t  DCW_Cal_Facter_Refresh(uint8_t item, uint8_t cal, uint32_t val);
extern   uint8_t  GR_Cal_Facter_Refresh(uint8_t item, uint8_t cal, uint32_t val);
extern   uint8_t  LC_Cal_M_V_Facter_Refresh(uint8_t item, uint8_t cal, uint32_t val);
extern   uint8_t  LC_Cal_A_V_Facter_Refresh(uint8_t item, uint8_t cal, uint32_t val);


extern   void     Cal_Facter_Reserve(void);
extern   uint8_t  Cal_Facter_Recover(void);

#endif
