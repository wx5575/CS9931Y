/******************************************************************************
 *                          本文件所引用的头文件
******************************************************************************/ 
 

#include "scpi_parser.h"
#include "ScpiInstructionTable.h"
#include "macro.h"
#include "App_config.h"
#include "stdlib.h"

#include "stm32f4xx.h"


/******************************************************************************
 *                            本文件内部宏定义
******************************************************************************/
#define                         _FRAME_CHK_SUPPORT                     (1)

typedef                         unsigned char                          portHASHCODE_TYPE;
#define                         HASH_INIT_CODE                         (13)  //取素数比较好
#define                         PARSER_START_POS                       (0)

#define                         PARAM_CONTAINER_NORMAL                 (0)
#define                         PARAM_CONTAINER_PTR_ARRAY              (1)

#define                         PARAM_SPECIAL_ATTR_MIN                 (0x01)
#define                         PARAM_SPECIAL_ATTR_MAX                 (0x02)
#define                         PARAM_SPECIAL_ATTR_DEF                 (0x04)
#define                         PARAM_SPECIAL_ATTR_INF                 (0x08)

//写入一个字符
#define                         PARAM_QUEUE_CHAR_WRITE(value)          (*(uint8_t *)(&t_ParserExeModuler.m_pparamQueue[t_ParserExeModuler.m_paramPutPosIndex++]) = value)
#define                         PARAM_QUEUE_CURR_POS_GET()             (&t_ParserExeModuler.m_pparamQueue[t_ParserExeModuler.m_paramPutPosIndex])
#define                         PARAM_QUEUE_CURR_POS_ADJ(value)        (t_ParserExeModuler.m_paramPutPosIndex += (value))
#define                         PARAM_QUEUE_SPLIT_SIGN_WRITE()         (PARAM_QUEUE_CHAR_WRITE('\0'))
                            
//单位符号类
#define                         SIGN_SYMBOL_CLASS(sign)                (sign << 3)
#define                         SIGN_SYMBOL_CLASS_GET(sign)            (sign >> 3)
#define                         SIGN_SYMBOL_INDEX_GET(sign)            (sign & 0x07)

#define                         REAL_ATTR_SPECIAL                      (0x01)
#define                         REAL_ATTR_UINT32                       (0x02)

//符号之间间隔长度 如:V -> kV  间隔长度为3
#define                         SIGN_INTERVAL_BIT                      (3)

//输出队列压入类型 -> 字符串
#define                         _OUTPUT_QUEUE_PUSH_STR                 (0)
//输出队列压入类型 -> 整形
#define                         _OUTPUT_QUEUE_PUSH_INT                 (1)
//输出队列压入类型 -> 字符
#define                         _OUTPUT_QUEUE_PUSH_CHAR                (2)
                                
/******************************************************************************
 *                       本文件所定义的全局结构体
******************************************************************************/  

//解析器状态
typedef enum
{
    STATUS_PARSERING = 0,
    STATUS_EXECUTING,
    STATUS_IDLE,                                                          

}ParserStatus;

//解析执行组件
typedef struct 
{
    //---------------------------------------------------------------------------------------------------------------
    //解析部分
    const ParserContainer                      *m_currParserContainerPtr;               //解析器当前命令结构体指针
    const ParserContainer                      *m_rootParserContainerPtr;               //解析结构体根地址 
    FP_pfInstructionExec                       *m_executeFunc;                          //指令执行函数 
    //指令参数地址指针
//    const uint8_t                                *m_pparamStrAddrArray[INSTRUCTION_MAX_PARAM+2]; //参数字符串地址指针数组
    const uint8_t                               **m_ppparamStrAddrArray;                  //指向参数字符串地址指针数组的指针
    const uint8_t                                *m_pparamQueue;                          //指向参数队列的指针

    //---------------------------------------------------------------------------------------------------------------
    
    uint16_t                                      m_instructionExecuteMaxTime;            //指令执行最长时间
    portQUEUELEN_TYPE                           m_parserPosIndex;                       //解析位置索引 记录当前的解析位置   
    portQUEUELEN_TYPE                           m_paramPutPosIndex;                     //参数放置位置索引
    //---------------------------------------------------------------------------------------------------------------

    uint8_t                                       m_parserStatus;                         //解析器状态  解析中 执行中 空闲
    ParserErrCode                               m_parserErrCode;                        //解析器错误码 
    uint8_t                                       m_endCodeIndex;                         //结束符索引
    uint8_t                                       m_responsionCtrl;                       //应答机制控制
    uint8_t                                       m_instructionAttr;                      //指令属性 执行、查询、公共指令
    uint8_t                                       m_instructionExecuteTimeOut;            //指令执行超时
    uint8_t                                       m_totalInstructionChains;               //命令链总数 命令连发
    uint8_t                                       m_singleInstructionScales;              //单个命令级数 一条命令由几级构成
    uint8_t                                       m_singleInstructionParamCnts;           //单个命令参数个数
    uint8_t                                       m_rootParserStructCnts;                 //根解析结构体个数
    uint8_t                                       m_currParserStructCnts;                 //当前解析结构体个数
    uint8_t                                       m_longLenPerScale[INSTRUCTION_MAX_SCALE];  //每级指令的长指令长度
    uint8_t                                       m_shortLenPerScale[INSTRUCTION_MAX_SCALE]; //每级指令的短指令长度

    //---------------------------------------------------------------------------------------------------------------
    
    //输入输出队列部分
    portQUEUELEN_TYPE   			            m_inputQueueLen;                        //输入队列长度
    portQUEUELEN_TYPE   			            m_outputQueueLen;                       //输出队列长度
     int8_t    	            				    m_inputQueue[QUEUE_MAX_LEN];            //输入队列
     int8_t    					                m_outputQueue[QUEUE_MAX_LEN];           //输出队列

}ParserExeModuler;

ParserExeModuler                t_ParserExeModuler;                 
//结束符
static uint8_t                    s_EndCode                               = 0;
//预执行合法性判断函数指针
static FP_pfValidityChk        *s_FP_pfPrevValidityChk                  = NULL;
//输出队列输出函数指针
static FP_pfOutputQueueSend    *s_FP_pfOutputQueueSend                  = NULL;
//实型参数拆分自定义函数指针
static FP_pfRealParamSplit     *s_FP_pfRealParamSplit                   = NULL;
/*
const uint8_t *const API_PEM_pParserExeModulerErrCodeArray[] = {

        (const uint8_t *)"+0",                          //"+0, No error"
		(const uint8_t *)"-102",                        //"-102, Syntax error"
		(const uint8_t *)"-105",                        //"-105, Execute not allowed"
		(const uint8_t *)"-108",                        //"-108, Parameter not allowed"
		(const uint8_t *)"-109",                        //"-109, Missing parameter"
		(const uint8_t *)"-113",                        //"-113, Undefined header"
		(const uint8_t *)"-120",                        //"-120, Parameter type error"
		(const uint8_t *)"-121",                        //"-121, Parameter length error"
		(const uint8_t *)"-151",                        //"-151, Invalid string data"
		(const uint8_t *)"-152",                        //"-152, Execute time out"
		(const uint8_t *)"-222",                        //"-222, Data out of range"
        (const uint8_t *)"-252",                        //"-252, Output Queue Full"

    };
*/
const uint8_t *const API_PEM_pParserExeModulerErrCodeArray[] = {

        (const uint8_t *)"+0, No error",
				(const uint8_t *)"-102, Syntax error",
				(const uint8_t *)"-105, Execute not allowed",
				(const uint8_t *)"-108, Parameter not allowed",
				(const uint8_t *)"-109, Missing parameter",
				(const uint8_t *)"-113, Undefined header",
				(const uint8_t *)"-120, Parameter type error",
				(const uint8_t *)"-121, Parameter length error",
				(const uint8_t *)"-151, Invalid string data",
				(const uint8_t *)"-152, Execute time out",
				(const uint8_t *)"-222, Data out of range",
        (const uint8_t *)"-252, Output Queue Full",
        (const uint8_t *)"-262, Sum Check Error",

    };
// const uint8_t *const API_PEM_pParserExeModulerErrCodeArray[] = {

//         (const uint8_t *)"1,00",
// 		(const uint8_t *)"0,Syntax error",
// 		(const uint8_t *)"0,Execute not allowed",
// 		(const uint8_t *)"0,Parameter not allowed",
// 		(const uint8_t *)"0,Missing parameter",
// 		(const uint8_t *)"0,Undefined header",
// 		(const uint8_t *)"0,Parameter type error",
// 		(const uint8_t *)"0,Parameter length error",
// 		(const uint8_t *)"0,Invalid string data",
// 		(const uint8_t *)"0,Execute time out",
// 		(const uint8_t *)"0,Data out of range",
//         (const uint8_t *)"0,Output Queue Full",
//         (const uint8_t *)"0,Sum Check Error",

//     };

typedef struct
{
    uint8_t                                       m_lowerLimitBitLen;
    uint8_t                                       m_upperLimitBitLen;
    //配置信息   高4位为字符串除去小数点之后的长度  低4位为小数点在字符串内的偏移
    uint8_t                                       m_configBitLen;
    uint8_t                                       m_dotBitLen;
    //符号信息                                  
    uint8_t                                       m_signBitLen;

}ParamRealSectorInfoStruct;

typedef struct                                
{
    uint8_t                                       *m_psignStr;
    //相对索引 相对于符号系中最低符号的索引 如kv 对 v 的索引为1  实际值为1*1000
    uint8_t                                        m_relativeIndex;

}SignSymbolInfo;

static const SignSymbolInfo st_SignSymbolTable[] = {
    
        //nA uA mA A 符号系列
        {"nA", SIGN_SYMBOL_CLASS(0)|0}, {"uA", SIGN_SYMBOL_CLASS(0)|1}, {"mA", SIGN_SYMBOL_CLASS(0)|2}, {"A", SIGN_SYMBOL_CLASS(0)|3},
        //V kV 符号系列
        {"V", SIGN_SYMBOL_CLASS(1)|0}, {"kV", SIGN_SYMBOL_CLASS(1)|1},
        //mohm ohm kohm Mohm Gohm Tohm 符号系列
        {"mohm", SIGN_SYMBOL_CLASS(2)|0}, {"ohm", SIGN_SYMBOL_CLASS(2)|1}, {"kohm", SIGN_SYMBOL_CLASS(2)|2}, 
        {"Mohm", SIGN_SYMBOL_CLASS(2)|3}, {"Gohm", SIGN_SYMBOL_CLASS(2)|4}, {"Tohm", SIGN_SYMBOL_CLASS(2)|5},
        //s 符号系列
        {"ms", SIGN_SYMBOL_CLASS(3)|0}, {"s", SIGN_SYMBOL_CLASS(3)|1},
        //Hz 符号系列
        {"Hz", SIGN_SYMBOL_CLASS(4)|0},
        //W 符号系列
        {"W", SIGN_SYMBOL_CLASS(5)|0},
    };

/******************************************************************************
 *                           本文件静态函数声明
******************************************************************************/ 

static uint32_t _API_PEMPrevExecuteChk(void);
static uint32_t _API_PEMSyntaxAnalyze(void);
static uint32_t _API_PEMInstructionAnalyze(void);
static uint32_t _API_PEMParameterAnalyze(ParamContainer *pparamContainer, uint8_t paramContainerCapacity, 
                                              uint8_t recvParamCnts, uint32_t paramContainerInfo);
 
static portHASHCODE_TYPE _API_PEMStrHashCodeGet(const uint8_t *prscStr, uint32_t len);
static  ParamTypeInfo _API_PEMParamTyleClassGet(int8_t *paccessAddr, uint32_t paramLen, 
                                                ParamContainer *pparamContainer,
                                                uint32_t *pparamSpecialAttr);
static  uint32_t _API_PEMIntegerParamValidChk(uint8_t *rscAddr, ParamContainer *pparamContainer, uint32_t paramSpecialAttr);
static  uint32_t _API_PEMCharacterParamValidChk(uint8_t *rscAddr, ParamContainer *pparamContainer, uint32_t paramSpecialAttr);
static  uint32_t _API_PEMStringParamValidChk(uint8_t *rscAddr, ParamContainer *pparamContainer, uint32_t paramSpecialAttr);
static  uint32_t _API_PEMRealParamValidChk(uint8_t *rscAddr, ParamContainer *pparamContainer, uint32_t paramSpecialAttr);
static  uint32_t _API_PEMFloatParamValidChk(uint8_t *rscAddr, ParamContainer *pparamContainer, uint32_t paramSpecialAttr);
static  void _API_PEMParamQueueStrWrite(const uint8_t *prscStrAddr, uint8_t rscStrLen);
static uint32_t _API_PEMNullValidityChk(struct EXECUTE_VALIDITY_CHK_INFO *pvalidityChkInfo);
static uint32_t _API_PEMParserErrChk(void);
static uint32_t _RealParamSplitServ(RealParamSplitStruct *prealParamSplitStruct);

#if (_FRAME_CHK_SUPPORT > 0)

    static uint8_t _API_PEMFrameChkSumGet(const uint8_t *prscQueue, portQUEUELEN_TYPE len);

#endif    

                  
/******************************************************************************
 *                        本文件静态函数数组指针声明
******************************************************************************/ 

