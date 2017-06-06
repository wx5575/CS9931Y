#ifndef    _API_SCHED_H
#define    _API_SCHED_H

#if defined(__cplusplus)

    extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/******************************************************************************
 *                             包含文件声明
******************************************************************************/ 



/******************************************************************************
 *                           文件接口信息宏定义
******************************************************************************/ 

#define	portuBASE_TYPE	uint32_t
#define	uint8	uint8_t			
			
#define           NO_SHECD_PROC_PANEL_MSG_ID                0x70000000
typedef		      unsigned long long int					portMSGID_TYPE;

#define           NONE_MSG_HANDLE               	        (0x00)
//消息队列深度
#define           MSG_QUEUE_DEPTH                           (5)
typedef           portuBASE_TYPE                            portMSG_TYPE;

#define           MSG_ID_GET(msgIdIndex)                    ((portMSGID_TYPE)1UL << msgIdIndex)

/*******************************************************************************
 *                           面板绘制标志宏定义
********************************************************************************/

#define           PANEL_DRAW_FLG_NORMAL                     (1)

/*******************************************************************************
 *                            消息安装类型定义
********************************************************************************/

#define  		  INSTALL_MSG_KEY                           (2)
#define  		  INSTALL_MSG_SCHED                         (4)
#define  		  INSTALL_MSG_COMM                          (8)
#define 		  INSTALL_MSG_PUSHBUTTON					(16)
/*******************************************************************************
 *              消息类型定义 --- 每一种消息类型都对应着一种消息ID
********************************************************************************/

#define           MSG_STATUS_NONE                           (0)
#define           MSG_STATUS_PRESS                          (1)
#define           MSG_STATUS_RELEASE                        (2)

#define  	      MSG_NONE                                  (0)
#define  		  MSG_INITIALIZING                          (MSG_NONE+1)
#define  		  MSG_INIT_OVER                             (MSG_INITIALIZING+1)
#define  		  MSG_CLOSE				                    (MSG_INIT_OVER+1)

#define  		  MSG_PREV_PAGE                             (MSG_CLOSE+1)
#define  		  MSG_NEXT_PAGE                             (MSG_PREV_PAGE+1)
#define  		  MSG_PREV_PAGE_INIT                        (MSG_NEXT_PAGE+1)
#define  		  MSG_NEXT_PAGE_INIT                        (MSG_PREV_PAGE_INIT+1)

#define  		  MSG_KEY                                   (MSG_NEXT_PAGE_INIT+1)
#define  		  MSG_SCHED                                 (MSG_KEY+1)
#define  		  MSG_COMM                                  (MSG_SCHED+1)
//#define  		  MSG_COMM                                  (MSG_PUSHBUTTON+1)

#define			  MSG_PUSHBUTTON							(MSG_COMM+1)
/*******************************************************************************
 *                            系统预定义消息ID宏定义
********************************************************************************/

#define  		  INVALID_DRIVE_MSG_ID_INDEX                (62)
#define  		  NONE_DRIVE_MSG_ID_INDEX                   (63)
#define  		  MSG_KEY_ID_INDEX                          (0) 
#define  		  MSG_SCHED_ID_INDEX                        (MSG_KEY_ID_INDEX+1) 
#define  		  MSG_COMM_ID_INDEX                         (MSG_SCHED_ID_INDEX+1)

//项目的需求的自定义消息在 MSG_USER_ID_INDEX 之后添加
#define           MSG_USER_ID_INDEX                         (MSG_COMM_ID_INDEX+1)


#define  		  NONE_DRIVE_MSG_ID                         MSG_ID_GET(NONE_DRIVE_MSG_ID_INDEX) 
#define  		  MSG_KEY_ID                                MSG_ID_GET(MSG_KEY_ID_INDEX) 
#define  		  MSG_SCHED_ID                              MSG_ID_GET(MSG_SCHED_ID_INDEX) 
#define  		  MSG_COMM_ID                               MSG_ID_GET(MSG_COMM_ID_INDEX)
#define           MSG_USER_ID                               MSG_ID_GET(MSG_USER_ID_INDEX) 


#define           NONE_DRIVE_MSG                            (0xff)

/*******************************************************************************
 *                              调度器信息宏定义 
********************************************************************************/

#define  		  INSTALL_SCHED_INIT                        (0x01)
#define  		  INSTALL_SCHED_WELL                        (0x02)
#define  		  INSTALL_SCHED_ERROR                       (0x04)
#define  		  INSTALL_SCHED_PANEL                       (0x08)

#define  		  SCHED_MSG_RUN_WELL                        (0x01)
#define  		  SCHED_MSG_RUN_ERROR                       (0x02)
#define  		  SCHED_MSG_COMMON                          (0x03)                  //面板类共同消息类型
#define  		  SCHED_MSG_COMMU                           (0x04)                  //通讯消息类型
#define  		  SCHED_MSG_INIT                            (0x05)                  //初始化信息

