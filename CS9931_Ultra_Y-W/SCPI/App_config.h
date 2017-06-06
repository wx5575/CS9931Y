#ifndef    _APP_CONFIG_H_
#define    _APP_CONFIG_H_


#define	        DEBUG_VERSION	  					            (1)
//#define    RELEASE_VERSION                                    (1)
//定制版本 -> 影响版本号的组成方式
#define         CUSTOMIZE_VERSION                               (0)
//仪器名字最大长度
#define         INSTRUMENT_NAME_MAX_LEN                         (15)  

/*******************************************************************************
 *                               软件版本宏定义
********************************************************************************/

//格式为 x.x.xx           MAJOR_VERSION.MINOR_VERSION.REVISION_VERSION
//规定：主版本号0系列为研发阶段版本    主版本号1系列以上为生产阶段版本
#define         MAJOR_VERSION                                   ('1')
#define         MINOR_VERSION                                   ('0')
#define         REVISION_HIGH_VERSION                           ('0')
#define         REVISION_LOW_VERSION                            ('1')

//定制版本版本号
//格式为 x.x.xx-xx        MAJOR_VERSION.MINOR_VERSION.REVISION_VERSION-CUSTOMIZE_VERSION
#if (CUSTOMIZE_VERSION > 0)
    
    #define     CUSTOMIZE_HIGH_VERSION                          ('0')
    #define     CUSTOMIZE_LOW_VERSION                           ('0')
#endif            

/*******************************************************************************
 *                               已使用标示宏定义
********************************************************************************/

#define         APP_USED_FLG                				    (0xa5)
#define	        SYSTERM_FIRST_RUN_ID		                    (0xa5)

/*******************************************************************************
 *                          应用运行过程宏定义配置
********************************************************************************/

//面板支持按键定义     -----    屏幕功能键    范围：0x00 - 0x1f
#define         KEY_FUNC_CODE                                   (0x00)
#define  		KEY_FUNC_F1                                     (KEY_FUNC_CODE+1) 
#define  		KEY_FUNC_F2                                     (KEY_FUNC_F1+1) 
#define  		KEY_FUNC_F3                                     (KEY_FUNC_F2+1)  
#define  		KEY_FUNC_F4                                     (KEY_FUNC_F3+1) 

//面板支持按键定义     -----    系统功能键    范围：0x40 - 0x5f
#define  		KEY_SYST_CODE                                   			(0x40) 
#define  		HAL_KEY_VAL_WITHSTAND                                   	(KEY_SYST_CODE+1)
#define  		HAL_KEY_VAL_INSULATION                                  	(HAL_KEY_VAL_WITHSTAND+1) 
#define  		HAL_KEY_VAL_UPPER                                     		(HAL_KEY_VAL_INSULATION+1) 
#define  		HAL_KEY_VAL_AUTO_MANUAL                                  	(HAL_KEY_VAL_UPPER+1)
#define  		HAL_KEY_VAL_LOWER                                  			(HAL_KEY_VAL_AUTO_MANUAL+1) 
#define  		HAL_KEY_VAL_MODE                                   			(HAL_KEY_VAL_LOWER+1) 
#define  		HAL_KEY_VAL_ARC                                   			(HAL_KEY_VAL_MODE+1)
#define  		HAL_KEY_VAL_TIME                                    		(HAL_KEY_VAL_ARC+1)
#define  		HAL_KEY_VAL_LEFT                                 			(HAL_KEY_VAL_TIME+1) 
#define  		HAL_KEY_VAL_EXIT                                 			(HAL_KEY_VAL_LEFT+1) 
#define  		HAL_KEY_VAL_UP                                 				(HAL_KEY_VAL_EXIT+1) 
#define  		HAL_KEY_VAL_DOWN                                 			(HAL_KEY_VAL_UP+1) 
#define  		HAL_KEY_VAL_RIGHT                                 			(HAL_KEY_VAL_DOWN+1) 
#define  		HAL_KEY_VAL_ENTER                                 			(HAL_KEY_VAL_RIGHT+1) 
#define  		HAL_KEY_VAL_START                                 			(HAL_KEY_VAL_ENTER+1)
//面板调度键定义       -----    调度键
#define  		SCHED_CODE                                      (0xa0) 
#define  		SCHED_RECV_FRAME                                (SCHED_CODE+1)
#define  		SCHED_LANGUAGE_SWITCH                           (SCHED_RECV_FRAME+1)
#define  		SCHED_TEST_START                                (SCHED_LANGUAGE_SWITCH+1)
#define         SCHED_TIME_REFRESH                              (SCHED_TEST_START+1)
#define  		SCHED_TEST_PASS                                 (SCHED_TIME_REFRESH+1)
#define  		SCHED_TEST_FAIL                                 (SCHED_TEST_PASS+1)
#define  		SCHED_RESU_SAVE                                 (SCHED_TEST_FAIL+1)
#define  		SCHED_TEST_HALT                                 (SCHED_RESU_SAVE+1)
#define  		SCHED_TEST_RESET                                (SCHED_TEST_HALT+1)

/*******************************************************************************
 *                            用户自定义消息ID宏定义
********************************************************************************/

//消息ID索引定义
#define         KEY_FUNC_F1_ID_INDEX                            (MSG_USER_ID_INDEX+1)
#define         KEY_FUNC_F2_ID_INDEX                            (KEY_FUNC_F1_ID_INDEX+1)
#define         KEY_FUNC_F3_ID_INDEX                            (KEY_FUNC_F2_ID_INDEX+1)
#define         KEY_FUNC_F4_ID_INDEX                            (KEY_FUNC_F3_ID_INDEX+1)

