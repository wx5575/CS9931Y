/******************************************************************************
 *                          本文件所引用的头文件
******************************************************************************/ 

#include "stm32f4xx.h"
// #include <lpc17xx.h>
// #include <system_lpc17xx.h>
// #include <core_cm3.h>
#include <stdint.h> 
#include    "api_sched.h"
#include    "Scpi_Parser.h"


/******************************************************************************
 *                            本文件内部宏定义
******************************************************************************/

#define                     PANEL_INSTALL_MSG                         (0x01)
#define                     SCHED_INSTALL_MSG                         (0x02)

#define NULL 0  
#define    FALSE                                            (0)
#define    TRUE                                            (1)
#define    portQUEUELEN_TYPE	uint32_t  
#define    portCPSR_TYPE	uint32_t
#define     INLINE                                          __inline

/******************************************************************************
 *                       本文件所定义的静态数据
******************************************************************************/ 

volatile portuBASE_TYPE         s_SchedulerRunStatus            = SCHEDULER_INIT;           //系统运行状态  
static volatile PanelUnit      *s_ptCurrentPanelUnit            = NULL;                     //当前面板指针
static volatile PanelUnit      *s_ptPrevPanelUnit          		= NULL;                     //前一面板指针
static volatile PanelUnit      *s_ptCurrentPanelUnitTmp         = NULL; 	     		    //当前面板指针暂存值
static volatile uint32_t  s_PanelInstallServMsgID;                                    //面板安装服务消息ID
static  Fp_pfPanelCallBackFunc *s_pfPanelCallBackFunc           = NULL;                     //面板回调函数值
static  uint32_t          s_SchedCommPanelMsgID           = 0;                        //调度器处理的面板共同信息ID 
static  Fp_pfTestSchedCBF      *s_PfTestSchedCBF                = NULL;				        //测试回调函数指针
static  Fp_pfSchedlerDriveMsgSampleCallBackFunc 
					           *s_pfSchedlerDriveMsgSampleCallBackFunc	             
                                                                = NULL;				        //调度器驱动消息采集回调函数指针
static  Fp_pfSchedCBF          *s_PfSchedCBF                    = NULL;                     //调度器回调函数指针 
static volatile SchedMsg        st_SchedMsg;                           		                //调度器消息  

/******************************************************************************
 *                       本文件所定义的全局结构体
******************************************************************************/

volatile PanelDriveMsg          t_PanelDriveMsg                      = {0};                 //面板消息
volatile PanelDriveMsg  		t_PanelDriveMsgSnapshot;                                    //面板驱动消息快照

static portuBASE_TYPE _API_SchedSendMessage(const PanelNoticeMsg *pmessage)
{
    portuBASE_TYPE   rt 	        		                    = TRUE;
    
    rt                                                          = s_pfPanelCallBackFunc((const PanelNoticeMsg *)pmessage);
    #if (DEBUG_VERSION > 0)
    {
        t_AppDebugWatch.m_uint8_1                               = rt;
    }
    #endif
    
    return rt;
}
static portuBASE_TYPE _API_SchedPanelDraw(PanelWindow *ptpanelWindow, portuBASE_TYPE drawInfo)
{
    if (NULL != ptpanelWindow)
    {
        //
    }

    return TRUE;
}

static portuBASE_TYPE _API_SchedSendMessage(const PanelNoticeMsg *pmessage);
static portuBASE_TYPE _API_SchedPanelDraw(PanelWindow *ptpanelWindow, portuBASE_TYPE drawInfo);


portCPSR_TYPE BSP_InterruptDisable(void)
{
    return TRUE;
}
#define portDISABLE_INTERRUPTS()					        BSP_InterruptDisable()
void BSP_InterruptEnable(portCPSR_TYPE level)
{
    
}
#define portENABLE_INTERRUPTS(level)			            BSP_InterruptEnable(level)
/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 填加一条驱动消息                                                              
 *                                                                           
 *  入口参数 : msgHandleStyle：驱动消息类型 分为远控、虚拟、真实三中类型
 *			   msgInstallCode: 消息安装码   用于判断面板是否安装了此消息
 *             msgID         : 消息ID       消息值                                                  
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 : 添加消息时 需要临界区的保护                                                               
 *             							                                                              
 *                                                                            
******************************************************************************/ 