/*******************************************************************************
 *                              调度器状态宏定义 
********************************************************************************/

#define  		  SCHEDULER_INIT                            (1)
#define  		  SCHEDULER_RUNNING                         (2)
#define  		  SCHEDULER_ERROR                           (3)

/******************************************************************************
 *                         文件接口数据结构体声明
******************************************************************************/ 

/******************************************************************************
 *                           驱动消息结构体声明
******************************************************************************/ 

typedef struct
{
    uint8                                                       m_installCodeIndex;
    uint8                                                       m_value;
    //消息状态 按下 释放
    uint8                                                       m_status;
                                                        
}DriveMsgUnit;

typedef struct
{
    DriveMsgUnit                                                m_queue[MSG_QUEUE_DEPTH];
    uint8                                                       m_writeIndex;
    uint8                                                       m_readIndex;

}DriveMsgQueue;


typedef struct _PANEL_DRIVE_MSG_
{
    portMSG_TYPE                                                m_msg;                                            		
    DriveMsgQueue                                               m_keyQueue;
    DriveMsgQueue                                               m_schedQueue;
    DriveMsgQueue                                               m_commQueue;

	DriveMsgQueue												m_pushbuttonQueue;

}PanelDriveMsg;

extern volatile PanelDriveMsg t_PanelDriveMsg;


typedef struct _PANEL_NOTICE_MSG_
{
	portMSG_TYPE                                                m_msg;                                            	
    
	uint8                                                       m_value;                          		                      
    uint8                                                       m_status;                                               
    
}PanelNoticeMsg;

typedef struct _SCHED_MSG_
{
    portMSG_TYPE                                                m_msg; 
    //调度器消息存取地址指针                  
    void                                                       *m_pMsgAccessAddr;                                         		

}SchedMsg;

/******************************************************************************
 *                           回调函数指针声明
******************************************************************************/ 

typedef portuBASE_TYPE  (Fp_pfPanelCallBackFunc)(const PanelNoticeMsg *ptpanelNoticeMsg);    //面板回调函数指针
typedef portuBASE_TYPE  (Fp_pfTestSchedCBF)(void); 							                 //测试回调函数指针
typedef portuBASE_TYPE  (Fp_pfSchedlerDriveMsgSampleCallBackFunc)(void); 					 //调度器驱动消息采集回调函数指针
typedef portuBASE_TYPE  (Fp_pfSchedCBF)(const SchedMsg *ptschedMsg);                         //调度器回调函数指针

/******************************************************************************
 *                           面板单元结构体声明
******************************************************************************/ 

typedef struct _PANEL_UNIT_
{
    uint8              				   				            m_panelID;                   //面板ID号 
    Fp_pfPanelCallBackFunc 		 	  			               *m_pfPanelCallBackFunc;       //面板回调函数指针拷贝值
	portMSGID_TYPE                       			            m_panelInstallServMsgID;     //面板安装服务函数类型拷贝值
    struct _PANEL_WINDOW_                                      *m_pwindow;                   //面板窗口指针    

}PanelUnit;

typedef struct _PANEL_WINDOW_
{
    void                                                       *m_void;

}PanelWindow;

/******************************************************************************
 *                            文件接口函数声明
******************************************************************************/ 

extern portuBASE_TYPE API_SchedDriveMsgAdd(portuBASE_TYPE msgHandleStyle, portuBASE_TYPE msgInstallCodeIndex, portuBASE_TYPE msg,
                                           portuBASE_TYPE msgStatus);

extern void API_SchedExecute(void);

extern void API_SchedPanelMsgIDInstall(portMSGID_TYPE panelMsgID);

extern void API_SchedPanelMsgIDUninstall(portMSGID_TYPE panelMsgID);

extern void API_SchedPanelMsgIDClr(void);

extern void API_SchedExecuteInit(const PanelUnit *ptPanel,                
									const portMSGID_TYPE schedProcPanelMsgID,
									Fp_pfSchedlerDriveMsgSampleCallBackFunc *pfschedlerDriveMsgSampleCallBackFunc, 
									Fp_pfSchedCBF *pfschedCallBackFunc, 
									Fp_pfTestSchedCBF *pfTestSchedCBF);

extern void API_SchedCommPanelMsgIDInstall(const portMSGID_TYPE schedProcPanelMsgID);

extern void API_SchedCommPanelMsgIDUninstall(const portMSGID_TYPE schedProcPanelMsgID);

extern void API_SchedCommPanelMsgIDClr(void);

extern void API_SchedTestSchedCBFInstall(Fp_pfTestSchedCBF *pfTestSchedCBF);

extern void API_SchedTestSchedCBFUninstall(void);

extern void API_ShcedPanelDisplay(const PanelUnit *ptpanel);


/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/

#if defined(__cplusplus)

    }
#endif 

#endif