#define  		KEY_VAL_WITHSTAND_ID_INDEX                      (KEY_FUNC_F4_ID_INDEX+1)
#define  		KEY_VAL_INSULATION_ID_INDEX                     (KEY_VAL_WITHSTAND_ID_INDEX+1) 
#define  		KEY_VAL_UPPER_ID_INDEX                          (KEY_VAL_INSULATION_ID_INDEX+1) 
#define  		KEY_VAL_AUTO_MANUAL_ID_INDEX                    (KEY_VAL_UPPER_ID_INDEX+1)
#define  		KEY_VAL_LOWER_ID_INDEX                         	(KEY_VAL_AUTO_MANUAL_ID_INDEX+1) 
#define  		KEY_VAL_MODE_ID_INDEX                          	(KEY_VAL_LOWER_ID_INDEX+1) 
#define  		KEY_VAL_ARC_ID_INDEX                          	(KEY_VAL_MODE_ID_INDEX+1)
#define  		KEY_VAL_TIME_ID_INDEX                           (KEY_VAL_ARC_ID_INDEX+1)
#define  		KEY_VAL_LEFT_ID_INDEX                        	(KEY_VAL_TIME_ID_INDEX+1)
#define  		KEY_VAL_EXIT_ID_INDEX                        	(KEY_VAL_LEFT_ID_INDEX+1)
#define  		KEY_VAL_UP_ID_INDEX                        		(KEY_VAL_EXIT_ID_INDEX+1)
#define  		KEY_VAL_DOWN_ID_INDEX                        	(KEY_VAL_UP_ID_INDEX+1)
#define  		KEY_VAL_RIGHT_ID_INDEX                        	(KEY_VAL_DOWN_ID_INDEX+1)
#define  		KEY_VAL_ENTER_ID_INDEX                        	(KEY_VAL_RIGHT_ID_INDEX+1)
#define			KEY_VAL_START_ID_INDEX							(KEY_VAL_ENTER_ID_INDEX+1)

//消息ID定义
#define         KEY_FUNC_F1_ID                                  			MSG_ID_GET(KEY_FUNC_F1_ID_INDEX)
#define         KEY_FUNC_F2_ID                                  			MSG_ID_GET(KEY_FUNC_F2_ID_INDEX)
#define         KEY_FUNC_F3_ID                                  			MSG_ID_GET(KEY_FUNC_F3_ID_INDEX)
#define         KEY_FUNC_F4_ID                                  			MSG_ID_GET(KEY_FUNC_F4_ID_INDEX)

#define  		KEY_VAL_WITHSTAND_ID                                	MSG_ID_GET(KEY_VAL_WITHSTAND_ID_INDEX)
#define  		KEY_VAL_INSULATION_ID                               	MSG_ID_GET(KEY_VAL_INSULATION_ID_INDEX)
#define  		KEY_VAL_UPPER_ID                                  		MSG_ID_GET(KEY_VAL_UPPER_ID_INDEX) 
#define  		KEY_VAL_AUTO_MANUAL_ID                               	MSG_ID_GET(KEY_VAL_AUTO_MANUAL_ID_INDEX)
#define  		KEY_VAL_LOWER_ID                               			MSG_ID_GET(KEY_VAL_LOWER_ID_INDEX)
#define  		KEY_VAL_MODE_ID                                			MSG_ID_GET(KEY_VAL_MODE_ID_INDEX)
#define  		KEY_VAL_ARC_ID                                			MSG_ID_GET(KEY_VAL_ARC_ID_INDEX)
#define  		KEY_VAL_TIME_ID                                 		MSG_ID_GET(KEY_VAL_TIME_ID_INDEX)
#define  		KEY_VAL_LEFT_ID                              			MSG_ID_GET(KEY_VAL_LEFT_ID_INDEX)
#define  		KEY_VAL_EXIT_ID                              			MSG_ID_GET(KEY_VAL_EXIT_ID_INDEX)
#define  		KEY_VAL_UP_ID                              				MSG_ID_GET(KEY_VAL_UP_ID_INDEX)
#define  		KEY_VAL_DOWN_ID                             			MSG_ID_GET(KEY_VAL_DOWN_ID_INDEX)
#define  		KEY_VAL_RIGHT_ID                              			MSG_ID_GET(KEY_VAL_RIGHT_ID_INDEX)
#define  		KEY_VAL_ENTER_ID                              			MSG_ID_GET(KEY_VAL_ENTER_ID_INDEX)
#define  		KEY_VAL_START_ID                               			MSG_ID_GET(KEY_VAL_START_ID_INDEX)
/*******************************************************************************
 *                   仪器面板ID宏定义 -- 每个面板ID号必须不同
********************************************************************************/

#define  		START_UP_PANEL_ID                               (0x01)
#define         MAIN_PANEL_ID                                   (0x02)
#define         SET_PANEL_ID                                    (0x03)
#define         ADJUST_PANEL_ID                                 (0x04)
/*******************************************************************************              
 *                            故障代码宏定义
********************************************************************************/

#define         IIC_FLASH_ACCESS_ERR                            (1)
#define         PARALLEL_FLASH_ACCESS_ERR                       (2)
#define         SPI_FLASH_ACCESS_ERR                            (3)

/*******************************************************************************
 *                              应用数据类型部分配置
********************************************************************************/

//结束符定义
typedef enum
{
    CR_LF_INDEX = 0,
    LF_INDEX,
    CODE_0x23_INDEX,                                            //#号

}END_CODE;

#endif

