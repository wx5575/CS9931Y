/******************************************************************************
 * 文件信息 :  
 *
 * 创 建 者 :  张旭
 *
 * 创建日期 :  2007.09.29
 * 
 * 原始版本 : 
 *     
 * 修改版本 :  
 *    
 * 修改日期 : 
 *
 * 修改内容 :
 * 
 * 审 核 者 :
 *
 * 附    注 :
 *
 * 描    述 :   源代码
 *
 * 版    权 :   南京长盛仪器有限公司 , Copyright Reserved
 * 
******************************************************************************/

/******************************************************************************
 *                              头文件卫士
******************************************************************************/ 
 
#ifndef    _SET_PANEL_H
#define    _SET_PANEL_H

#if defined(__cplusplus)

    extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
			
#ifndef    _API_SCHED_H

    #include    "Api_sched.h"     
#endif

		

#ifndef    _SCPI_PARSER_H_

    #include    "scpi_parser.h"     
#endif
			
/******************************************************************************
 *                             包含文件声明
******************************************************************************/ 

/******************************************************************************
 *                           文件接口信息宏定义
******************************************************************************/
			
#define uint8		uint8_t
extern uint32_t Lock_key_counter;//第一次进入设置界面对所需响应或不响应按键执行操作
extern uint32_t test_mode_flag;
extern uint16_t ac_dc_flag;
extern uint32_t test_display_flag;
extern uint32_t Set_Resistor_time_flag;
extern uint32_t time_dc_flag;
extern uint32_t Set_Current_flag;
extern uint32_t Set_Test_mode;
extern uint16_t Set_Mode_mark;
// extern uint32_t time_flag;								//设置AC时间时判断是否有小数点的标志为0的时候有小数点为1的时候没有小数点
/******************************************************************************
 *                         文件接口数据结构体声明
******************************************************************************/
// 仪器型号宏定义 增加或减少型号时要修改 Model_ReadSysModel()和 MEM_InitAllMemParam()函数
#define             CS9931YS                 	(0)
           

#define             MAX_MODEL_NUMB              (1)    // 型号个数


typedef struct
{
    uint8_t                       *m_InstrumentModel[MAX_MODEL_NUMB];     // 仪器型号
    uint8_t                       *m_InstrumentID;                        // 仪器ID号
    uint8_t                       *m_InstrumentSoftwareVersion;           // 仪器软件版本号

}InstrumentAttrConfig;

// typedef struct _ENV_PARAM_
// {
//     uint8_t                    m_sysModel;      // 机型

//     uint8_t                    m_keySound;      // 按键声音
//     //uint8                    m_keyLock;       // 按键锁

// }EnvParam;

// extern EnvParam t_EnvParam;    

extern InstrumentAttrConfig t_InstrumentAttrConfig;      

extern const PanelUnit  t_SetPanel;

// extern void LIB_Set_Test_Mode(void);

extern void LIB_Set_Time(uint32_t time);

extern void LIB_Set_Current(uint32_t current); //写入8G,9G

extern void LIB_WriteAdd(uint16_t mode, uint32_t voltage);//写入G1

extern void LIB_Set_Auto_Manual(uint32_t flag);

// extern void _Set_current_enter_control(void);//保存参数时候调用的函数
/******************************************************************************
 *                            文件接口函数声明
******************************************************************************/ 



/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/

#if defined(__cplusplus)

    }
#endif 

#endif