static portuBASE_TYPE _int_API_RSDriveMsgUnitPut(DriveMsgQueue *pdriveMsgQueue, portuBASE_TYPE installCodeIndex, 
                                                portuBASE_TYPE value,
                                                portuBASE_TYPE status)
{
    DriveMsgUnit  *pdriveMsgUnit                                = pdriveMsgQueue->m_queue;
    
    if (pdriveMsgQueue->m_writeIndex < MSG_QUEUE_DEPTH)
    {
        pdriveMsgUnit                                          += pdriveMsgQueue->m_writeIndex;
        pdriveMsgUnit->m_installCodeIndex                       = installCodeIndex;
        pdriveMsgUnit->m_value                                  = value;
        pdriveMsgUnit->m_status                                 = status;
        pdriveMsgQueue->m_writeIndex++;

        return TRUE;
    }
    //消息添加失败
    return FALSE;
}

//读取时固定从t_PanelDriveMsgSnapshot读取 并返回是否还有消息可读  有消息可读 返回TRUE 
//无消息可读 返回FALSE
static portuBASE_TYPE _int_API_RSDriveMsgUnitGet(PanelNoticeMsg *ppanelNoticeMsg, 
                                                 DriveMsgQueue *pdriveMsgQueue,
                                                 uint8 *installCodeIndex)
{
    DriveMsgUnit  *pdriveMsgUnit                                = pdriveMsgQueue->m_queue;
    
    pdriveMsgUnit                                              += pdriveMsgQueue->m_readIndex;
    ppanelNoticeMsg->m_value                                    = pdriveMsgUnit->m_value;
    ppanelNoticeMsg->m_status                                   = pdriveMsgUnit->m_status;
    //返回消息安装码索引
    *installCodeIndex                                           = (pdriveMsgUnit->m_installCodeIndex);
    pdriveMsgQueue->m_readIndex++;
    if (pdriveMsgQueue->m_readIndex == pdriveMsgQueue->m_writeIndex)
    {
        return FALSE;
    }
    return TRUE;
}

/*
static portuBASE_TYPE _API_RSDriveMsgNullChk(DriveMsgQueue *pdriveMsgQueue)
{
    portuBASE_TYPE  rt                                          = FALSE;
    portCPSR_TYPE   level                                       = portDISABLE_INTERRUPTS();

    if (pdriveMsgQueue->m_readIndex == pdriveMsgQueue->m_writeIndex)
    {
        rt                                                      = TRUE;
    }
    portENABLE_INTERRUPTS(level);
    return rt;
}
*/

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 填加一条驱动消息                                                              
 *                                                                           
 *  入口参数 : msgHandleStyle：驱动消息类型 分为远控、虚拟、真实三中类型
 *			   msgInstallCode: 消息安装码   用于判断面板是否安装了此消息
 *             msgID         : 消息ID       消息值                                                  
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 : 添加消息时 需要临界区的保护                                                               
 *             							                                                              
 *                                                                            
******************************************************************************/ 

static portuBASE_TYPE _int_API_SchedDriveMsgAdd(PanelDriveMsg *ptpanelDriveMsg, portuBASE_TYPE msg, 
								                portuBASE_TYPE installCodeIndex, 
                                                portuBASE_TYPE value,
                                                portuBASE_TYPE status)
{
	portuBASE_TYPE     rt                                       = FALSE;
    DriveMsgQueue     *pdriveMsgQueue                           = &ptpanelDriveMsg->m_keyQueue;
    
    //看是否为有效信息   若不是则退出
	if (NONE_DRIVE_MSG_ID_INDEX != installCodeIndex)
    {
		//驱动消息类型
		ptpanelDriveMsg->m_msg                                 |= msg; 
		if (INSTALL_MSG_KEY == msg)
		{
		    pdriveMsgQueue                                      = &ptpanelDriveMsg->m_keyQueue;	
		}
		else if (INSTALL_MSG_SCHED == msg)
		{
		    pdriveMsgQueue                                      = &ptpanelDriveMsg->m_schedQueue;
		}
        else if (INSTALL_MSG_COMM == msg)
        {
            //
        }
        rt = _int_API_RSDriveMsgUnitPut(pdriveMsgQueue, installCodeIndex, value, status);
    }

    return rt;
}

