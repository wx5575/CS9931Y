
#ifndef __UART_H
#define __UART_H 
#ifndef    _STDINT_H_
#include "stdint.h"
#endif 
#define IER_RBR		0x01
#define IER_THRE	0x02
#define IER_RLS		0x04

#define IIR_PEND	0x01
#define IIR_RLS		0x03
#define IIR_RDA		0x02
#define IIR_CTI		0x06
#define IIR_THRE	0x01

#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80

// #define BUFSIZE		0x40

#define    TRUE                                             (1)

void UART0_Init (void);
int UART0_SendByte (int ucData);
int UART0_GetChar (void);
void UART0_SendString (unsigned char *s);
void UART2_Init (void);
int UART2_SendByte (int ucData);
int UART2_GetChar (void);
void UART2_SendString (unsigned char *s);
void UART0_SendChar(uint16_t disp);
void UART2_SendChar(uint16_t disp);
// void UARTSend( uint32_t portNum, uint8_t *BufferPtr, uint32_t Length );
int Send_check(void);














/******************************************************************************
 *                            文件接口函数声明
******************************************************************************/ 

// extern uint32_t API_SchedDriveMsgAdd(uint32_t msgHandleStyle, uint32_t msgInstallCodeIndex, uint32_t msg,
//                                            uint32_t msgStatus);

// extern void API_SchedExecute(void);

// extern void API_SchedPanelMsgIDInstall(portMSGID_TYPE panelMsgID);

// extern void API_SchedPanelMsgIDUninstall(portMSGID_TYPE panelMsgID);

// extern void API_SchedPanelMsgIDClr(void);

// extern void API_SchedExecuteInit(const PanelUnit *ptPanel,                
// 									const portMSGID_TYPE schedProcPanelMsgID,
// 									Fp_pfSchedlerDriveMsgSampleCallBackFunc *pfschedlerDriveMsgSampleCallBackFunc, 
// 									Fp_pfSchedCBF *pfschedCallBackFunc, 
// 									Fp_pfTestSchedCBF *pfTestSchedCBF);

// extern void API_SchedCommPanelMsgIDInstall(const portMSGID_TYPE schedProcPanelMsgID);

// extern void API_SchedCommPanelMsgIDUninstall(const portMSGID_TYPE schedProcPanelMsgID);

// extern void API_SchedCommPanelMsgIDClr(void);

// extern void API_SchedTestSchedCBFInstall(Fp_pfTestSchedCBF *pfTestSchedCBF);

// extern void API_SchedTestSchedCBFUninstall(void);

// extern void API_ShcedPanelDisplay(const PanelUnit *ptpanel);





#define             MAX_MODEL_NUMB              (13)    // 型号个数

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
// EnvParam t_EnvParam;
// InstrumentAttrConfig t_InstrumentAttrConfig = {
//     
//     {"CS5600", "CS5601", "CS5602", "CS5603", "CS2676AN", "CS2676DN", "CS2676EN", "CS2676N", "CS5050", "CS5051", "CS5052", "CS5053", "CS5101"},
//     "xxxxxxxxxx",

//     "2.0.00"
// }; 





#endif