static uint32_t (*pf_API_PEMParamValidChk[])(uint8_t *rscAddr, ParamContainer *pparamContainer, uint32_t paramSpecialAttr) = {

        _API_PEMIntegerParamValidChk, _API_PEMCharacterParamValidChk, _API_PEMStringParamValidChk, 
        _API_PEMRealParamValidChk, _API_PEMFloatParamValidChk,
    };

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

void API_PEMInit(ParserContainer *ptrootParserContainer, uint8_t prootParserContainerCapacity,
                 uint8_t endCodeIndex, uint8_t responsionCtrl, FP_pfOutputQueueSend *pfOutputQueueSend)
{
    //获取根解析容器信息
    t_ParserExeModuler.m_rootParserContainerPtr             = ptrootParserContainer;
    t_ParserExeModuler.m_rootParserStructCnts               = prootParserContainerCapacity;
    //初始化解析器状态
    t_ParserExeModuler.m_parserStatus                       = STATUS_IDLE;
    t_ParserExeModuler.m_parserErrCode                      = STATUS_NO_ERROR;
    //初始化解析地址索引
    t_ParserExeModuler.m_parserPosIndex                     = PARSER_START_POS;
    t_ParserExeModuler.m_currParserContainerPtr             = t_ParserExeModuler.m_rootParserContainerPtr;
    t_ParserExeModuler.m_currParserStructCnts               = t_ParserExeModuler.m_rootParserStructCnts;
    //初始化结束符
    t_ParserExeModuler.m_endCodeIndex                       = endCodeIndex; 
    //初始化预执行合法性检测函数指针
    s_FP_pfPrevValidityChk                                  = _API_PEMNullValidityChk;
    //初始化输出队列发送函数指针
    s_FP_pfOutputQueueSend                                  = pfOutputQueueSend;
    //初始化实型参数拆分自定义函数指针为空
    //s_FP_pfRealParamSplit                                   = NULL;
    API_PEMRealParamSplitFuncInstall(_RealParamSplitServ);//
//     if ((CR_LF_INDEX == endCodeIndex) || (LF_INDEX == endCodeIndex))
//     {
//         //结束符为LF--换行
//         s_EndCode                                           = '\n';
//     }
//     else
//     {
        //结束符为#
        s_EndCode                                           = '#';
//     }
    //初始化应答机制
    t_ParserExeModuler.m_responsionCtrl                     = responsionCtrl;
    //清空输入队列
    _API_PEMInputQueueFlush();
    //清空输出队列
    _API_PEMOutputQueueFlush();
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
int8_t              paramQueue[QUEUE_MAX_LEN];

void API_PEMExecute(void)
{    
    const uint8_t       *pparamStrAddrArray[INSTRUCTION_MAX_PARAM+2];     //参数字符串地址指针数组
    //int8              paramQueue[QUEUE_MAX_LEN];
// 	s_FP_pfOutputQueueSend((const uint8_t *)t_ParserExeModuler.m_inputQueue, t_ParserExeModuler.m_inputQueueLen);
    //判断是否需要执行
    if (STATUS_PARSERING == t_ParserExeModuler.m_parserStatus)
    {
        ExecuteValidityChkInfo  t_validityChkInfo;

        t_ParserExeModuler.m_ppparamStrAddrArray                = pparamStrAddrArray;
        t_ParserExeModuler.m_pparamQueue                        = (const uint8_t *)paramQueue;
        
        //---------------------------------------------解析器--------------------------------------------------

        #if (_FRAME_CHK_SUPPORT > 0)
        {
            //计算帧校验和是否正确 接收到的校验和是否等于计算出的校验和 结束符为#时 没有校验和
            //t_ParserExeModuler.m_inputQueueLen长度为包含结束符的长度
            if (CODE_0x23_INDEX != t_ParserExeModuler.m_endCodeIndex)
            {
                if (((uint8_t)t_ParserExeModuler.m_inputQueue[t_ParserExeModuler.m_inputQueueLen-1])
                     != _API_PEMFrameChkSumGet((const uint8_t *)t_ParserExeModuler.m_inputQueue, t_ParserExeModuler.m_inputQueueLen-1))
                {
                    API_PEMExecuteErrCodeSet(STATUS_SUM_CHECK_ERROR);
                    _API_PEMParserErrChk();
                    t_validityChkInfo.m_step                    = VALIDITY_CHK_STEP_RREV_EXECUTE;
                    goto _quit;
                }
                else
                {
                    //修正接收到的数据长度 因为解析过程中是不需要校验和数据的
                    t_ParserExeModuler.m_inputQueue[t_ParserExeModuler.m_inputQueueLen - 1] = ';';
                    t_ParserExeModuler.m_inputQueueLen--;
                }
            }
        }
        #endif

        do {
            //预执行判断 -> 语法分析 -> 指令分析 -> 参数分析   指令分析中 包含指令部分分析与参数部分分析
            if ((TRUE == _API_PEMPrevExecuteChk()) && (TRUE == _API_PEMSyntaxAnalyze()) && (TRUE == _API_PEMInstructionAnalyze()))
            {
        //---------------------------------------------执行器--------------------------------------------------
                //解析器进入执行状态
                t_ParserExeModuler.m_parserStatus               = STATUS_EXECUTING;
                //判断执行函数是否为空 若不为空则执行此函数
                if (NULL != t_ParserExeModuler.m_executeFunc)
                {
                    t_validityChkInfo.m_pfinstructionExec       = t_ParserExeModuler.m_executeFunc;
                    t_validityChkInfo.m_instructionAttr         = t_ParserExeModuler.m_instructionAttr; 
                    t_validityChkInfo.m_step                    = VALIDITY_CHK_STEP_RREV_EXECUTE;
                    
                    //当预执行合法性检测函数返回值不为TRUE时 终止执行
                    if (TRUE != s_FP_pfPrevValidityChk(&t_validityChkInfo))
                    {
                        _API_PEMParserErrChk();
                        break;
                    }         
                    //处于活动状态 且控制方式为远控时 所有指令都响应
                    //t_ParserExeModuler.m_singleInstructionParamCnts+2， 因为增加了命令执行函数、命令属性信息
                    if (TRUE == (*t_ParserExeModuler.m_executeFunc)(t_ParserExeModuler.m_singleInstructionParamCnts+2, 
                            t_ParserExeModuler.m_ppparamStrAddrArray))
                    {
                        //执行指令 执行成功时
                        if ((t_ParserExeModuler.m_instructionAttr & INSTRUCTION_ATTR_EXECUT)
                            && (STATUS_NO_ERROR == t_ParserExeModuler.m_parserErrCode))
                        {
                            //压入执行成功信息
                            API_PEMOutputQueueErrMsgPush(t_ParserExeModuler.m_parserErrCode, NO_APPENDED_SIGN);
                        }
                        //对于多级连发指令 只有在执行成功时 才添加分割符  ； 指令可以为执行指令和查询指令
                        if (t_ParserExeModuler.m_totalInstructionChains > 1)
                        {
                            API_PEMOutputQueueCharPush(';', NO_APPENDED_SIGN);
                        }
                    }
                    //函数执行中发生错误
                    else 
                    {
                        //判断错误码:t_ParserExeModuler.m_parserErrCode 类型 
                        //当t_ParserExeModuler.m_parserErrCode 不为 STATUS_NO_ERROR时 将其压入输出队列
                        _API_PEMParserErrChk();
                        break;
                    }
                }                        
            }
            else
            {
                //中间发生错误 停止解析后面指令 退出解析执行部分
                t_validityChkInfo.m_pfinstructionExec       = NULL;
                t_validityChkInfo.m_instructionAttr         = INSTRUCTION_ATTR_NONE;
                break;
            }

        }while (--t_ParserExeModuler.m_totalInstructionChains > 0);

        //--------------------------------------------输出队列-------------------------------------------------
_quit:  t_validityChkInfo.m_step                                = VALIDITY_CHK_STEP_RREV_OUTPUT;
        //不为广播通讯状态且输出队列准备好->通过输出队列长度来判断  且 控制状态为不为非活动状态
        if ((0 != t_ParserExeModuler.m_outputQueueLen) && (TRUE == s_FP_pfPrevValidityChk(&t_validityChkInfo)))
        {
            //若为执行指令 则必须在应答机制关闭时 输出队列不进行输出操作
            //其他情况下执行输出队列输出操作
            if (!((t_ParserExeModuler.m_instructionAttr & INSTRUCTION_ATTR_EXECUT)
                && (CLOSE == t_ParserExeModuler.m_responsionCtrl)))
            {
                #if (_FRAME_CHK_SUPPORT > 0)
                {
                     if (CODE_0x23_INDEX != t_ParserExeModuler.m_endCodeIndex)
                     {
                        //计算帧校验和
                        t_ParserExeModuler.m_outputQueue[t_ParserExeModuler.m_outputQueueLen] =
                            _API_PEMFrameChkSumGet((const uint8_t *)t_ParserExeModuler.m_outputQueue, t_ParserExeModuler.m_outputQueueLen);
                        t_ParserExeModuler.m_outputQueueLen++;
                     }
                }
                #endif

                //添加结束符 下位机返回信息固定以CR+LF为结束符
 								t_ParserExeModuler.m_outputQueue[t_ParserExeModuler.m_outputQueueLen++] = '#';
//                 t_ParserExeModuler.m_outputQueue[t_ParserExeModuler.m_outputQueueLen++] = '\r';
//                 t_ParserExeModuler.m_outputQueue[t_ParserExeModuler.m_outputQueueLen++] = '\n';
                //执行输出队列发送函数
                if (NULL != s_FP_pfOutputQueueSend)
                {
                    s_FP_pfOutputQueueSend((const uint8_t *)t_ParserExeModuler.m_outputQueue, t_ParserExeModuler.m_outputQueueLen);
                }
            }
        }
        //--------------------------------------------初始化PEM-------------------------------------------------
        //清空接收队列
        _API_PEMInputQueueFlush();
        //清空发送队列
        _API_PEMOutputQueueFlush();
        //初始化解析地址索引
        t_ParserExeModuler.m_parserPosIndex                     = PARSER_START_POS;
        //初始化命令链总级数
        t_ParserExeModuler.m_totalInstructionChains             = 0;
        t_ParserExeModuler.m_parserErrCode                      = STATUS_NO_ERROR;
        //解析器恢复空闲状态
        t_ParserExeModuler.m_parserStatus                       = STATUS_IDLE;
    }
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

static uint32_t _API_PEMParserErrChk(void)
{
    if (STATUS_NO_ERROR != t_ParserExeModuler.m_parserErrCode)
    {
        //将错误信息压入输出队列中
        API_PEMOutputQueueErrMsgPush(t_ParserExeModuler.m_parserErrCode, NO_APPENDED_SIGN);
        return FALSE;    
    }
    return TRUE;
}

static uint32_t _API_PEMPrevExecuteChk(void)
{
    //初始化执行函数与执行属性
    t_ParserExeModuler.m_executeFunc                    = NULL;
    t_ParserExeModuler.m_instructionAttr                = INSTRUCTION_ATTR_NONE;
    t_ParserExeModuler.m_paramPutPosIndex               = 0;
    return _API_PEMParserErrChk();
}

/******************************************************************************
 *  函数名称 :                                                                
 *                                                                           
 *  函数功能 : 对接收到的字符串进行分析  只解析一条命令串                                                                
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
 *  附    注 : 在本函数内产生的错误 全部为语法错误STATUS_SYNTAX_ERROR                                                              
 *             语法分析程序对双引号范围内的字符不进行分析                                                               
 *                                                                            
******************************************************************************/

static uint32_t _API_PEMSyntaxAnalyze(void)
{
     int8_t                   recvMsg                             = 0;
    portQUEUELEN_TYPE       parserStartPosIndex                 = PARSER_START_POS + t_ParserExeModuler.m_parserPosIndex;
    portQUEUELEN_TYPE       i                                   = parserStartPosIndex;
    //空格符号出现位置
    portQUEUELEN_TYPE       spaceSignAppearPos                  = 0;
    //星号符号出现的次数
    portQUEUELEN_TYPE       starSignAppearPos                   = 0;
    //第二个双引号处 后面紧跟的一定是逗号 因此需要触发判断下 本变量记录的是此刻的位置
    portQUEUELEN_TYPE       quoteSignValidChkPos                = 0;
    //空格出现的次数
    uint32_t           spaceSignAppearCnts                = 0;
    //小数点出现的次数
    uint32_t           dotAppearCnts                      = 0;
    //引号出现的次数
    uint32_t           quoteSignAppearCnts                 = 0;
    //第二个双引号处 后面紧跟的一定是逗号 因此需要触发判断下
    uint32_t           quoteSignFormateValid               = TRUE;
    //小写字符是否出现
    uint32_t           lowerCaseCharAppear                 = FALSE;
    //问号出现的次数
    uint32_t           interrogationSignAppearCnts         = 0;
    uint32_t           longLenPerScale                     = 0;
    uint32_t           shortLenPerScale                    = 0;
    uint32_t           currParserContainerCapacity         = t_ParserExeModuler.m_currParserStructCnts;
    const ParserContainer   *currParserContainerPtr              = t_ParserExeModuler.m_currParserContainerPtr;
    
    //初始化标志位
    t_ParserExeModuler.m_instructionAttr                        = INSTRUCTION_ATTR_EXECUT;
    t_ParserExeModuler.m_instructionExecuteTimeOut              = FALSE;
    t_ParserExeModuler.m_singleInstructionScales                = 0;
    t_ParserExeModuler.m_singleInstructionParamCnts             = 0;
    memset((void *)t_ParserExeModuler.m_longLenPerScale, '\0', sizeof(t_ParserExeModuler.m_longLenPerScale));
    memset((void *)t_ParserExeModuler.m_shortLenPerScale, '\0', sizeof(t_ParserExeModuler.m_shortLenPerScale));

    do{
        //获取一个输入字符
        recvMsg                                             = t_ParserExeModuler.m_inputQueue[i];
        //合法字符 分析的只是特殊字符 不包括A - Z   a - z    0 - 9
        if ((' ' == recvMsg) || (':' == recvMsg) || (',' == recvMsg) || ('"' == recvMsg)  
            || ('*' == recvMsg) || ('?' == recvMsg) || (';' == recvMsg))
        {
            //星号字符情况
            if ('*' == recvMsg)
            {
                //当为第一个出现的字符时
                if (parserStartPosIndex == i) 
                { 
                    //从根路径开始解析  先暂存在currParserStructPtr中 等到语法正确后再更新值s 
                    currParserContainerPtr                       = t_ParserExeModuler.m_rootParserContainerPtr;
                    currParserContainerCapacity                  = t_ParserExeModuler.m_rootParserStructCnts;
                    starSignAppearPos++;
                    //置位公用命令
                    t_ParserExeModuler.m_instructionAttr      |= INSTRUCTION_ATTR_COMM;
                    //解析地址索引向后加一
                    t_ParserExeModuler.m_parserPosIndex++;
                }
                //此时星号已经出现超过一次了 双引号没有出现时->语法错误
                else if (0 == quoteSignAppearCnts)
                {
                    break;
                }
            }
            //冒号字符情况  
            else if (':' == recvMsg)
            {
                if (parserStartPosIndex == i) 
                { 
                    //从根路径开始解析  先暂存在currParserStructPtr中 等到语法正确后再更新值s 
                    currParserContainerPtr                        = t_ParserExeModuler.m_rootParserContainerPtr;
                    currParserContainerCapacity                   = t_ParserExeModuler.m_rootParserStructCnts;
                    //解析地址索引向后加一
                    t_ParserExeModuler.m_parserPosIndex++;
                }
                //判断星号是否出现过且双引号是否出现过
                else if (0 == quoteSignAppearCnts)  
                {
                    if (starSignAppearPos > 0)
                    {
                        break;      
                    }
                    //单个命令串级数增加判断
                    else
                    {
                        //记录本级指令段的长、短指令长度  此处情况是前几级  不包括最后一级
                        t_ParserExeModuler.m_longLenPerScale[t_ParserExeModuler.m_singleInstructionScales]  = longLenPerScale;
                        t_ParserExeModuler.m_shortLenPerScale[t_ParserExeModuler.m_singleInstructionScales] = shortLenPerScale; 
                        t_ParserExeModuler.m_singleInstructionScales++;
                        //将当前的冒号替换成结束符'\0'
                        t_ParserExeModuler.m_inputQueue[i]       = '\0'; 
                        //下一级指令段解析 初始化小写字符出现过标识符    
                        lowerCaseCharAppear                      = FALSE;
                        longLenPerScale                          = 0;
                        shortLenPerScale                         = 0;
                    }
                }
            }
            //空格字符情况 空格在一条命令串中只能出现一次  但在具有双引号参数中可以出现多次 
            //空格符也可出现在实数类型和浮点类型中     
            else if (' ' == recvMsg)
            {
                //不在双引号范围内
                if (0 == (quoteSignAppearCnts % 2))
                {
                    spaceSignAppearCnts++;
                }
                //记录下空格第一次出现时的位置
                if (1 == spaceSignAppearCnts)
                {
                    //判断空格出现时 当前命令短长度是否为零 若为零 语法错误 出现了冒号后面直接跟空格的错误
                    if (0 == longLenPerScale)
                    {
                        break;
                    }
                    spaceSignAppearPos                          = i;
                    //将当前的空格替换成结束符'\0'
                    t_ParserExeModuler.m_inputQueue[i]          = '\0';
                    //判断空格字符的下一个字符是否结束符 ; 若是则发生语法错误
                    if (';' == t_ParserExeModuler.m_inputQueue[i+1])
                    {
                        break;
                    }
                    else
                    {
                        //在此假设后面有参数 参数个数加一
                        t_ParserExeModuler.m_singleInstructionParamCnts++;
                    }
                }
            }
            //双引号字符情况 双引号字符必须成对出现
            else if ('"' == recvMsg)
            {
                quoteSignAppearCnts++; 
                if (0 == (quoteSignAppearCnts % 2))
                {
                    //触发判断尾随的字符是否为逗号 先假设为不合法
                    quoteSignFormateValid                       = FALSE;
                    //记录当前的位置
                    quoteSignValidChkPos                        = i;
                }  
            }
            //逗号字符情况 每出现一个
            else if (',' == recvMsg)
            {
                //逗号必须在空格之后出现  空格和逗号之间的位置间隔必须大于1
                if ((0 == spaceSignAppearCnts) || ((i - spaceSignAppearPos) <= 1))
                {
                    break;
                }
                //若第二个双引号后尾随的是逗号字符 则设置quoteSignFormateValid = TRUE
                if (FALSE == quoteSignFormateValid)
                {
                    quoteSignFormateValid                       = TRUE;
                }
                t_ParserExeModuler.m_singleInstructionParamCnts++;
                //当双引号出现次数为成对出现后 将逗号替换为结束符'\0'
                if (0 == (quoteSignAppearCnts % 2))
                {
                    //将当前的冒号替换成结束符'\0'
                    t_ParserExeModuler.m_inputQueue[i]          = '\0';
                }
            }
            //问号字符情况
            else if ('?' == recvMsg)
            {
                //不在双引号范围内
                if (0 == (quoteSignAppearCnts % 2))
                {
                    interrogationSignAppearCnts++;
                }
                //双引号没有出现 但是却出现了两次问号的情况下 -> 语法错误
                if (1 == interrogationSignAppearCnts)
                {
                    //取消执行属性    对于一条接收到的指令来讲 查询属性和执行属性是互斥的
                    t_ParserExeModuler.m_instructionAttr &= (~INSTRUCTION_ATTR_EXECUT);
                    //置位查询属性
                    t_ParserExeModuler.m_instructionAttr |= INSTRUCTION_ATTR_QUERY;
                }
                else
                {
                    break;
                }
            }
        }
        //判断字符是否在A - Z范围内 
        else if ((recvMsg >= 'A') && (recvMsg <= 'Z'))
        {
            //在指令解析阶段 若在小写字符后面出现了大写字符 则语法错误
            if (0 == spaceSignAppearCnts)
            {
                if (TRUE == lowerCaseCharAppear)
                {
                    break;
                }
                //短指令长度加一
                shortLenPerScale++;
                //长指令长度加一
                longLenPerScale++;
            }
        }
        //判断字符是否在a - z    0 - 9 + - . 范围内 
        else if (((recvMsg >= 'a') && (recvMsg <= 'z'))
            || ((recvMsg >= '0') && (recvMsg <= '9')) || ('+' == recvMsg) || ('-' == recvMsg) || ('.' == recvMsg))
        {
            //空格符未出现时 才是指令部分
            if (spaceSignAppearCnts == 0)
            {
                //长指令长度加一
                longLenPerScale++;
                lowerCaseCharAppear                                = TRUE;
            }

            //双引号外面小数点个数大于一 语法错误
            if ('.' == recvMsg)
            {
                if (0 == (quoteSignAppearCnts % 2))
                {
                    dotAppearCnts++;
                }

                if (dotAppearCnts > 1)
                {
                    break;
                }
            }
        }
        else
        {
            //非法字符
            break;
        }

        //对第二个双引号后面尾随的字符是否为逗号进行判断 且不为';'时
        //第二个分号后面可以直接为';'
        if ((FALSE == quoteSignFormateValid) && ((i - quoteSignValidChkPos) == 1)
            && (recvMsg != ';'))
        {
            //没有被尾随的逗号置位为合法 -> 语法错误
            break;    
        }
        //索引下一个字符
        i++;

    //; 分号做为指令与指令之间的分隔符
    }while (recvMsg != ';');

    //判断解析程序是否进行到最后一个字符 若没有则代表中间一定出现了语法错误
    if (recvMsg != ';')
    {
        t_ParserExeModuler.m_parserErrCode                      = STATUS_SYNTAX_ERROR;
    }    
    else
    {
        //对双引号出现的情况进行判断 双引号字符必须成对出现
        //判断结束符之前的字符是否为逗号 若为逗号 则认定为语法错误  逗号已经被上面的解析程序替换为'\0'
        if ((0 != (quoteSignAppearCnts % 2)) || ('\0' == t_ParserExeModuler.m_inputQueue[i-2])
            || (0 == longLenPerScale))
        {
            t_ParserExeModuler.m_parserErrCode                  = STATUS_SYNTAX_ERROR;
        }
        else
        {
            //将结束符';'替换为'\0'
            t_ParserExeModuler.m_inputQueue[i-1]                = '\0';
            //记录本级指令段的长、短指令长度  此处情况是最后一级
            t_ParserExeModuler.m_longLenPerScale[t_ParserExeModuler.m_singleInstructionScales]  = longLenPerScale;
            t_ParserExeModuler.m_shortLenPerScale[t_ParserExeModuler.m_singleInstructionScales] = shortLenPerScale;
        }
    }
    //检查错误信息
    if (TRUE != _API_PEMParserErrChk())
    {
        return FALSE;
    }   
    //此时更新当前解析结构体指针值
    t_ParserExeModuler.m_currParserContainerPtr                    = currParserContainerPtr;
    //此时更新当前解析结构体容量
    t_ParserExeModuler.m_currParserStructCnts                      = currParserContainerCapacity;
    //当前指令级数自加一
    t_ParserExeModuler.m_singleInstructionScales++;

    return TRUE;
}

/******************************************************************************
 *  函数名称 :                                                                
 *                                                                           
 *  函数功能 : 指令分析 查找对应的指令是否存在 对输入队列里数据进行分析                                                              
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

static uint32_t _API_PEMInstructionAnalyze(void)
{
    HeadScaleParserStruct           *ptprevScaleParserStruct;
    TailScaleParserStruct           *ptlastScaleParserStruct;
    const ParserContainer           *ptparserContainer;
    const ParamTotalContainer       *ptparamTotalContainer;
    uint8_t                           *currParserAddr;
    const uint8_t                     *prscStrAddr;
    void                            *pvoid;
    uint32_t                   segmentLongHashCode, segmentShortHashCode, segmentLongLen, segmentShortLen, i;
    uint32_t                   longInstructionHashCode, shortInstructionHashCode, instructionAttribute;
    uint32_t                   rscStrCmpTrig, parserStructType, instructionMatchChkTrig = TRUE;
    uint32_t                   singleInstructionScales     = 0;
    uint32_t                   paramContainerInfo          = PARAM_CONTAINER_NORMAL;
    uint32_t                   paramTotalContainerAttr;
    //使用uint8_t类型 代表解析容器容量、参数的最大个数为256
    uint8_t                            nextParserContainerCapacity, totalParamCnts, recvParamCnts, optionalParamCnts;

    do {                                        
        //当前解析容器指针
        ptparserContainer                                       = t_ParserExeModuler.m_currParserContainerPtr;
        //更新当前解析地址
        currParserAddr                                          = (uint8_t *)&t_ParserExeModuler.m_inputQueue[t_ParserExeModuler.m_parserPosIndex];
        //获取指令段中长指令长度
        segmentLongLen                                          = t_ParserExeModuler.m_longLenPerScale[singleInstructionScales];
        //获取指令段中短指令长度
        segmentShortLen                                         = t_ParserExeModuler.m_shortLenPerScale[singleInstructionScales];
        //获取指令段HASH码 长指令长度等于短指令长度时 此时接收到的指令为短指令 获取短指令码
        if (segmentLongLen == segmentShortLen)
        {
            //获取指令段中短指令HASH码
            segmentShortHashCode                                = _API_PEMStrHashCodeGet(currParserAddr, segmentShortLen);
        }
        else
        {
            //获取指令段中长指令HASH码
            segmentLongHashCode                                 = _API_PEMStrHashCodeGet(currParserAddr, segmentLongLen);
        }
        
        //查找整个容器 容器指针ptparserContainer向后遍历
        for (i = 0; i < t_ParserExeModuler.m_currParserStructCnts; i++, ptparserContainer++)
        {
            //消除源字符串比较触发标志位
            rscStrCmpTrig                                       = FALSE;
            instructionMatchChkTrig                             = TRUE;
            parserStructType                                    = (ptparserContainer->m_parserStructType);
            //头级解析结构体 或 中间级解析结构体
            if ((PARSER_HEAD_SCALE == parserStructType) || (PARSER_MIDDLE_SCALE == parserStructType))
            {
                ptprevScaleParserStruct                         = (HeadScaleParserStruct *)(ptparserContainer->m_pparserStructAddr);
                //此处没有采用直接地址指针转换的方法是因为不同的平台其长度也不一样
                prscStrAddr                                     = ptprevScaleParserStruct->m_pinstructionStr;
                pvoid                                           = (void *)ptprevScaleParserStruct->m_pnextParserContainer;
                longInstructionHashCode                         = ptprevScaleParserStruct->m_longInstructionHashCode;
                shortInstructionHashCode                        = ptprevScaleParserStruct->m_shortInstructionHashCOde;
                instructionAttribute                            = ptprevScaleParserStruct->m_instructionAttribute;
                //此处为下一个解析容器容量
                nextParserContainerCapacity                     = ptprevScaleParserStruct->m_nextParserContainerCapacity;
            }
            //末级解析结构体 或 单级解析结构体 或 他们的特殊格式 即参数容器中的容器指针是否指向指针数组
            else
            {
                ptlastScaleParserStruct                         = (TailScaleParserStruct *)(ptparserContainer->m_pparserStructAddr);
                prscStrAddr                                     = ptlastScaleParserStruct->m_pinstructionStr;
                pvoid                                           = (void *)ptlastScaleParserStruct->m_executeFunc;
                longInstructionHashCode                         = ptlastScaleParserStruct->m_longInstructionHashCode;
                shortInstructionHashCode                        = ptlastScaleParserStruct->m_shortInstructionHashCOde;
                instructionAttribute                            = ptlastScaleParserStruct->m_instructionAttribute;
            }
            //判断本段指令是否短指令  短指令下长指令长度==短指令长度
            if (segmentLongLen == segmentShortLen)
            {
                //比较短指令HASH码
                if (shortInstructionHashCode == segmentShortHashCode)
                {
                    rscStrCmpTrig                               = TRUE;
                }
            }
            //长指令 比较长指令HASH码
            else if (longInstructionHashCode == segmentLongHashCode)
            {
                rscStrCmpTrig                                   = TRUE;
            }
            //判断源字符串比较是否成功
            if ((TRUE == rscStrCmpTrig) && (0 == strncmp((void *)currParserAddr, (void *)prscStrAddr, segmentLongLen)))
            {
                //判断是否为前级指令段  包括头级和中间级
                if ((PARSER_HEAD_SCALE == parserStructType) || (PARSER_MIDDLE_SCALE == parserStructType))
                {
                    //匹配成功 进入指令下一级匹配
                    t_ParserExeModuler.m_parserPosIndex        += segmentLongLen+1;
                    //移动到下一个容器
                    t_ParserExeModuler.m_currParserContainerPtr = (ParserContainer *)pvoid;
                    //更新容器容量
                    t_ParserExeModuler.m_currParserStructCnts   = nextParserContainerCapacity;
                    //已经匹配成功就不需要下面指令是否匹配成功的判断了 如果判断反而会有问题 因为那里用
                    //t_ParserExeModuler.m_currParserStructCnts和i比较的 而这里前者被更新为下级指令结构体个数了
                    //其实那里要比较的是更新前的值
                    instructionMatchChkTrig                     = FALSE;
                }
                //末级指令段 或 单级指令段 
                //判断指令所支持的级数与所接收到的级数是否相同 加上这个判断--指令连发有问题
                //else if (INSTRUCTION_SCALES_GET(instructionAttribute) != t_ParserExeModuler.m_singleInstructionScales)
                //{
                    //未定义的头  即未定义的指令
                //    t_ParserExeModuler.m_parserErrCode          = STATUS_UNDEFINED_HEADER;
                //}
                else if (!((t_ParserExeModuler.m_instructionAttr & instructionAttribute) & ~INSTRUCTION_ATTR_COMM))
                {
                    //未定义的头  即未定义的指令
                    t_ParserExeModuler.m_parserErrCode          = STATUS_UNDEFINED_HEADER;
                }
                else if (singleInstructionScales != (t_ParserExeModuler.m_singleInstructionScales - 1))
                {
                    //未定义的头  即未定义的指令
                    t_ParserExeModuler.m_parserErrCode          = STATUS_UNDEFINED_HEADER;
                }
                //匹配成功 -> 进入参数解析部分
                else
                {
                    //获取参数总容器
                    ptparamTotalContainer                       = ptlastScaleParserStruct->m_pparamTotalContainer;
                    //总参数容器指针不为空时
                    if (NULL != ptparamTotalContainer)
                    {
                        //获取总参数容器属性
                        paramTotalContainerAttr                 = ptparamTotalContainer->m_containerAttr;
                        //支持执行和查询参数容器
                        if (CONTAINER_ATTR_DOUBLE == (CONTAINER_ATTR_DOUBLE & paramTotalContainerAttr))
                        {
                            //接收到查询指令
                            if (INSTRUCTION_ATTR_QUERY & t_ParserExeModuler.m_instructionAttr)
                            {
                                if (!(CONTAINER_ATTR_QUERY & paramTotalContainerAttr))
                                {
                                    //参数容器指针后移
                                    ptparamTotalContainer++;
                                }
                            }
                            //接收到执行指令
                            else
                            {
                                if (!(CONTAINER_ATTR_EXECUTE & paramTotalContainerAttr))
                                {
                                    //参数容器指针后移
                                    ptparamTotalContainer++;
                                }
                            }
                        }
                        //只支持执行或查询参数容器
                        else
                        {
                            //接收到查询指令
                            if (INSTRUCTION_ATTR_QUERY & t_ParserExeModuler.m_instructionAttr)
                            {
                                if (!(CONTAINER_ATTR_QUERY & paramTotalContainerAttr))
                                {
                                    //未找到总参数容器 因此赋值为空
                                    ptparamTotalContainer       = NULL;
                                }
                            }
                            //接收到执行指令
                            else
                            {
                                if (!(CONTAINER_ATTR_EXECUTE & paramTotalContainerAttr))
                                {
                                    //未找到总参数容器 因此赋值为空
                                    ptparamTotalContainer       = NULL;
                                }
                            }
                        }
                    }
                    //所接收到的参数个数 使用REGISTER 使执行速度变快
                    recvParamCnts                               = t_ParserExeModuler.m_singleInstructionParamCnts;
                    //判断本指令是否具有参数信息
                    if (NULL == ptparamTotalContainer)
                    {
                        //不具有参数信息
                        totalParamCnts                          = 0;
                        optionalParamCnts                       = 0;
                    }
                    //本指令具有参数信息
                    else 
                    {
                        //获取总参数个数和可选参数信息属性值
                        totalParamCnts                          = TOTAL_PARAM_INFO_TOT_CNTS_GET(ptparamTotalContainer->m_paramInfo);
                        optionalParamCnts                       = TOTAL_PARAM_INFO_OPT_CNTS_GET(ptparamTotalContainer->m_paramInfo);
                    }                
                    //判断指令所支持的参数和所接收到的参数个数是否相同
                    if (totalParamCnts != recvParamCnts)
                    {
                        //指令所支持参数大于所接收到的参数个数 且 接收到的参数大于等于(totalParamCnts - optionalParamCnts)时
                        //即指令必须携带的参数个数 也认为正确
                        //除此之外 认定指令参数长度错误
                        if (!((totalParamCnts > recvParamCnts) 
                            && (recvParamCnts >= (totalParamCnts - optionalParamCnts))))
                        {
                            //若所支持的参数长度为零 则认为是参数不允许错误
                            if (0 == totalParamCnts)
                            {
                                t_ParserExeModuler.m_parserErrCode  = STATUS_PARAM_NOT_ALLOWED;
                            }
                            //指令参数长度错误 即个数错误
                            else
                            {
                                t_ParserExeModuler.m_parserErrCode  = STATUS_PARAM_LENGTH_ERROR;
                            }
                            break;
                        }
                    }
                    //判断是否为公用命令
                    if (INSTRUCTION_ATTR_COMM & t_ParserExeModuler.m_instructionAttr)
                    {
                        //判断本指令是否为公用命令
                        if (!(instructionAttribute & INSTRUCTION_ATTR_COMM))
                        {
                            //未定义的头  即未定义的指令
                            t_ParserExeModuler.m_parserErrCode      = STATUS_UNDEFINED_HEADER;
                            break;
                        }
                    }
                    //查询指令的尾级长度要加上问号的长度
                    if (INSTRUCTION_ATTR_QUERY & t_ParserExeModuler.m_instructionAttr)
                    {
                        segmentLongLen++;
                    }                 
                    //初始化参数存放地址索引
                    t_ParserExeModuler.m_paramPutPosIndex       = 0;
                    //匹配成功 进入指令参数部分  指令与参数之间通过一个空格字符分隔
                    t_ParserExeModuler.m_parserPosIndex        += segmentLongLen + 1; 
                    //第一项参数记录执行函数地址信息
                    t_ParserExeModuler.m_ppparamStrAddrArray[0] = (const uint8_t *)pvoid;
                    //第二项参数记录指令属性:查询属性 or 执行属性
                    t_ParserExeModuler.m_ppparamStrAddrArray[1] = (const uint8_t *)(t_ParserExeModuler.m_instructionAttr&(INSTRUCTION_ATTR_QUERY|INSTRUCTION_ATTR_EXECUT));
                    //记录执行函数地址
                    t_ParserExeModuler.m_executeFunc            = (FP_pfInstructionExec *)pvoid;
                }
                break;
            }
        }
        //判断指令是否匹配成功  当两者的值相等时 代表匹配失败
        if ((TRUE == instructionMatchChkTrig) && (t_ParserExeModuler.m_currParserStructCnts == i))
        {
            //未定义的头  即未定义的指令
            t_ParserExeModuler.m_parserErrCode                  = STATUS_UNDEFINED_HEADER;
            break;
        }          
                              
    }while (++singleInstructionScales < t_ParserExeModuler.m_singleInstructionScales);

    //如果为此两种类型之一 则代表没有接收到完整指令 解析也不完整
    if ((PARSER_HEAD_SCALE == parserStructType) || (PARSER_MIDDLE_SCALE == parserStructType))
    {
        t_ParserExeModuler.m_parserErrCode                      = STATUS_UNDEFINED_HEADER;
    }
    //判断指令解析是否发生错误
    if (t_ParserExeModuler.m_parserErrCode != STATUS_NO_ERROR)
    {
        //将错误信息压入输出队列中
        API_PEMOutputQueueErrMsgPush(t_ParserExeModuler.m_parserErrCode, NO_APPENDED_SIGN);
        return FALSE;
    }
    //进行参数解析 当参数个数不为零时或参数个数为零且可选参数大于零时 
    else if ((0 != recvParamCnts) || (optionalParamCnts > 0))
    {
        if ((PARSER_SINGLE_SCALE_PCPA == parserStructType) 
            || (PARSER_TAIL_SCALE_PCPA == parserStructType))
        {
            paramContainerInfo                                  = PARAM_CONTAINER_PTR_ARRAY;
        }
        return (_API_PEMParameterAnalyze((ptparamTotalContainer->m_pparamContainer), totalParamCnts,
                                         recvParamCnts, paramContainerInfo));    
    } 
    return TRUE;
}

/******************************************************************************
 *  函数名称 :                                                                
 *                                                                           
 *  函数功能 :                                                               
 *                                                                           
 *  入口参数 : 解析容器首地址                                                               
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
 *  附    注 : 参数解析起始地址:                                                                
 *                                                                            
 *                                                                            
******************************************************************************/

static uint32_t _API_PEMParameterAnalyze(ParamContainer *pparamContainer, uint8_t paramContainerCapacity, 
                                              uint8_t recvParamCnts, uint32_t paramContainerInfo)
{
    uint32_t       i                                       = 0;
    //固定从索引号为2的地址处存放  索引号0处：执行函数指针值  索引号1处：执行查询属性
    uint32_t       paramPutPosIndex                        = 2;
    uint32_t       paramLen, paramSpecialAttr;
    int8_t               *paccessAddr;
    uint32_t       paramTypeClass;
    ParamContainer    **ppparamContainer;
    
    //若参数容器指针指向的是指针数组类型 则对pparamContainer值进行修正
    if (PARAM_CONTAINER_PTR_ARRAY == paramContainerInfo)
    {
        ppparamContainer                                        = (ParamContainer **)pparamContainer;
        pparamContainer                                         = *ppparamContainer++;
    }

    for (; i < paramContainerCapacity; i++, paramPutPosIndex++)
    {
        //对所接收到参数进行解析
        if (i < recvParamCnts)
        {
            paccessAddr                                         = &t_ParserExeModuler.m_inputQueue[t_ParserExeModuler.m_parserPosIndex];
            paramLen                                            = strlen((void *)paccessAddr);
            //获取参数类型
            paramTypeClass                                      = _API_PEMParamTyleClassGet(paccessAddr, paramLen, pparamContainer, &paramSpecialAttr);
            //参数类型无法识别 返回错误
            if (PARAM_TYPE_NONE == paramTypeClass)
            {
                break;
            }
            //解析地址索引移动到下一个位置处 加1是因为参数与参数之间有分隔符','
            t_ParserExeModuler.m_parserPosIndex                    += paramLen + 1;
        }
        //对可省略参数进行处理 若接收到可省略参数 则后面的参数全部省略 如AA:BB [param_1, [param_2, [param_3]]]
        else
        {
            //访问地址设置为空
            paccessAddr                                         = NULL;
            //参数长度固定赋值为零
            paramLen                                            = 0;
            //参数特殊属性
            paramSpecialAttr                                    = PARAM_SPECIAL_ATTR_DEF;
            //获取参数类型
            paramTypeClass                                      = PARAM_TYPE_CLASS_GET(pparamContainer->m_paramStructType);
        }
        //记录参数存放地址  t_ParserExeModuler.m_paramPutPosIndex的值调整发生在各类型参数合法性判断函数中
        t_ParserExeModuler.m_ppparamStrAddrArray[paramPutPosIndex] = (const uint8_t *)PARAM_QUEUE_CURR_POS_GET();
        
        //参数合法性检测  先由参数类型转换为参数类型索引号
        paramTypeClass                                        >>= 4;
        if (FALSE == (*pf_API_PEMParamValidChk[paramTypeClass])((uint8_t *)paccessAddr, pparamContainer, paramSpecialAttr))
        {
            break;
        }
        //参数容器指针向后移动
        if (PARAM_CONTAINER_NORMAL == paramContainerInfo)
        {
            pparamContainer++;
        }
        else
        {
            pparamContainer                                     = *ppparamContainer++;
        }
    }
    //检查错误信息
    if (TRUE != _API_PEMParserErrChk())
    {
        return FALSE;
    }
    return TRUE;
}

/******************************************************************************
 *  函数名称 :                                                                
 *                                                                           
 *  函数功能 : 分析接收到的参数类型                                                              
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
 *  附    注 : 本函数只对参数类型进行分析 不多参数错误进行判断                                                           
 *                                                                            
 *                                                                            
******************************************************************************/

static  ParamTypeInfo _API_PEMParamTyleClassGet(int8_t *paccessAddr, uint32_t paramLen, 
                                                ParamContainer *pparamContainer,
                                                uint32_t *pparamSpecialAttr)
{
    uint32_t       i                                       = 0;
    //参数结构体类型
    ParamTypeInfo       paramStructTypeClass                    = PARAM_TYPE_CLASS_GET(pparamContainer->m_paramStructType);
    //引号出现的次数
    uint32_t       quoteSignAppearCnts                     = 0;
    //小数点出现的次数
    uint32_t       dotSignAppearCnts                       = 0;
    //当前正在分析的字符
    uint32_t       analyseChar                             = 0;
    //正负符号出现次数
    //uint32_t       signAppearCnts                          = 0;    
    //数字出现次数
    uint32_t       numbAppearCnts                          = 0; 
    //字符出现次数
    uint32_t       charAppearCnts                          = 0; 
    //空格符出现次数
    uint32_t       spaceAppearCnts                         = 0; 
    //字符e出现次数
    uint32_t       charAppearCnts_e                        = 0; 
    //字符E出现次数
    uint32_t       charAppearCnts_E                        = 0;
    //空格符出现位置 字符出现起始位置 数字、小数点出现终止位置
    uint32_t       spaceStartAppearPos = 0, charStartAppearPos = 0, numbDotEndAppearPos = 0;
    //符号出现位置
    //uint32_t       signStartAppearPos                      = 0;
    ParamTypeInfo        paramTypeClass                          = PARAM_TYPE_NONE, paramTypeClassTmp;
    
    //for (; i < paramLen; i++, paccessAddr++)
    for (; i < paramLen; i++)
    {
        analyseChar                                             = (*(paccessAddr + i));
        //引号字符
        if ('"' == analyseChar)
        {
            quoteSignAppearCnts++;
        }
        //小数点                        
        else if ('.' == analyseChar)
        {
            dotSignAppearCnts++; 
            numbDotEndAppearPos                                 = i;
        }
        /* SCPI程序对符号不予以支持
        //正负号                     
        else if (('+' == analyseChar) || ('-' == analyseChar))
        {
            signAppearCnts++;
            numbDotEndAppearPos                                 = i;
            //符号出现位置
            signStartAppearPos                                  = i;
        }
        */
        //数字
        else if ((analyseChar >= '0') && (analyseChar <= '9'))
        {
            numbAppearCnts++;
            numbDotEndAppearPos                                 = i;
        }
        //空格符
        else if (' ' == analyseChar)
        {
            spaceAppearCnts++;
            //记录起始位置
            if (1 == spaceAppearCnts)
            {
                spaceStartAppearPos                             = i;
            }
        }
        //其他情况 认为是字符
        else
        {
            if ('e' == analyseChar)
            {
                charAppearCnts_e++;
            }
            else if ('E' == analyseChar)
            {
                charAppearCnts_E++;
            }
            charAppearCnts++;
            //记录起始位置
            if (1 == charAppearCnts)
            {
                charStartAppearPos                              = i;
            }
        }
    }
    //若双引号字符出现 则为串类型  本程序中 规定双引号字符在此段参数下 只能出现两次
    if (quoteSignAppearCnts > 0)
    {
        if (2 == quoteSignAppearCnts)
        {
            paramTypeClass                                      = PARAM_TYPE_STRING;
        }
    }
    //浮点数 实数的认定是由是否包含小数点或空格符来认定的 目前不支持浮点数
    //1 kV  也可被识别为实数
    else if ((dotSignAppearCnts > 0) || (spaceAppearCnts > 0))
    {
        //判断小数点的个数是否为1 或 小写字符e出现次数大于零 或 大写E出现次数大于1 必须有数字出现
        if ((charAppearCnts_e > 0) || (charAppearCnts_E > 1) || (0 == numbAppearCnts) || (spaceAppearCnts > 1))
        {
            //格式错误
        }
        //若包含了 E 或 e 则参数为浮点数
        else if ((1 == charAppearCnts_E) || (1 == charAppearCnts_e))
        {
            //目前不支持浮点数类型
            //paramTypeClass                                    = PARAM_TYPE_FLOAT;
        }
        //参数为实数
        else
        {
            //对小数点 数字 空格符 字符的位置进行判断 
            if (((charStartAppearPos > spaceStartAppearPos) && (numbDotEndAppearPos < spaceStartAppearPos)
            //格式 -> 0.123 kV  1 kV
                    && ((1 == spaceAppearCnts) && (charAppearCnts > 0)))
            //格式 -> 0.123 
                || ((spaceAppearCnts == 0) && (charAppearCnts == 0) && (1 == dotSignAppearCnts)))
            {
                paramTypeClass                                  = PARAM_TYPE_REAL;
            }
            //不支持的类型: 0x123kV   0.123（有空格）
        }
        /*
        //判断小数点的个数是否为1 或 小写字符e出现次数大于零 或 大写E出现次数大于1 必须有数字出现 小数点必须出现
        if ((charAppearCnts_e > 0) || (charAppearCnts_E > 1) || (0 == numbAppearCnts) || (spaceAppearCnts > 1)
            || (1 != dotSignAppearCnts))
        {
            //格式错误
        }
        //若包含了 E  则参数为浮点数
        else if (1 == charAppearCnts_E)
        {
            //目前不支持浮点数类型
            //paramTypeClass                                    = PARAM_TYPE_FLOAT;
        }
        //参数为实数
        else
        {
            //对小数点 数字 空格符 字符的位置进行判断  格式不能太随意 不然会增加解析程序的复杂程度
            //有符号情况 格式 -> 0.123 kV  
            if (1 == spaceAppearCnts)
            {
                if ((charStartAppearPos > spaceStartAppearPos) && (numbDotEndAppearPos < spaceStartAppearPos)
                    && (charAppearCnts > 0))
                {
                    paramTypeClass                              = PARAM_TYPE_REAL;
                }
            }
            //无符号情况 格式 -> 0.123 
            else if ((charAppearCnts == 0))
            {
                paramTypeClass                                  = PARAM_TYPE_REAL;
            }
            //不支持的类型: 0x123kV   0.123（有空格） 1 kV
        }
        */
    }
    //字符型数据、整数类型不能包含小数点、空格符    此为硬性规定
    //字符型的认定是看接收到的字符数量是否为零来进行判定的 本程序规定 字符型类型不包含正负号 +、-
    //字符型数据必须包含字符 
    else if (charAppearCnts > 0)
    {
        paramTypeClass                                          = PARAM_TYPE_CHARACTER;
    }
    //整数类型  返回PARAM_TYPE_INTEGER类型 不对具体的整数类型做区分
    else
    {
        paramTypeClass                                          = PARAM_TYPE_INTEGER;
    }

    //初始化参数特殊属性
    (*pparamSpecialAttr)                                        = 0;
    //判断分析出来的参数类型是否与参数容器中指定的参数类型相匹配  此处是判断参数类型是否一致
    if (paramStructTypeClass != paramTypeClass)
    {
        paramTypeClassTmp                                       = paramTypeClass;
        paramTypeClass                                          = PARAM_TYPE_NONE;
        //再次判断下参数类型是否为字符型 且 参数容器中类型为整型、实型、浮点型
        if ((PARAM_TYPE_CHARACTER == paramTypeClassTmp) && ((PARAM_TYPE_INTEGER == paramStructTypeClass) 
            || (PARAM_TYPE_REAL == paramStructTypeClass) || (PARAM_TYPE_FLOAT == paramStructTypeClass)))
        {                                                           
            //判断接收到的字符串是否为具有特殊属性的字符串
            //目前特殊属性只支持 最大值 最小值 默认值 三种  对于一般而言已经足够了
            if ((0 == strcmp((void *)"MINimum", (void *)paccessAddr)) || (0 == strcmp((void *)"MIN", (void *)paccessAddr)))
            {
                (*pparamSpecialAttr)                           |= PARAM_SPECIAL_ATTR_MIN;
            }
            else if ((0 == strcmp((void *)"MAXimum", (void *)paccessAddr)) || (0 == strcmp((void *)"MAX", (void *)paccessAddr)))
            {
                (*pparamSpecialAttr)                           |= PARAM_SPECIAL_ATTR_MAX;
            }
            else if ((0 == strcmp((void *)"DEFault", (void *)paccessAddr)) || (0 == strcmp((void *)"DEF", (void *)paccessAddr)))
            {
                (*pparamSpecialAttr)                           |= PARAM_SPECIAL_ATTR_DEF;
            }
            //重新定义参数类型 是指参数的类型
            paramTypeClass                                      = paramStructTypeClass;
        }
        //分析出的类型为整形 但参数容器中类型为字符型
        else if ((PARAM_TYPE_INTEGER == paramTypeClassTmp) && (PARAM_TYPE_CHARACTER == paramStructTypeClass))
        {
            //重新定义参数类型 是指参数的类型
            paramTypeClass                                      = paramStructTypeClass;
        }
        //分析出的类型为整形 但参数容器中类型为实数型
        else if ((PARAM_TYPE_INTEGER == paramTypeClassTmp) && (PARAM_TYPE_REAL == paramStructTypeClass))
        {
            //重新定义参数类型 是指参数的类型
            paramTypeClass                                      = paramStructTypeClass;
        }
    }
    
    //判断有误解析到合法的参数类型
    if (PARAM_TYPE_NONE == paramTypeClass)
    {
        t_ParserExeModuler.m_parserErrCode                      = STATUS_PARAM_TYPE_ERROR;
    }
    return paramTypeClass;
}

/******************************************************************************
 *  函数名称 :                                                                
 *                                                                           
 *  函数功能 : 整形参数合法性判断                                                            
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

uint32_t _API_PEMParamDefSpecialChk(uint32_t paramSpecialAttr)
{
    //判断参数属性是由含有特殊属性->默认值
    if (paramSpecialAttr & PARAM_SPECIAL_ATTR_DEF)
    {
        //向参数队列中写入"DEF"
        _API_PEMParamQueueStrWrite((const uint8_t *)"DEF", strlen((void *)"DEF"));
        return TRUE;
    }
    return FALSE;
}

static  uint32_t _API_PEMIntegerParamValidChk(uint8_t *rscAddr, ParamContainer *pparamContainer, uint32_t paramSpecialAttr)
{
    uint32_t              upperLimit, lowerLimit;
    uint32_t       paramStructType;
    void               *pparamStructAddr;
    int32_t               recvValue;
    uint8_t               paramStr[11] = {0};

    //判断参数属性是由含有特殊属性->默认值
    if (TRUE == _API_PEMParamDefSpecialChk(paramSpecialAttr))
    {
        return TRUE;
    }
    paramStructType                                             = pparamContainer->m_paramStructType;
    pparamStructAddr                                            = pparamContainer->m_pparamStructAddr;
    switch (paramStructType)
    {
        case PARAM_TYPE_UINT8:
        case PARAM_TYPE_INT8:
                
            upperLimit                                          = ((ParamUint8Struct *)pparamStructAddr)->m_upperLimit;
            lowerLimit                                          = ((ParamUint8Struct *)pparamStructAddr)->m_lowerLimit;
            break;

        case PARAM_TYPE_UINT16:
        case PARAM_TYPE_INT16:
            
            upperLimit                                          = ((ParamUint16Struct *)pparamStructAddr)->m_upperLimit;
            lowerLimit                                          = ((ParamUint16Struct *)pparamStructAddr)->m_lowerLimit;
            break;

        case PARAM_TYPE_UINT32:
        case PARAM_TYPE_INT32:
            
            upperLimit                                          = ((ParamUint32Struct *)pparamStructAddr)->m_upperLimit;
            lowerLimit                                          = ((ParamUint32Struct *)pparamStructAddr)->m_lowerLimit;
            break;

        default:
            break;
    }
    //判断参数属性是由含有特殊属性->最大值
    if (paramSpecialAttr & PARAM_SPECIAL_ATTR_MAX)
    {
        recvValue                                               = upperLimit;    
    }
    //判断参数属性是由含有特殊属性->最小值
    else if (paramSpecialAttr & PARAM_SPECIAL_ATTR_MIN)
    {
        recvValue                                               = lowerLimit; 
    }
    //不具有特殊属性         
    else
    {
        recvValue                                               = atoi((void *)rscAddr);
        //无符号判定
        if ((PARAM_TYPE_UINT8 == paramStructType) || (PARAM_TYPE_UINT16 == paramStructType)
            || (PARAM_TYPE_UINT32 == paramStructType))
        {
            if ((recvValue > upperLimit) || (recvValue < lowerLimit))
            {
                t_ParserExeModuler.m_parserErrCode              = STATUS_DATA_OUT_OF_RANGE;
            }    
        }
        //有符号判定
        else
        {
            if (((int32_t)recvValue > (int32_t)upperLimit) || ((int32_t)recvValue < (int32_t)lowerLimit))
            {
                t_ParserExeModuler.m_parserErrCode              = STATUS_DATA_OUT_OF_RANGE;
            }
        }
    
        if (STATUS_NO_ERROR != t_ParserExeModuler.m_parserErrCode)
        {
            return FALSE;    
        }
    }
    LIB_ConvertNmubToString(recvValue, LIB_DataBitLenGet(recvValue), paramStr);
    _API_PEMParamQueueStrWrite(paramStr, strlen((void *)paramStr));
    return TRUE;
}

/******************************************************************************
 *  函数名称 :                                                                
 *                                                                           
 *  函数功能 : 字符型参数合法性判断                                                            
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
 *  附    注 : 字符型参数具有可省略属性 但不具有PARAM_SPECIAL_ATTR_MAX PARAM_SPECIAL_ATTR_MIN属性                                                          
 *                                                                            
 *                                                                            
******************************************************************************/

static  uint32_t _API_PEMCharacterParamValidChk(uint8_t *rscAddr, ParamContainer *pparamContainer, uint32_t paramSpecialAttr)
{
    uint32_t           i                                   = 0;
    ParamCharacterStruct   *pparamStructAddr;
    const CharacterUnit    *pcharacterUnitAddr;
    uint32_t           characterTableNumbs;

    //判断参数属性是由含有特殊属性->默认值
    if (TRUE == _API_PEMParamDefSpecialChk(paramSpecialAttr))
    {
        return TRUE;
    }
    pparamStructAddr                                            = (ParamCharacterStruct *)pparamContainer->m_pparamStructAddr;
    pcharacterUnitAddr                                          = pparamStructAddr->m_pcharacterTable;
    characterTableNumbs                                         = pparamStructAddr->m_characterTableNumbs;
    //对接收到的字符做合法性判断
    for (; i < characterTableNumbs; i++, pcharacterUnitAddr++)
    {
        if (0 == strcmp((void *)rscAddr, (void *)(pcharacterUnitAddr->m_plabelStr)))
        {
            break;
        }
    }
    if (characterTableNumbs == i)
    {
        //接收到字符不合法  错误类型定义为参数不允许
        t_ParserExeModuler.m_parserErrCode                      = STATUS_PARAM_NOT_ALLOWED;
    }
    else
    {
        //获取字符所对应的值并写入参数队列中 值为uint8_t类型
        PARAM_QUEUE_CHAR_WRITE(pcharacterUnitAddr->m_value);
        //写入分隔符'\0'
        PARAM_QUEUE_SPLIT_SIGN_WRITE();
    }

    if (STATUS_NO_ERROR != t_ParserExeModuler.m_parserErrCode)
    {
        return FALSE;    
    }
    return TRUE;
}

/******************************************************************************
 *  函数名称 :                                                                
 *                                                                           
 *  函数功能 : 串型参数合法性判断                                                            
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
 *  附    注 : 串类型是指以两个双引号为开始和结束标识的类型 如"ABC"                                                           
 *                                                                            
 *                                                                            
******************************************************************************/

static  uint32_t _API_PEMStringParamValidChk(uint8_t *rscAddr, ParamContainer *pparamContainer, uint32_t paramSpecialAttr)
{
    uint32_t           i                                   = 0;
    //获取接收字符串长度 等于 总长度-1 去除尾部双引号
    uint32_t           recvStrLen                          = 0;
    const uint8_t            *pnoValidStr;
    uint32_t           noValidStrLen;

    //判断参数属性是由含有特殊属性->默认值
    if (TRUE == _API_PEMParamDefSpecialChk(paramSpecialAttr))
    {
        return TRUE;
    }
    pnoValidStr                                                 = ((ParamStringStruct *)pparamContainer->m_pparamStructAddr)->m_pinvalidStr;
    noValidStrLen                                               = strlen((void *)pnoValidStr);
    //对接收到的串做合法性判断
    //指针值向下调整 略过起始双引号
    rscAddr++;
    recvStrLen                                                  = strlen((void *)rscAddr) - 1;
    //扫描整个非法字符串 判断所接收到的参数中是否存在非法字符
    for (; i < noValidStrLen; i++)
    {
        if (-1 != LIB_Strnpos(rscAddr, pnoValidStr[i], recvStrLen))
        {
            //找到非法值
            t_ParserExeModuler.m_parserErrCode                  = STATUS_INVALID_STRING_DATA;
            break;
        }
    }

    if (STATUS_NO_ERROR != t_ParserExeModuler.m_parserErrCode)
    {
        return FALSE;    
    }
    //向参数队列中写入串类型值 
    _API_PEMParamQueueStrWrite((const uint8_t *)rscAddr, recvStrLen);

    return TRUE;
}

/******************************************************************************
 *  函数名称 :                                                                
 *                                                                           
 *  函数功能 : 实型参数合法性判断                                                            
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
 *  附    注 : 对于实数而言 本解析程序硬性规定：必须出现小数点 接收到的长度必须与配置的长度相匹配                                                          
 *                                                                            
 *                                                                             
******************************************************************************/

static void _API_PEMRealParamConfigInfoGet(void *pparamStructAddr, ParamRealUint32Struct *pparamConfigStruct, 
                                           ParamTypeInfo paramStructType, uint32_t index)
{
    ParamRealUint16Struct   *pparamRealUint16Struct;
    ParamRealUint32Struct   *pparamRealUint32Struct;
    
    if (PARAM_TYPE_REAL_UINT16 == paramStructType)
    {
        pparamRealUint16Struct                      = (ParamRealUint16Struct *)pparamStructAddr;
        pparamRealUint16Struct                     += index;
        pparamConfigStruct->m_upperLimit            = pparamRealUint16Struct->m_upperLimit;
        pparamConfigStruct->m_lowerLimit            = pparamRealUint16Struct->m_lowerLimit;
        pparamConfigStruct->m_configInfo			= pparamRealUint16Struct->m_configInfo;
        pparamConfigStruct->m_signSymbolIndex		= pparamRealUint16Struct->m_signSymbolIndex;
    }
    else if (PARAM_TYPE_REAL_UINT32 == paramStructType)
    {
        pparamRealUint32Struct                      = (ParamRealUint32Struct *)pparamStructAddr;
        pparamRealUint32Struct                     += index;
        pparamConfigStruct->m_upperLimit            = pparamRealUint32Struct->m_upperLimit;
        pparamConfigStruct->m_lowerLimit            = pparamRealUint32Struct->m_lowerLimit;
        pparamConfigStruct->m_configInfo			= pparamRealUint32Struct->m_configInfo;
        pparamConfigStruct->m_signSymbolIndex		= pparamRealUint32Struct->m_signSymbolIndex;
    }
}

// static uint32_t _API_PEMRealSignBitLenGet(uint32_t signSymbolIndex)
// {
//     //同一组符号量化位长度之间间隔3
//     return SIGN_INTERVAL_BIT*SIGN_SYMBOL_INDEX_GET((st_SignSymbolTable[signSymbolIndex]).m_relativeIndex);
// }

static uint32_t _API_PEMRealSignBitLenGet(uint32_t signSymbolIndex)
{
	if(signSymbolIndex!=SIGN_SYMBOL_INDEX_NONE)
    //同一组符号量化位长度之间间隔3
    return SIGN_INTERVAL_BIT*SIGN_SYMBOL_INDEX_GET((st_SignSymbolTable[signSymbolIndex]).m_relativeIndex);
	else
		return SIGN_SYMBOL_INDEX_NONE;
}

static uint32_t _API_PEMRealParamSectorBitLenInfoGet(ParamRealUint32Struct *pparamSectorStruct, 
                                                           ParamRealSectorInfoStruct *pparamRealSectorInfoStruct)
{
    uint32_t      dotBitLenValue                  = REAL_CFG_INFO_DOT_POS_GET(pparamSectorStruct->m_configInfo);
    uint32_t      configBitLenValue	            = REAL_CFG_INFO_STR_LEN_GET(pparamSectorStruct->m_configInfo);
    uint32_t      signBitLenValue	                = _API_PEMRealSignBitLenGet(pparamSectorStruct->m_signSymbolIndex);
    
    if (NULL != pparamRealSectorInfoStruct)
    {
        //若小数点配置为无 则小数点量化位长度与配置字符串长度相同
        if (REAL_CFG_INFO_NO_DOT == dotBitLenValue)
        {
            dotBitLenValue                              = configBitLenValue;
        }
        //小数点量化位长度值
        pparamRealSectorInfoStruct->m_dotBitLen         = dotBitLenValue;
        if (SIGN_SYMBOL_INDEX_NONE == signBitLenValue)
        {
            signBitLenValue                             = 0;  
        }
        //单位符号量化位长度值
        pparamRealSectorInfoStruct->m_signBitLen        = signBitLenValue;
        //配置字符串长度量化位长度值
        pparamRealSectorInfoStruct->m_configBitLen      = configBitLenValue;
        //下限量化位长度值
        pparamRealSectorInfoStruct->m_lowerLimitBitLen  = LIB_DataBitLenGet(pparamSectorStruct->m_lowerLimit) 
                                                          + dotBitLenValue + signBitLenValue;
        //上限量化位长度值
        pparamRealSectorInfoStruct->m_upperLimitBitLen  = LIB_DataBitLenGet(pparamSectorStruct->m_upperLimit) 
                                                          + dotBitLenValue + signBitLenValue;
    }
    return TRUE;
}

static  uint32_t _API_PEMRealParamValidChk(uint8_t *rscAddr, ParamContainer *pparamContainer, uint32_t paramSpecialAttr)
{
    ParamRealUint32Struct t_paramRealStruct;
    ParamRealSectorInfoStruct t_paramRealSectorInfoStruct;
    //参数结构体类型
    ParamTypeInfo         paramStructType                       = pparamContainer->m_paramStructType;
    void                 *pparamStructAddr                      = pparamContainer->m_pparamStructAddr;  
    uint8_t                *pparamQueuePos                        = (uint8_t *)(PARAM_QUEUE_CURR_POS_GET());
    //获取实数参数段配置个数信息
    uint32_t        realParamInfoSectionCnts              = REAL_PARAM_INFO_SECTION_CNTS_GET(pparamContainer->m_paramStructInfo);
    uint32_t        recvDataBitLen, configDataBitLen, signSymbolIndex;
    uint32_t        recvValidStrLen, recvSignBitLen, recvTotalBitLen, recvNeedModifyBitLen;
    uint32_t        sectorDotSignBitLen;
    uint32_t        i, j;
    //空格符、符号单位、小数点符号位置 初始化为未找到
    int8_t         spaceSignPos, unitSignPos, recvDotBitLen;
    //放大或缩小位长度
    int8_t         zoomOutInBitLen;
    int32_t                 recvValue;

    //判断参数属性是否含有特殊属性->默认值
    if (TRUE == _API_PEMParamDefSpecialChk(paramSpecialAttr))
    {
        return TRUE;
    }
    //获取下限配置段中下限值、小数点、符号索引、字符串长度位长信息 此处为了正确的获得字符串量化位长度值
    _API_PEMRealParamConfigInfoGet(pparamStructAddr, &t_paramRealStruct, paramStructType, 0);
    //程序规定 实数的多个配置段之间的字符串长度必须相同 对于应用来讲 一般可以满足此情况 
    configDataBitLen                                            = REAL_CFG_INFO_STR_LEN_GET(t_paramRealStruct.m_configInfo);;
    //初始化接收字符串中符号位长度为零
    recvSignBitLen                                              = 0;
    //---------------------------------------------------------------------------------------------------
    
    //判断参数属性是否含有特殊属性->最大值
    if (paramSpecialAttr & PARAM_SPECIAL_ATTR_MAX)      
    {
        //获取上限配置段中上限值、小数点、符号索引、字符串长度位长信息
        _API_PEMRealParamConfigInfoGet(pparamStructAddr, &t_paramRealStruct, paramStructType, realParamInfoSectionCnts - 1);
        _API_PEMRealParamSectorBitLenInfoGet(&t_paramRealStruct, &t_paramRealSectorInfoStruct);
        recvValue                                               = t_paramRealStruct.m_upperLimit;                                     
    }
    //判断参数属性是否含有特殊属性->最小值
    else if (paramSpecialAttr & PARAM_SPECIAL_ATTR_MIN)
    {
        _API_PEMRealParamSectorBitLenInfoGet(&t_paramRealStruct, &t_paramRealSectorInfoStruct);
        recvValue                                               = t_paramRealStruct.m_lowerLimit;
    }
    //参数不具有特殊属性 进入正常流程判断
    else
    {
        //----------------------第一步  符号合法性判断  只接收已经配置的符号-----------------------------

        //获得符号与数字之间的分隔符 -> 空格符 进而找到符号所在位置
        spaceSignPos                                            = LIB_Strpos((void *)rscAddr, ' ');
        //根据是否接收到空格符 进行相应判断
        
        //判断是否找到空格符 若找到 则代表后面一定有符号字符 -> 进行符号判断
        if (-1 != spaceSignPos)
        {
            //空格处写入结束符
            rscAddr[spaceSignPos]                               = '\0';
            unitSignPos                                         = spaceSignPos + 1;
        }
        //遍历配置段区域信息
        for (i = 0; i < realParamInfoSectionCnts; i++)
        {
            _API_PEMRealParamConfigInfoGet(pparamStructAddr, &t_paramRealStruct, paramStructType, i);
            //接收到空格符 则代表后面一定有符号字符 -> 进行符号判断
            if (-1 != spaceSignPos)
            {
                signSymbolIndex                                 = t_paramRealStruct.m_signSymbolIndex;
                //从单位符号表中找匹配的单位符号 找到的索引为：signSymbolIndex
                if (0 == strcmp((void *)(st_SignSymbolTable[signSymbolIndex]).m_psignStr, (void *)&rscAddr[unitSignPos]))
                {
                    //记录接收字符串单位的量化值                                   
                    recvSignBitLen                              = _API_PEMRealSignBitLenGet(signSymbolIndex);
                    break;
                }
            }
            //未接收到空格符 -> 判断配置内是否有符号配置
            else
            {
                if (t_paramRealStruct.m_signSymbolIndex != SIGN_SYMBOL_INDEX_NONE)
                {
                    break;
                }
            }
        }
        //(接收到空格符时 且 找不到匹配的单位符号) 或 (未接收到空格符时 且 配置段区域内配置了符号)
        if ((-1 != spaceSignPos) && (i == realParamInfoSectionCnts)
            || ((-1 == spaceSignPos) && (i != realParamInfoSectionCnts)))
        {
            //参数所携带的单位符号错误
            t_ParserExeModuler.m_parserErrCode              = STATUS_PARAM_TYPE_ERROR;
            return FALSE;
        }
        //----------------------第二步  去除接收字符的前导零字符-----------------------------------------
        //例如： 000999.9 M -> 999.9 M 
        //注意在此步中 一定不能让接收到的字符串扩展 不然有可能会冲毁掉后面接收的参数数据
        //通过调整rscAddr指针来达到去除前导零的效果
        
        //此步程序运行之后的结果为 000000.9 -> .9
        while ('0' == (*rscAddr))
        {
            rscAddr++;
        }
        //----------------------第三步  截断字符串-------------------------------------------------------
        //注意在此步中 一定不能让接收到的字符串扩展 不然有可能会冲毁掉后面接收的参数数据
        //此处按照配置信息中的长度

        //获取此刻的字符串小数点位置 和 字符串长度
        recvDotBitLen                                           = LIB_Strpos((void *)rscAddr, '.');
        recvValidStrLen                                         = strlen((void *)rscAddr);
        j                                                       = configDataBitLen;
        //根据小数点位置对有效字符串长度进行修正 小数点存在且位置不为零时
        //存在小数点且小数点不在首位时
        if (recvDotBitLen > 0)
        {
            //存在小数点且小数点在配置字符串长度以内 则字符串有效长度加一 包含小数点长度
            if (recvDotBitLen < j)
            {
                j++;
            }
            //小数点在配置字符串长度外面 则认为此小数点没有用途 省略掉
            else
            {
                recvDotBitLen                                   = -1;
            }
        }
        //判断是否需要对字符串进行截断
        if (recvValidStrLen > j)
        {
            rscAddr[j]                                          = '\0';
            //接收字符串的长度修正为配置长度
            recvValidStrLen                                     = j;
        }
        //----------------------第四步  计算接收到的字符串的总量化位长度值-------------------------------
        //总量化位长度=小数点量化位长度 + 接收值量化位长度 + 单位量化位长度 + 需要修补量化位长度
        if (-1 != recvDotBitLen)  
        {
            if (recvDotBitLen < j)
            {
                //将小数点后的字符数据左移
                LIB_StringLsl((uint8_t *)&rscAddr[recvDotBitLen], recvValidStrLen-recvDotBitLen, 1);
                //存在小数点 则长度值减一 此长度为除去小数点之后的长度
                recvValidStrLen--;
            }
        }
        else
        {
            //未接收到小数点 则小数点长度为字符串长度
            recvDotBitLen                                       = recvValidStrLen;
        }
        //计算去除小数点之后字符串的整型值
        recvValue                                               = atoi((void *)rscAddr);
        //计算接收到字符串的值位长度 此位长度为接收到字符串的有效个数 如：.01 位长度为二 .001 位长度为三
        recvDataBitLen                                          = LIB_DataBitLenGet(recvValue);
        //计算需要修正的位长度                                                           
        recvNeedModifyBitLen                                    = configDataBitLen - recvValidStrLen;
        //计算接收到字符串的总量化值
        recvTotalBitLen                                         = recvDotBitLen + recvDataBitLen
                                                                  + recvSignBitLen + recvNeedModifyBitLen;
        //----------------------第五步  根据字符串的总量化位长度需找一个匹配的段配置区域-----------------
        //通过从底向上的遍历来查找一个匹配的配置区域
        //遍历整个段配置区域
        for (i = 0; i < realParamInfoSectionCnts; i++)
        {
            _API_PEMRealParamConfigInfoGet(pparamStructAddr, &t_paramRealStruct, paramStructType, i);
            //计算段配置区域的位长度范围
            _API_PEMRealParamSectorBitLenInfoGet(&t_paramRealStruct, &t_paramRealSectorInfoStruct);
            //若接收的值为零 则认为是一个特例 直接定位到配置段区域中的最低端
            if (0 == recvValue)
            {
                break;
            }
            //在最低段配置区域内 判断是否低于下限 若低于下限 则提示数据范围出错
            if ((recvTotalBitLen < t_paramRealSectorInfoStruct.m_lowerLimitBitLen) && (0 == i))
            {
                t_ParserExeModuler.m_parserErrCode  = STATUS_DATA_OUT_OF_RANGE;
                return FALSE;
            }
            if (recvTotalBitLen <= t_paramRealSectorInfoStruct.m_upperLimitBitLen)
            {
                break;
            }
        }
        //判断是否查找到合适的段配置区域
        if (i == realParamInfoSectionCnts)
        {
            //未找到 则提示数据范围出错
            t_ParserExeModuler.m_parserErrCode                  = STATUS_DATA_OUT_OF_RANGE;
            return FALSE;
        }
        //----------------------第六步  根据找到的段配置区域 对接收到的字符串值进行放大或缩小------------
        //接收到的值放大或缩小后 要根据得到的值与所找的段配置区域进行上限判断 查看是否有继续上翻的可能    
        //计算放大或缩小位长度 = 需要修正的位长度 + 单位量化位长度 + 小数点量化位长度 -
        //                       (目的段配置区域单位量化位长度 + 目的段配置区域小数点量化位长度)
        zoomOutInBitLen  = recvNeedModifyBitLen + recvSignBitLen + recvDotBitLen
                           - (t_paramRealSectorInfoStruct.m_signBitLen + t_paramRealSectorInfoStruct.m_dotBitLen);
        if (zoomOutInBitLen < 0)
        {
            //除以相差值
            recvValue                                          /= LIB_Get10nData(abs(zoomOutInBitLen));
        }
        else
        {
            //乘以相差值
            recvValue                                          *= LIB_Get10nData(zoomOutInBitLen);
        }
        //接收到的值应该比所找到的配置段区域中的下限值要大
        if (recvValue < t_paramRealStruct.m_lowerLimit)
        {
            //未找到 则提示数据范围出错
            t_ParserExeModuler.m_parserErrCode                  = STATUS_DATA_OUT_OF_RANGE;
            return FALSE;
        }
        //判断有无上翻可能
        for (; i < realParamInfoSectionCnts; i++)
        {
            if (recvValue > t_paramRealStruct.m_upperLimit)
            {
                // 比最大值还大，超范围，2012.05.24
				if (i == (realParamInfoSectionCnts - 1))
		        {
		            //未找到 则提示数据范围出错
		            t_ParserExeModuler.m_parserErrCode          = STATUS_DATA_OUT_OF_RANGE;
		            return FALSE;
		        }

				//计算当前段配置区域中的符号量化位长度+小数点量化位长度
                sectorDotSignBitLen                             = t_paramRealSectorInfoStruct.m_signBitLen 
                                                                  + t_paramRealSectorInfoStruct.m_dotBitLen;
                //获取上翻后的段配置区域信息
                _API_PEMRealParamConfigInfoGet(pparamStructAddr, &t_paramRealStruct, paramStructType, i+1);
                //获取上翻后的段配置区域位长度范围
                _API_PEMRealParamSectorBitLenInfoGet(&t_paramRealStruct, &t_paramRealSectorInfoStruct);
                //计算需要缩小的值
                zoomOutInBitLen                                 = t_paramRealSectorInfoStruct.m_signBitLen 
                                                                  + t_paramRealSectorInfoStruct.m_dotBitLen
                                                                  - sectorDotSignBitLen;
                //计算新的字符串接收值
                recvValue                                      /= LIB_Get10nData(zoomOutInBitLen);
            }
            else
            {
                break;
            }
        }
        //判断是否查找到合适的段配置区域
        if (i == realParamInfoSectionCnts)
        {
            //未找到 则提示数据范围出错
            t_ParserExeModuler.m_parserErrCode                  = STATUS_DATA_OUT_OF_RANGE;
            return FALSE;
        }
    }
    //----------------------第七步  根据找到的段配置区域 对接收到的字符串值进行格式化--------------------
    //判断实型参数拆分自定义函数指针是否为空
    if (NULL == s_FP_pfRealParamSplit)
    {
        //标准的拆分过程 将数据拆分为字符串形式
        LIB_ConvertNmubToString(recvValue, t_paramRealSectorInfoStruct.m_configBitLen, pparamQueuePos);
        //判断是否需要插入小数点
        if (t_paramRealSectorInfoStruct.m_configBitLen != t_paramRealSectorInfoStruct.m_dotBitLen)
        {
            LIB_StrInsert(pparamQueuePos, t_paramRealSectorInfoStruct.m_dotBitLen, '.');
            t_paramRealSectorInfoStruct.m_configBitLen++;
        }
        //更新参数存放位置索引
        PARAM_QUEUE_CURR_POS_ADJ(t_paramRealSectorInfoStruct.m_configBitLen);
        //压入单位符号信息
        if (-1 != spaceSignPos)
        {
            pparamQueuePos  = (uint8_t *)((st_SignSymbolTable[t_paramRealStruct.m_signSymbolIndex]).m_psignStr);
            //写入空格符
            PARAM_QUEUE_CHAR_WRITE(' ');
            _API_PEMParamQueueStrWrite(pparamQueuePos, strlen((void *)pparamQueuePos));
        }
        else
        {
            //写入分隔符'\0'
            PARAM_QUEUE_SPLIT_SIGN_WRITE();
        }
    }        
    //调用自定义的实型参数拆分函数
    else
    {
        RealParamSplitStruct    t_RealParamSplitStruct;

        t_RealParamSplitStruct.m_pdestSplitAddr                 = pparamQueuePos;
        t_RealParamSplitStruct.m_rawDecValue                    = recvValue;
        t_RealParamSplitStruct.m_rawConfigLen                   = t_paramRealSectorInfoStruct.m_configBitLen;
        t_RealParamSplitStruct.m_rawDotValue                    = t_paramRealSectorInfoStruct.m_dotBitLen;
        t_RealParamSplitStruct.m_rawSignValue                   = st_SignSymbolTable[t_paramRealStruct.m_signSymbolIndex].m_relativeIndex;
        if (FALSE == s_FP_pfRealParamSplit(&t_RealParamSplitStruct))
        {
            return FALSE;
        }
    }

    return TRUE;
}

/******************************************************************************
 *  函数名称 :                                                                
 *                                                                           
 *  函数功能 : 浮点型参数合法性判断                                                            
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

static  uint32_t _API_PEMFloatParamValidChk(uint8_t *rscAddr, ParamContainer *pparamContainer, uint32_t paramSpecialAttr)
{
    
    return TRUE;
}

/******************************************************************************
 *  函数名称 :                                                                
 *                                                                           
 *  函数功能 : 参数队列写入字符串                                                              
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

static void _API_PEMParamQueueStrWrite(const uint8_t *prscStrAddr, uint8_t rscStrLen)
{
    //写入串类型值 
    strncpy((void *)(PARAM_QUEUE_CURR_POS_GET()), (void *)prscStrAddr, rscStrLen);
    //更新参数存放位置索引
    PARAM_QUEUE_CURR_POS_ADJ(rscStrLen);
    //写入分隔符'\0'
    PARAM_QUEUE_SPLIT_SIGN_WRITE();
}

void API_PEMParamQueueIntegerWrite(uint32_t rscValue, uint8_t len)
{
    LIB_ConvertNmubToString(rscValue, len, (uint8_t *)(PARAM_QUEUE_CURR_POS_GET()));
    //更新参数存放位置索引
    PARAM_QUEUE_CURR_POS_ADJ(len);
}

void API_PEMParamQueueCharWrite(uint8_t rscChar)
{
    PARAM_QUEUE_CHAR_WRITE(rscChar);
}

void API_PEMParamQueueSplitSignWrite(uint8_t rscChar)
{
    //写入分隔符'\0'
    PARAM_QUEUE_SPLIT_SIGN_WRITE();
}

void API_PEMParamQueuePosAdjust(uint16_t len)
{
    //更新参数存放位置索引
    PARAM_QUEUE_CURR_POS_ADJ(len);
}

uint8_t *API_PEMParamQueueCurrentPtrGet(void)
{
    return (uint8_t *)(PARAM_QUEUE_CURR_POS_GET());
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

void _API_PEMInputQueueFlush(void)
{
    //加入结束符 ; 的长度
    memset(t_ParserExeModuler.m_inputQueue, '\0', t_ParserExeModuler.m_inputQueueLen+1);
    t_ParserExeModuler.m_inputQueueLen                      = PARSER_START_POS;  
}

void _API_PEMOutputQueueFlush(void)
{
    //加入结束符 ; 的长度
    memset(t_ParserExeModuler.m_outputQueue, '\0', t_ParserExeModuler.m_outputQueueLen+1);
    t_ParserExeModuler.m_outputQueueLen                     = 0;  
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

static uint32_t _API_PEMOutputQueuePush(void *pvoid, portQUEUELEN_TYPE len, 
                                             uint32_t appendSign, uint32_t style)
{
    uint32_t   i                                       = (appendSign == NO_APPENDED_SIGN)?(0):(1);
    
    //判断范围
    if (((t_ParserExeModuler.m_outputQueueLen + len + i) > (QUEUE_MAX_LEN-1))
        && (!((2 == style) && ((uint32_t)pvoid == STATUS_OUTPUT_QUEUE_FULL))))
    {
        t_ParserExeModuler.m_parserErrCode                   = STATUS_OUTPUT_QUEUE_FULL;
        //信息添加失败
        return  FALSE;
    }
    //对字符串进行操作
    if (_OUTPUT_QUEUE_PUSH_STR == style)
    {
        memcpy((void *)&t_ParserExeModuler.m_outputQueue[t_ParserExeModuler.m_outputQueueLen], (void *)pvoid, len);
    }
    //对整形数操作
    else if (_OUTPUT_QUEUE_PUSH_INT == style)
    {
        LIB_ConvertNmubToString((uint32_t)pvoid, len, (void *)&t_ParserExeModuler.m_outputQueue[t_ParserExeModuler.m_outputQueueLen]);
    }
    //字符型操作
    else
    {
        t_ParserExeModuler.m_outputQueue[t_ParserExeModuler.m_outputQueueLen] = (uint32_t)pvoid;
    }
    t_ParserExeModuler.m_outputQueueLen                    += len;
    if (NO_APPENDED_SIGN != appendSign)
    {
        t_ParserExeModuler.m_outputQueue[t_ParserExeModuler.m_outputQueueLen++] = (int8_t)appendSign;
    }
    return FALSE;
}

uint32_t API_PEMOutputQueueErrMsgPush(ParserErrCode parserErrCode, uint32_t appendSign)
{
    return _API_PEMOutputQueuePush((void *)API_PEM_pParserExeModulerErrCodeArray[parserErrCode], 
        strlen((void *)API_PEM_pParserExeModulerErrCodeArray[parserErrCode]), appendSign, _OUTPUT_QUEUE_PUSH_STR);
}

uint32_t API_PEMExecuteErrCodeSet(ParserErrCode executeErrCode)
{
    if ((STATUS_EXECUTE_NOT_ALLOWED == executeErrCode) || (STATUS_PARAM_NOT_ALLOWED == executeErrCode)
        || (STATUS_EXECUTE_TIME_OUT == executeErrCode) || (STATUS_DATA_OUT_OF_RANGE == executeErrCode)
        || (STATUS_SUM_CHECK_ERROR  == executeErrCode))
    {
        t_ParserExeModuler.m_parserErrCode                      = executeErrCode;
        return TRUE;
    }
    return FALSE;
}

uint32_t API_PEMOutputQueueStrPush(const uint8_t *prscStr, uint32_t appendSign)
{
    return _API_PEMOutputQueuePush((void *)prscStr, strlen((void *)prscStr), appendSign, _OUTPUT_QUEUE_PUSH_STR);
}

uint32_t API_PEMOutputQueueStrnPush(const uint8_t *prscStr, uint32_t len, uint32_t appendSign)
{
    return _API_PEMOutputQueuePush((void *)prscStr, len, appendSign, _OUTPUT_QUEUE_PUSH_STR);
}

uint32_t API_PEMOutputQueueIntegerPush(uint32_t rscData, uint32_t len, uint32_t appendSign)
{
    return _API_PEMOutputQueuePush((void *)rscData, len, appendSign, _OUTPUT_QUEUE_PUSH_INT);
}  

uint32_t API_PEMOutputQueueCharPush(uint32_t appendChar, uint32_t appendSign)
{
    return _API_PEMOutputQueuePush((void *)appendChar, 1, appendSign, _OUTPUT_QUEUE_PUSH_CHAR);
}  

void int_API_PEMRead(uint8_t recvMsg)
{
	//解析器空闲时 则开始接收
    if (STATUS_IDLE == t_ParserExeModuler.m_parserStatus)
    {
        //系统解析器空闲 且收到命令串结束符 解析器开始进行命令解析 
        //出现命令结束符
        if (recvMsg == s_EndCode)
        {
    	    //若结束符为CR+LF时 要判断上一字符是否为CR 若不为CR 则LF无效
            if (CR_LF_INDEX == t_ParserExeModuler.m_endCodeIndex)
            {
                //判断有无接收到回车符CR
                if ('\r' == t_ParserExeModuler.m_inputQueue[t_ParserExeModuler.m_inputQueueLen - 1])
                {
                    //结束符正确 在CR的位置处写入LF
                    t_ParserExeModuler.m_inputQueueLen--;
                }
                else
                {
                    //结束符错误 保存接收到的字符内容
                    t_ParserExeModuler.m_inputQueue[t_ParserExeModuler.m_inputQueueLen] = recvMsg;
                    // 清除接受缓冲区
                    _API_PEMInputQueueFlush();
                    return;
                }
            }
            //添加字符串结束符
    	    t_ParserExeModuler.m_inputQueue[t_ParserExeModuler.m_inputQueueLen] = ';';
    	    //系统指令解析执行状态 处理中
    	    t_ParserExeModuler.m_parserStatus               = STATUS_PARSERING;
            //指令链默认从根指令解析容器开始解析
            t_ParserExeModuler.m_currParserContainerPtr     = t_ParserExeModuler.m_rootParserContainerPtr;
            t_ParserExeModuler.m_currParserStructCnts       = t_ParserExeModuler.m_rootParserStructCnts;
            //命令链数加一
            t_ParserExeModuler.m_totalInstructionChains++; 
        }
        //接收命令字符串以及参数
        else
        {
    	    t_ParserExeModuler.m_inputQueue[t_ParserExeModuler.m_inputQueueLen++] = recvMsg;
            //命令链个数增加判断
            if (';' == recvMsg)
            {
                //命令链数加一
                t_ParserExeModuler.m_totalInstructionChains++;  
            }
    	    if (t_ParserExeModuler.m_inputQueueLen >= QUEUE_MAX_LEN)
    	    {
    		    //减一  使之在t_ParserExeModuler.m_inputQueue范围内
                t_ParserExeModuler.m_inputQueueLen--;
                //在接收缓冲区的最后一个字节处放入字符串结束符
    		    t_ParserExeModuler.m_inputQueue[t_ParserExeModuler.m_inputQueueLen] = ';';
    		    //系统指令解析执行状态 处理中
    	        t_ParserExeModuler.m_parserStatus           = STATUS_PARSERING;
    	    }
    	}
    }
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

static portHASHCODE_TYPE _API_PEMStrHashCodeGet(const uint8_t *prscStr, uint32_t len)
{
    uint32_t              hashCode                            = HASH_INIT_CODE;
    portHASHCODE_TYPE   maxHashCode                         = (portHASHCODE_TYPE)-1;
	uint8_t               rscChar;
   
    while (len--)
	{
        rscChar											    = (*prscStr++);
        hashCode                                            = hashCode<<7;
		hashCode                                           += rscChar;
        hashCode                                           %= maxHashCode;
    }
    return (portHASHCODE_TYPE)hashCode;
}

/******************************************************************************
 *  函数名称 :                                                                
 *                                                                           
 *  函数功能 : 数据获取函数组                                                              
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
 *  附    注 : 数据以大端方式组织                                                               
 *                                                                            
 *                                                                            
******************************************************************************/

uint8_t API_PEMItegerParamReaduint8_t(const uint8_t *rscAddr)
{
    return (uint8_t)atoi((void *)rscAddr);
}

int8_t API_PEMItegerParamReadInt8(const uint8_t *rscAddr)
{
    return (int8_t)atoi((void *)rscAddr);
}

uint16_t API_PEMItegerParamReadUint16(const uint8_t *rscAddr)
{
    return (uint16_t)atoi((void *)rscAddr);
}

int16_t API_PEMItegerParamReadInt16(const uint8_t *rscAddr)
{
    return (int16_t)(API_PEMItegerParamReadUint16(rscAddr));
}

uint32_t API_PEMItegerParamReadUint32(const uint8_t *rscAddr)
{
    return (uint32_t)atoi((void *)rscAddr);
}

int32_t API_PEMItegerParamReadInt32(const uint8_t *rscAddr)
{
    return atoi((void *)rscAddr);
}

/******************************************************************************
 *  函数名称 :                                                                
 *                                                                           
 *  函数功能 :  预执行合法性检测函数安装 卸载函数                                                             
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

static uint32_t _API_PEMNullValidityChk(struct EXECUTE_VALIDITY_CHK_INFO *pvalidityChkInfo)
{
    return TRUE;
} 

void API_PEMPrevValidityChkFuncInstall(FP_pfValidityChk *pfprevExecuteValidityChk)
{
    s_FP_pfPrevValidityChk                                      = pfprevExecuteValidityChk;
}

void API_PEMPrevValidityChkFuncUninstall(void)
{
    s_FP_pfPrevValidityChk                                      = NULL;
}

void API_PEMRealParamSplitFuncInstall(FP_pfRealParamSplit *pfRealParamSplit)
{
    s_FP_pfRealParamSplit                                       = pfRealParamSplit;
}

void API_PEMRealParamSplitFuncUninstall(void)
{
    s_FP_pfRealParamSplit                                       = NULL;
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
/*
static uint32_t _API_PEMSegmentShortLenGet(uint32_t longLen)
{
    uint32_t   i                                       = t_ParserExeModuler.m_parserPosIndex;

    longLen                                                += i;                                    
    for (;i < longLen; i++)
    {
        if ((t_ParserExeModuler.m_inputQueue[i] >= 'A') && (t_ParserExeModuler.m_inputQueue[i] <= 'Z'))
        {
            continue;
        }
        else if ('*' == t_ParserExeModuler.m_inputQueue[i])
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return i - t_ParserExeModuler.m_parserPosIndex;
} 

static uint32_t _API_PEMSegmentLongLenGet(void)
{
    uint32_t   i                                       = t_ParserExeModuler.m_parserPosIndex;

    //在通讯指令解析过程中 '\0' 和 ';' 都可以做为指令结束符 
    while (('\0' != t_ParserExeModuler.m_inputQueue[i]) && (';' != t_ParserExeModuler.m_inputQueue[i]))
    {
        i++;
    }

    return i - t_ParserExeModuler.m_parserPosIndex;
}
*/

/******************************************************************************
 *  函数名称 :                                                                
 *                                                                           
 *  函数功能 : 实型数据拆分函数                                                             
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
static uint32_t _RealParamSplitServ(RealParamSplitStruct *prealParamSplitStruct)
{
    uint8_t               revParamMappingVal  = 0;
    uint8_t               dotMappingVal       = prealParamSplitStruct->m_rawDotValue;

    LIB_ConvertNmubToString(prealParamSplitStruct->m_rawDecValue, prealParamSplitStruct->m_rawConfigLen, (uint8_t *)(PARAM_QUEUE_CURR_POS_GET()));
    //更新参数存放位置索引
    PARAM_QUEUE_CURR_POS_ADJ(prealParamSplitStruct->m_rawConfigLen);
    //API_PEMParamQueueIntegerWrite(prealParamSplitStruct->m_rawDecValue, prealParamSplitStruct->m_rawConfigLen);
    API_PEMParamQueueCharWrite(' ');
    if (REAL_CFG_INFO_NO_DOT == dotMappingVal)
    {
        dotMappingVal                       = prealParamSplitStruct->m_rawConfigLen;
    }
    revParamMappingVal                      =   prealParamSplitStruct->m_rawConfigLen + dotMappingVal
                                              + (prealParamSplitStruct->m_rawSignValue & 0x07)*3;
    API_PEMParamQueueCharWrite(revParamMappingVal/10 + '0');
    API_PEMParamQueueCharWrite(revParamMappingVal%10 + '0');
    PARAM_QUEUE_CHAR_WRITE('\0');

    return TRUE;
}

/******************************************************************************
 *  函数名称 :                                                                
 *                                                                           
 *  函数功能 : 通讯帧校验和获取函数                                                              
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
 *  附    注 : 帧指令串中所有字符累加和|0x80                                                               
 *                                                                            
 *                                                                            
******************************************************************************/
#if (_FRAME_CHK_SUPPORT > 0)

static uint8_t _API_PEMFrameChkSumGet(const uint8_t *prscQueue, portQUEUELEN_TYPE len)
{
    portQUEUELEN_TYPE   i                                       = 0;
    uint8_t               chkSum                                = 0;
    
    for (; i < len; i++)
    {
        chkSum                                                 += prscQueue[i];
    }    
    return chkSum|0x80;
}
#endif