static portuBASE_TYPE _API_SchedDriveMsgAdd(PanelDriveMsg *ptpanelDriveMsg, portuBASE_TYPE msg, 
								            portuBASE_TYPE installCodeIndex, 
                                            portuBASE_TYPE value,
                                            portuBASE_TYPE status)
{
    portuBASE_TYPE  rt                                          = FALSE;
    portCPSR_TYPE   level                                       = portDISABLE_INTERRUPTS();

	rt = _int_API_SchedDriveMsgAdd(ptpanelDriveMsg, msg, installCodeIndex, value, status);
	portENABLE_INTERRUPTS(level);

    return rt;
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 填加一条驱动消息                                                              
 *                                                                           
 *  入口参数 : msgHandleStyle：驱动消息类型 分为远控、虚拟、真实三中类型
 *			   msgInstallCodeIndex: 消息安装码   用于判断面板是否安装了此消息
 *             msgID         : 消息ID       消息值                                                  
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 : 添加消息时 不需要临界区的保护                                                               
 *             							                                                              
 *                                                                            
******************************************************************************/ 

portuBASE_TYPE API_SchedDriveMsgAdd(portuBASE_TYPE msgHandleStyle, portuBASE_TYPE msgInstallCodeIndex, portuBASE_TYPE msg,
                                    portuBASE_TYPE msgStatus)
{
	return _API_SchedDriveMsgAdd((PanelDriveMsg *)&t_PanelDriveMsg, msgHandleStyle, msgInstallCodeIndex, msg, msgStatus);
}

portuBASE_TYPE API_SchedInnerDriveMsgAdd(portuBASE_TYPE msgHandleStyle, portuBASE_TYPE msgInstallCodeIndex, portuBASE_TYPE msg,
                                     portuBASE_TYPE msgStatus)
{
	return _API_SchedDriveMsgAdd((PanelDriveMsg *)&t_PanelDriveMsgSnapshot, msgHandleStyle, msgInstallCodeIndex, msg, msgStatus);
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 安装面板安装服务函数ID号(既所支持响应的消息ID）ID值参照AppConfig.h                                                              
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 :                                                                
 *                                                                            
 *                                                                            
******************************************************************************/

void API_SchedPanelMsgIDInstall(portMSGID_TYPE panelMsgID)
{
    s_PanelInstallServMsgID                                    |= panelMsgID;    
}

void API_SchedPanelMsgIDUninstall(portMSGID_TYPE panelMsgID)
{
    s_PanelInstallServMsgID                                    &= (~panelMsgID);    
}

void API_SchedPanelMsgIDClr(void)
{
    s_PanelInstallServMsgID                                     = 0;   
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 清空面板安装服务函数ID号(既所支持响应的消息ID）ID值参照AppConfig.h                                                              
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 :                                                                
 *                                                                            
 *                                                                            
******************************************************************************/

static void _API_SchedEnvironmentInit(void)
{
//     #if (COMPILE_ENV == AVR_GCC)
//     {
//         s_PanelInstallServMsgID                                 = PGM_STRUCT_MEMBER_WORD_READ(s_ptCurrentPanelUnit, PanelUnit, m_panelInstallServMsgID);
//         s_pfPanelCallBackFunc                                   = (volatile Fp_pfPanelCallBackFunc *)(PGM_STRUCT_MEMBER_WORD_READ(s_ptCurrentPanelUnit, PanelUnit, m_pfPanelCallBackFunc));
//     }
//     #elif (COMPILE_ENV == ARM_KEIL)
//     {
        s_PanelInstallServMsgID                                 = s_ptCurrentPanelUnit->m_panelInstallServMsgID;                
        s_pfPanelCallBackFunc                                   = s_ptCurrentPanelUnit->m_pfPanelCallBackFunc;
//     }
//     #endif
 }

static void _API_SchedPanelSwitchExecCore(PanelNoticeMsg *ptpanelNoticeMsg)
{
    portuBASE_TYPE  rt                                          = TRUE;
    
    //更新消息句柄为初始化面板消息
    ptpanelNoticeMsg->m_msg                                     = MSG_CLOSE;
    //对当前面板发送面板关闭消息
    rt  = _API_SchedSendMessage((const PanelNoticeMsg *)ptpanelNoticeMsg);
    if (TRUE == rt)
    {
        s_ptPrevPanelUnit                                       = s_ptCurrentPanelUnit;
        s_ptCurrentPanelUnit                                    = s_ptCurrentPanelUnitTmp;
        //初始化此面板调度环境
	    _API_SchedEnvironmentInit();
        ptpanelNoticeMsg->m_msg                                 = MSG_INITIALIZING;
        _API_SchedSendMessage((const PanelNoticeMsg *)ptpanelNoticeMsg);
        //面板绘制
        _API_SchedPanelDraw(s_ptCurrentPanelUnit->m_pwindow, PANEL_DRAW_FLG_NORMAL);
        ptpanelNoticeMsg->m_msg                                 = MSG_INIT_OVER;
        _API_SchedSendMessage((const PanelNoticeMsg *)ptpanelNoticeMsg);
    }
}

static portuBASE_TYPE _API_SchedPanelSwitchExec(PanelNoticeMsg *ptpanelNoticeMsg)
{
    portuBASE_TYPE  rt                                          = FALSE;
    
    //面板回调函数执行成功后 判断是否产生新面板初始化消息
    if (s_ptCurrentPanelUnit != s_ptCurrentPanelUnitTmp)
    {
        _API_SchedPanelSwitchExecCore(ptpanelNoticeMsg); 
        rt                                                      = TRUE;       
    }

    return rt;
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 :                                                               
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 :                                                                
 *                                                                            
 *                                                                            
******************************************************************************/

static INLINE void _int_API_SchedDriveMsgCpyAndClr(void)
{
    //清除驱动消息
	memcpy((void *)&t_PanelDriveMsgSnapshot, (const void *)&t_PanelDriveMsg, sizeof(PanelDriveMsg));
	memset((void *)&t_PanelDriveMsg, 0, sizeof(PanelDriveMsg)); 
}

// static portuBASE_TYPE _API_SchedSendMessage(const PanelNoticeMsg *pmessage)
// {
//     portuBASE_TYPE   rt 	        		                    = TRUE;
//     
//     rt                                                          = s_pfPanelCallBackFunc((const PanelNoticeMsg *)pmessage);
//     #if (DEBUG_VERSION > 0)
//     {
//         t_AppDebugWatch.m_uint8_1                               = rt;
//     }
//     #endif
//     
//     return rt;
// }

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 :                                                               
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 :                                                                
 *                                                                            
 *                                                                            
******************************************************************************/

void API_ShcedPanelDisplay(const PanelUnit *ptpanel)
{
    s_ptCurrentPanelUnitTmp                                     = (PanelUnit *)ptpanel;
}
void API_ShcedPanelDisplayExec(const PanelUnit *ptpanel)
{
    PanelNoticeMsg  t_panelNoticeMsg         	                = {0};      
    
    API_ShcedPanelDisplay(ptpanel);
    _API_SchedPanelSwitchExecCore(&t_panelNoticeMsg);
}

const PanelUnit * API_ShcedCurrentPanelGet(void)
{
    return (const PanelUnit *)s_ptCurrentPanelUnit;
}

const PanelUnit * API_ShcedPrevPanelGet(void)
{
    return (const PanelUnit *)s_ptPrevPanelUnit;
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 面板安装消息检测                                                         
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 :                                                                
 *                                                               
 *                                                                            
******************************************************************************/

portuBASE_TYPE API_RSPanelInstallKeyChk(portuBASE_TYPE msgMaskCodeIndex)
{
    portuBASE_TYPE  rt 	        		                        = FALSE;
	portMSGID_TYPE  msgInstallMaskCode	                        = (portMSGID_TYPE)((portMSGID_TYPE)1 << msgMaskCodeIndex);
	
    //通过与运算来判断是否安装了此驱动消息
    if (msgInstallMaskCode == (s_PanelInstallServMsgID & msgInstallMaskCode))
    {
        rt                    			                        = PANEL_INSTALL_MSG;    
    }
	#if (SCHED_COMMON_MSG_SUPPORT > 0)
	{
		if (msgInstallMaskCode == (s_SchedCommPanelMsgID & msgInstallMaskCode))
		{
			rt                    	                           |= SCHED_INSTALL_MSG;     
		}
	}
	#endif
    
    return rt;
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 :                                                               
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 : 为了简化省略了对调度器回调函数是否存在的判断                                                               
 *                                                                            
 *                                                                            
******************************************************************************/

void API_SchedExecute(void)
{
    volatile PanelNoticeMsg     t_panelNoticeMsg         	    = {0};                   //定义面板通知消息结构体
	volatile portuBASE_TYPE     drivemsgInstallCodeIndex        = 0;                     //驱动消息安装码
    volatile portuBASE_TYPE     rt								= TRUE;
	volatile portuBASE_TYPE     schedMsgHandleCircle            = INSTALL_SCHED_INIT;    //调度器循环处理消息标志
	volatile portuBASE_TYPE     driveMsgInstallResult           = 0;					 //驱动消息安装判断结果
	volatile portCPSR_TYPE      level;
    Fp_pfSchedCBF              *pfschedCBF;
    Fp_pfTestSchedCBF          *pftestSchedCBF;
    
	switch (s_SchedulerRunStatus)
    {
        //调度器初始化状态  不接收输入的消息
        case SCHEDULER_INIT:
		
			t_panelNoticeMsg.m_msg                              = MSG_INITIALIZING;      //更新消息句柄为初始化面板消息
			schedMsgHandleCircle                                = INSTALL_SCHED_INIT;    //调度器初始化信息
			//初始化此面板调度环境
			_API_SchedEnvironmentInit();
            //调用此面板的回调函数 并发送消息
            rt                                                  = _API_SchedSendMessage((const PanelNoticeMsg *)&t_panelNoticeMsg);
            //面板显示
            _API_SchedPanelDraw(s_ptCurrentPanelUnit->m_pwindow, PANEL_DRAW_FLG_NORMAL);
            //更新消息句柄为初始化结束面板消息
            t_panelNoticeMsg.m_msg                              =  MSG_INIT_OVER;
            rt                                                  = _API_SchedSendMessage((const PanelNoticeMsg *)&t_panelNoticeMsg);
			#if (DEBUG_VERSION > 0)
            {
                t_AppDebugWatch.m_uint8_1                       = rt;
            }
            #endif
            if (TRUE == rt)
			{
				//系统进入运行态
                s_SchedulerRunStatus                            = SCHEDULER_RUNNING; 
                do {

                    //面板回调函数执行成功后 判断是否产生新面板初始化消息
                    rt  = _API_SchedPanelSwitchExec((PanelNoticeMsg *)&t_panelNoticeMsg);
                }while (TRUE == rt);
			}
			else
			{
				//系统进入错误态
                s_SchedulerRunStatus                            = SCHEDULER_ERROR;
			}
            break;
        
		//运行态
        case SCHEDULER_RUNNING: 
		
			//判断是否安装了调度器驱动消息采集函数
			if (NULL != s_pfSchedlerDriveMsgSampleCallBackFunc)
			{
				(*s_pfSchedlerDriveMsgSampleCallBackFunc)();						    //调度器驱动消息采集回调函数
			}
			//共享资源拍照
            level                                               = portDISABLE_INTERRUPTS();        
            _int_API_SchedDriveMsgCpyAndClr();
            portENABLE_INTERRUPTS(level);
			
			//初始化调度器运行信息为调度器运行正常
            schedMsgHandleCircle                                = INSTALL_SCHED_WELL; 
			
			//调度器接收到驱动消息
            if ((MSG_NONE != t_PanelDriveMsgSnapshot.m_msg) && (NULL != s_pfPanelCallBackFunc))
            {
                do{
					//按键类消息
                    if (t_PanelDriveMsgSnapshot.m_msg & INSTALL_MSG_KEY)
                    {
                        t_panelNoticeMsg.m_msg                  = MSG_KEY;
                        level   = portDISABLE_INTERRUPTS();
                        //判断消息队列已空 若已空 则清除真实键消息
                        if (FALSE == _int_API_RSDriveMsgUnitGet((PanelNoticeMsg *)&t_panelNoticeMsg, 
                                                                (DriveMsgQueue *)&t_PanelDriveMsgSnapshot.m_keyQueue, 
                                                                (uint8 *)&drivemsgInstallCodeIndex))
                        {
                            t_PanelDriveMsgSnapshot.m_msg        &= ~INSTALL_MSG_KEY; 
                        }
                        portENABLE_INTERRUPTS(level);
                    }
					//调度器类消息
                    else if (t_PanelDriveMsgSnapshot.m_msg & INSTALL_MSG_SCHED)
					{
                        t_panelNoticeMsg.m_msg                  = MSG_SCHED;
                        level   = portDISABLE_INTERRUPTS();
                        //判断消息队列已空 若已空 则清除调度器消息
                        if (FALSE == _int_API_RSDriveMsgUnitGet((PanelNoticeMsg *)&t_panelNoticeMsg, 
                                                                (DriveMsgQueue *)&t_PanelDriveMsgSnapshot.m_schedQueue, 
                                                                (uint8 *)&drivemsgInstallCodeIndex))
                        {
                            t_PanelDriveMsgSnapshot.m_msg        &= ~INSTALL_MSG_SCHED; 
                        }
                        portENABLE_INTERRUPTS(level);
                    }
                    //通讯类消息
                    else if (t_PanelDriveMsgSnapshot.m_msg & INSTALL_MSG_COMM)
					{
                        t_panelNoticeMsg.m_msg                  = MSG_COMM;
                        level   = portDISABLE_INTERRUPTS();
                        //判断消息队列已空 若已空 则清除调度器消息
                        if (FALSE == _int_API_RSDriveMsgUnitGet((PanelNoticeMsg *)&t_panelNoticeMsg, 
                                                                (DriveMsgQueue *)&t_PanelDriveMsgSnapshot.m_commQueue, 
                                                                (uint8 *)&drivemsgInstallCodeIndex))
                        {
                            t_PanelDriveMsgSnapshot.m_msg        &= ~INSTALL_MSG_COMM; 
                        }
                        portENABLE_INTERRUPTS(level);
                    }
                    
					//获取此面板是否安装了此类消息
                    driveMsgInstallResult                       = API_RSPanelInstallKeyChk(drivemsgInstallCodeIndex);
                    //只有在安装此按键后 才能进行以下操作
                    if (FALSE != driveMsgInstallResult)     
                    {
                        //判断此消息ID是否为所有面板都有共同响应行为(调度器面板共同驱动消息) 如真实键消息中的EXIT键
                        if (SCHED_INSTALL_MSG == (driveMsgInstallResult & SCHED_INSTALL_MSG))
                        {
                            if (NULL != s_pfPanelCallBackFunc)
							{
								if (TRUE == _API_SchedSendMessage((const PanelNoticeMsg *)&t_panelNoticeMsg))
								{
									//调度器消息类型面板类共同消息
									st_SchedMsg.m_msg            = SCHED_MSG_COMMON;
									//获取面板共同消息存放地址
									st_SchedMsg.m_pMsgAccessAddr = (void *)&t_panelNoticeMsg;
									//运行调度器回调函数 本质上也是响应按键
									rt = (*s_PfSchedCBF)((const SchedMsg *)&st_SchedMsg);
								}
							} 
                        }
                        //面板各自按键服务
                        if (PANEL_INSTALL_MSG == (driveMsgInstallResult & PANEL_INSTALL_MSG))
                        {	
							//调用此面板的回调函数 并发送消息 获取回调函数返回码
                            if (NULL != s_pfPanelCallBackFunc)
                            {
                                rt                              = _API_SchedSendMessage((const PanelNoticeMsg *)&t_panelNoticeMsg);         
                            }
                        }
                        //面板回调函数执行成功后 判断是否产生新面板初始化消息
                        if (TRUE == rt)
                        {
							_API_SchedPanelSwitchExec((PanelNoticeMsg *)&t_panelNoticeMsg);
						}
                        //如果在面板消息处理发生错误 退出循环 
						else
                        {
                            //获取调度器错误代码
                            //清除调度器运行正常
                            schedMsgHandleCircle               &= ~INSTALL_SCHED_WELL;
                            //置位调度器运行失败
                            schedMsgHandleCircle               |= INSTALL_SCHED_ERROR;  
                            //系统进入错误态
                            s_SchedulerRunStatus                = SCHEDULER_ERROR;
                            break;                                                        
                        }
                    }
				//循环处理完面板所有消息 	
                }while (NONE_MSG_HANDLE != t_PanelDriveMsgSnapshot.m_msg);               
			}

            API_PEMExecute();
			break;
        
        //错误态
        case SCHEDULER_ERROR:
            
            break;
		
		default:
            break;
	}
    
    //----------------------------------------------------------------------------------------------------------------
    //处理调度器回调函数信息      
    //共享资源拍照
    level                                                       = portDISABLE_INTERRUPTS();        
    pfschedCBF                                                  = s_PfSchedCBF;
    pftestSchedCBF                                              = s_PfTestSchedCBF;
    portENABLE_INTERRUPTS(level);
    if (NULL != pftestSchedCBF)
    {
        pftestSchedCBF();
    }
    if (NULL != pfschedCBF)
    {
        //调度器服务函数 可实现对调度器产生的多个信息多次调用
        do{
            if (schedMsgHandleCircle & INSTALL_SCHED_WELL)
            {
                schedMsgHandleCircle                           &= ~INSTALL_SCHED_WELL;
                st_SchedMsg.m_msg                               = SCHED_MSG_RUN_WELL;
                st_SchedMsg.m_pMsgAccessAddr                    = (void *)&t_panelNoticeMsg; 
            }
            else if (schedMsgHandleCircle & INSTALL_SCHED_ERROR)
            {
                schedMsgHandleCircle                           &= ~INSTALL_SCHED_ERROR;
                st_SchedMsg.m_msg                               = SCHED_MSG_RUN_ERROR;
                st_SchedMsg.m_pMsgAccessAddr                    = (void *)&rt;
            }
            else if (schedMsgHandleCircle & INSTALL_SCHED_INIT)
            {
                schedMsgHandleCircle                           &= ~INSTALL_SCHED_INIT;
                st_SchedMsg.m_msg                               = SCHED_MSG_INIT;
            }
            //运行调度器回调函数 发送调度器信息
            (*pfschedCBF)((const SchedMsg *)&st_SchedMsg);
            
        }while (((portuBASE_TYPE)0) != schedMsgHandleCircle);
    }
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 安装调度器面板共同消息 此面板消息为共同的面板所共有                                                                
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 : 调度器面板共同消息为所有面板共有的驱动消息 由调度器的回调函数来处理                                                               
 *                                                                            
 *                                                                            
******************************************************************************/
void API_SchedCommPanelMsgIDInstall(const portMSGID_TYPE schedProcPanelMsgID)
{
    s_SchedCommPanelMsgID 					                   |= schedProcPanelMsgID;    
}

void API_SchedCommPanelMsgIDUninstall(const portMSGID_TYPE schedProcPanelMsgID)
{
	s_SchedCommPanelMsgID 					                   &= (~schedProcPanelMsgID);    
}

void API_SchedCommPanelMsgIDClr(void)
{
    s_SchedCommPanelMsgID 					                    = 0;   
}                             

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 安装功能测试回调函数                                                              
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 : 在测试过程中 循环不断的执行此测试回调函数 完成系统测试功能                                                               
 *                                                                            
 *                                                                            
******************************************************************************/

void API_SchedTestSchedCBFInstall(Fp_pfTestSchedCBF *pfTestSchedCBF)
{
	s_PfTestSchedCBF                                            = pfTestSchedCBF;
}

void API_SchedTestSchedCBFUninstall(void)
{
	s_PfTestSchedCBF                                            = NULL;
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 调度器初始化                                                              
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 :                                                                
 *                                                                            
 *                                                                            
******************************************************************************/
void API_SchedExecuteInit(const PanelUnit *ptPanel, 
							const portMSGID_TYPE schedProcPanelMsgID,
							Fp_pfSchedlerDriveMsgSampleCallBackFunc *pfschedlerDriveMsgSampleCallBackFunc, 
							Fp_pfSchedCBF *pfschedCBF, 
							Fp_pfTestSchedCBF *pfTestSchedCBF)
{
	s_ptCurrentPanelUnit           			                    = (PanelUnit *)ptPanel;	//当前面板
	s_ptCurrentPanelUnitTmp        			                    = (PanelUnit *)ptPanel;	//当前面板备份
    s_ptPrevPanelUnit              			                    = (PanelUnit *)ptPanel;	//前一面板
    s_SchedCommPanelMsgID                                       = schedProcPanelMsgID;	//调度器面板共同消息
    s_PfTestSchedCBF                                            = pfTestSchedCBF;
	s_pfSchedlerDriveMsgSampleCallBackFunc                      = pfschedlerDriveMsgSampleCallBackFunc;
	s_PfSchedCBF                                                = pfschedCBF;
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 根据面板的资源表来绘制整幅面板                                                             
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 :                                                             
 *                                                                        
 *                                                                            
******************************************************************************/
// static portuBASE_TYPE _API_SchedPanelDraw(PanelWindow *ptpanelWindow, portuBASE_TYPE drawInfo)
// {
//     if (NULL != ptpanelWindow)
//     {
//         //
//     }

//     return TRUE;
// }
/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/
