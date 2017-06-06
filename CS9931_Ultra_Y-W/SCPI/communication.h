#ifndef    _COMMUNICATION_H_
#define    _COMMUNICATION_H_
typedef enum
{
    TEST_STATE_TESTING  = '0',
    TEST_STATE_WAITING  = '1',

}TestStatus; 

typedef struct _PARAM_
{
    uint32_t                            m_val;         //值
	uint8_t                             m_range;       //档位

}Param;
/******************************************************************************
 *                            文件接口函数声明
******************************************************************************/

extern void APP_CommProtocolLogicInit(uint32_t localAddr);

extern uint32_t InstructionExec_RST(int argc, const uint8_t *argv[]);

extern uint32_t InstructionExec_IDN(int argc, const uint8_t *argv[]);

extern uint32_t InstructionExec_FileRead(int argc, const uint8_t *argv[]);

extern uint32_t InstructionExec_FileCatalogSingle(int argc, const uint8_t *argv[]);
// 通讯指令集执行函数

extern uint32_t InstructionExec_CommSAddr(int argc, const uint8_t *argv[]);

extern uint32_t InstructionExec_CommRemote(int argc, const uint8_t *argv[]);

extern uint32_t InstructionExec_CommLocal(int argc, const uint8_t *argv[]);

extern uint32_t InstructionExec_CommControl(int argc, const uint8_t *argv[]);

// 源指令执行函数

extern uint32_t InstructionExec_SrcTestStart(int argc, const uint8_t *argv[]);

extern uint32_t InstructionExec_SrcTestStop(int argc, const uint8_t *argv[]);

extern uint32_t InstructionExec_SrcTestStatus(int argc, const uint8_t *argv[]);

extern uint32_t InstructionExec_SrcTestFetch(int argc, const uint8_t *argv[]);

extern uint32_t InstructionExec_SrcTestMode(int argc, const uint8_t *argv[]);

extern uint32_t InstructionExec_SourceListFIndex(int argc, const uint8_t *argv[]);

extern uint32_t InstructionExec_SourceListMode(int argc, const uint8_t *argv[]);

// STEP指令执行函数
//*********************************DCW STEP指令**********************************//
uint32_t InstructionExec_StepMode(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepDcwTtime(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepDcwRtime(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepDcwItime(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepDcwVolt(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepDcwCurHigh(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepDcwCurLow(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepDcwArc(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepDcwRange(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepDcwFtime(int argc, const uint8_t *argv[]);

//*********************************ACW STEP指令**********************************//
uint32_t InstructionExec_StepAcwTtime(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepAcwRtime(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepAcwItime(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepAcwVolt(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepAcwCurHigh(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepAcwCurLow(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepAcwArc(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepAcwFREQuency(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepAcwRange(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepAcwFtime(int argc, const uint8_t *argv[]);

//*********************************GR STEP指令**********************************//

uint32_t InstructionExec_StepGrCurr(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepGrLow(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepGrHigh(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepGrTtime(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepGrItime(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepGrFREQuency(int argc, const uint8_t *argv[]);

//*********************************IR STEP指令**********************************//
uint32_t InstructionExec_StepIrVolt(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepIrArange(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepIrHigh(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepIrLow(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepIrTtime(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepIrRtime(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepIrItime(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepIrDtime(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepIrDmode(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepLcCCurr(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepIrHighMax(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepIrHighMin(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepIrLowMax(int argc, const uint8_t *argv[]);

//*********************************LC STEP指令**********************************//
uint32_t InstructionExec_StepLcTtime(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepLcRtime(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepLcItime(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepLcVolt(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepLcCurHigh(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepLcCurLow(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepLcArc(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepLcFREQuency(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepLcRange(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepLcFtime(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepLcPHASe(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepLcMDnetwork(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepLcMDvol(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepLcAssistvol(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_StepLcMDCom(int argc, const uint8_t *argv[]);

// 系统指令执行函数

uint32_t InstructionExec_SysRhint(int argc, const uint8_t *argv[]);
uint32_t InstructionStruct_SystemOALArm(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_SysRsave(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_SysOcover(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_SysLanguage(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_SysTime(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_SysDate(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_SysKeyKlock(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_SysPswNew(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_SysPswNow(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_SysPswStatus(int argc, const uint8_t *argv[]);

//结果指令执行函数

uint32_t InstructionExec_ResuCapacityUsed(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_ResuCapacityFree(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_ResuCapacityAll(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_ResuCapacityPass(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_ResuCapacityFail(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_ResuPpercent(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_ResuStatistics(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_ResuClear(int argc, const uint8_t *argv[]);

uint32_t InstructionExec_ResuFetchAll(int argc, const uint8_t *argv[]);



void Comm_UpdateTestStatusForComm(TestStatus testStatus);

uint32_t rt_uart_write(const void* buffer, uint32_t size);
uint8_t get_usart2_send_st(void);
uint8_t get_usart2_busy_st(void);
void reset_usart2_busy_st(void);
void set_usart2_busy_st(void);

#endif
