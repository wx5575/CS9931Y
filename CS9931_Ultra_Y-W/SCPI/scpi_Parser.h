#include <stdint.h>
#ifndef    _SCPI_PARSER_H_
#define    _SCPI_PARSER_H_
#define                         QUEUE_MAX_LEN                          (100)


#if (QUEUE_MAX_LEN < 256)
    
    typedef                     unsigned char                          portQUEUELEN_TYPE; 
#elif (QUEUE_MAX_LEN < 65536)
    
    typedef                     unsigned short int                     portQUEUELEN_TYPE; 
#else
    
    typedef                     unsigned int                           portQUEUELEN_TYPE;  
#endif                  


// #define     		uint8_t    unsigned char                         
// #define         int8_t			signed char                                     
// #define         int8_t		unsigned short int                              
// #define         int16_t		signed short int                                
// #define         uint32_t		unsigned long int                               
// #define        int32_t		signed  long int                                
//所支持最大指令级数为8级
#define                         INSTRUCTION_MAX_SCALE                  (4)
//指令最大参数 本程序所支持的指令参数最大数目为16个
#define                         INSTRUCTION_MAX_PARAM                  (16)


//无单位符号
#define                         SIGN_SYMBOL_INDEX_NONE                 (0xff)
#define                         SIGN_SYMBOL_INDEX_NA                   (0x00)
#define                         SIGN_SYMBOL_INDEX_UA                   (0x01)
#define                         SIGN_SYMBOL_INDEX_MA                   (0x02)
#define                         SIGN_SYMBOL_INDEX_A                    (0x03)
#define                         SIGN_SYMBOL_INDEX_V                    (0x04)
#define                         SIGN_SYMBOL_INDEX_KV                   (0x05)
#define                         SIGN_SYMBOL_INDEX_mOHM                 (0x06)
#define                         SIGN_SYMBOL_INDEX_OHM                  (0x07)
#define                         SIGN_SYMBOL_INDEX_KOHM                 (0x08)
#define                         SIGN_SYMBOL_INDEX_MOHM                 (0x09)
#define                         SIGN_SYMBOL_INDEX_GOHM                 (0x0a)
#define                         SIGN_SYMBOL_INDEX_TOHM                 (0x0b)
#define                         SIGN_SYMBOL_INDEX_MS                   (0x0c)
#define                         SIGN_SYMBOL_INDEX_S                    (0x0d)
#define                         SIGN_SYMBOL_INDEX_HZ                   (0x0e)
#define                         SIGN_SYMBOL_INDEX_W                    (0x0f)

/******************************************************************************
 *                           指令解析信息宏定义
******************************************************************************/ 

//压入输出队列时 不用添加符号 ','
#define                         NO_APPENDED_SIGN                       ((uint32_t)-1)

#define                         INSTRUCTION_ATTR_NONE                  (0x00)                               
#define                         INSTRUCTION_ATTR_EXECUT                (0x01)
#define                         INSTRUCTION_ATTR_QUERY                 (0x02)
#define                         INSTRUCTION_ATTR_OPTIONAL              (0x04)
#define                         INSTRUCTION_ATTR_COMM                  (0x10)

//宏函数 定义指令属性中的 指令级数  最大指令级数为8级
#define                         INSTRUCTION_ATTR_SCALES(scales)        (scales << 5)
//宏函数 获取指令级数 最大指令级数为8级
#define                         INSTRUCTION_SCALES_GET(attr)           (attr >> 5)

//第一项 函数执行地址（或为函数执行编号） 第二项 查询函数还是执行函数  第三项 具体参数  参数1'\0'参数2'\0'参数3'\0'
//参数队列与输入队列共用 数据格式以大端方式存放 并使用相应的函数进行获取 规避了大小端与内存对齐方面的要求
typedef uint32_t (FP_pfInstructionExec)(int argc, const uint8_t *argv[]);

struct EXECUTE_VALIDITY_CHK_INFO;
//执行合法性检查函数指针
typedef uint32_t (FP_pfValidityChk)(struct EXECUTE_VALIDITY_CHK_INFO *pexecuteValidityChkInfo);
//输出队列发送函数指针
typedef uint32_t (FP_pfOutputQueueSend)(const uint8_t *prscAddr, portQUEUELEN_TYPE len);

typedef struct
{
    uint8_t                                      *m_pdestSplitAddr;
    uint32_t                                      m_rawDecValue;
    uint8_t                                       m_rawConfigLen;
    uint8_t                                       m_rawDotValue;
    uint8_t                                       m_rawSignValue;

}RealParamSplitStruct;

//实型数据拆分自定义函数
typedef uint32_t (FP_pfRealParamSplit)(RealParamSplitStruct *prealParamSplitStruct);

/******************************************************************************
 *                         文件接口数据结构体声明
******************************************************************************/

/******************************************************************************
 *                       执行合法性检查信息结构体定义说明
******************************************************************************/

#define                         VALIDITY_CHK_STEP_RREV_EXECUTE         (0)
#define                         VALIDITY_CHK_STEP_RREV_OUTPUT          (1)

struct EXECUTE_VALIDITY_CHK_INFO   
{
    FP_pfInstructionExec                       *m_pfinstructionExec;            //执行函数
    uint8_t                                       m_instructionAttr;              //执行属性                                          
    uint8_t                                       m_step;                         //阶段
};
// typedef struct TEST_FENTCH_INFO_
// {
// 	uint32_t 		Test_Diaplay_VOL;
// 	uint32_t 		Test_Dispaly_CUR;
// 	uint32_t 		Test_Dispaly_IR;
// 	uint32_t        Test_Status;
// }FentchInfo;
// volatile FentchInfo t_FentchInfo;
// typedef struct _PARAM_INFO_
// {
//     volatile uint32_t	        Voltage_AC;
// 	volatile uint32_t	        Voltage_DC;

// 	volatile int32_t	        Current_AC;
// 	volatile int32_t	        Current_DC;
// 	volatile uint32_t			Current_AC_flag;
// 	volatile uint32_t			Current_DC_flag;
// 	volatile uint32_t			Current_AC_LOW_flag;
// 	volatile uint32_t			Current_DC_LOW_flag;

// 	volatile int32_t	        Current_AC_LOW;
// 	volatile int32_t	        Current_DC_LOW;

// 	volatile int32_t	        Time_AC;
// 	volatile int32_t	        Time_DC;
// 	volatile uint32_t	        Set_time_flag;
// 	volatile uint32_t			Set_time_dc_flag;

// 	volatile uint32_t			Arc_DC;
// 	volatile uint32_t			Arc_DC_Data;

// 	volatile uint32_t			Arc_AC;
// 	volatile uint32_t			Arc_AC_Data;
// 		
// 	volatile uint32_t	        Test_current_mode;
// 	volatile uint32_t	        Test_Mode;

// 	volatile uint32_t			AC_Test_display;
// 	volatile uint32_t         DC_Test_display;
// 	
// }ParamInfo;
// volatile ParamInfo st_ParamInfo;
typedef struct EXECUTE_VALIDITY_CHK_INFO ExecuteValidityChkInfo;

/******************************************************************************
 *                         指令解析结构体定义说明
 ******************************************************************************
 *                                                                          
 *  本解析执行模块所支持的用于指令解析结构体定义的类型如下:
 *  当指令只有一级时 使用SingleScaleParserStruct来定义指令解析结构体
 *  当指令大于一级时 使用 HeadScaleParserStruct, MiidleScaleParserStruct, TailScaleParserStruct组合来定义解析结构体                                                                       
 *                                                                                                              
 *  HeadScaleParserStruct:   多级指令的头解析结构体 用于指令中的第一级指令 如：AA:BB:CC 中的AA级
 *                                                                          
 *  MidleScaleParserStruct:  多级指令（大于两级）的中间解析结构体 用于指令中的中间级指令 如：AA:BB:CC 中的BB级                                                                       
 *                           但是若 指令只有两级指令构成 则不能使用MidleScaleParserStruct结构体来定义
 *                                                                                   
 *  TailScaleParserStruct:   多级指令的尾解析结构体 用于指令中的最后一级指令 如：AA:BB:CC 中的CC级  
 *                           当指令只有两级指令构成时 则第二级指令由TailScaleParserStruct来定义
 *                           如AA:BB 中格的BB级指令                                            
 *                                                                          
 *  SingleScaleParserStruct: 单级解析结构体 当指令只存在一级时 用此解析结构                                                                       
 *                           体定义指令  如*RST指令等                                                                                   
 *                                                                                                                                                                          
 *                                                                                                                                                        
******************************************************************************/

/******************************************************************************
 *                           指令解析结构体声明
******************************************************************************/

//本解析执行模块最大可支持256条根指令   每级指令下面又可最多支持256条子级指令   整条指令最多可支持8级

struct PARSER_CONTAINER;

//前几级解析结构体
struct HEAD_SCALE_PARSER_STRUCT
{
    const uint8_t                                *m_pinstructionStr;                      //指令字符串地址
    const struct PARSER_CONTAINER              *m_pnextParserContainer;                 //下一个解析容器
    uint8_t                                       m_longInstructionHashCode;              //长指令哈希码
    uint8_t                                       m_shortInstructionHashCOde;             //短指令哈希码
    uint8_t                                       m_instructionAttribute;                 //命令属性 是否为可选指令
    uint8_t                                       m_nextParserContainerCapacity;          //下一个解析容器容量
};
                         
//m_instructionAttribute    在HeadScaleParserStruct里含义如下
//bit   7   6   5   4   3   2   1   0
//bit7 - bit5 :扩展位 用于功能扩展
//bit4        :扩展位 用于功能扩展
//bit3        :扩展位 用于功能扩展
//bit2        :指示本级指令是否可选
//bit1        :扩展位 用于功能扩展
//bit0        :扩展位 用于功能扩展

typedef struct HEAD_SCALE_PARSER_STRUCT    HeadScaleParserStruct;

//m_instructionAttribute    在MidleScaleParserStruct里含义如下
//bit   7   6   5   4   3   2   1   0
//bit7 - bit5 :扩展位 用于功能扩展
//bit4        :扩展位 用于功能扩展
//bit3        :扩展位 用于功能扩展
//bit2        :指示本级指令是否可选
//bit1        :扩展位 用于功能扩展
//bit0        :扩展位 用于功能扩展

typedef struct HEAD_SCALE_PARSER_STRUCT    MidleScaleParserStruct;

struct PARAM_TOTAL_CONTAINER;

//末级解析结构体
struct TAIL_SCALE_PARSER_STRUCT
{
    const uint8_t                                *m_pinstructionStr;                      //指令字符串地址
    FP_pfInstructionExec                       *m_executeFunc;                          //指令执行函数 
    const struct PARAM_TOTAL_CONTAINER         *m_pparamTotalContainer;                 //参数总容器结构体指针
    uint8_t                                       m_longInstructionHashCode;              //长指令哈希码
    uint8_t                                       m_shortInstructionHashCOde;             //短指令哈希码
    uint8_t                                       m_instructionAttribute;                 //命令属性
};

typedef struct TAIL_SCALE_PARSER_STRUCT    TailScaleParserStruct;

//m_instructionAttribute    在TailScaleParserStruct里含义如下
//bit   7   6   5   4   3   2   1   0
//bit7 - bit5 :指示指令级数  最大支持8级指令
//bit4        :指示本指令是否为公用命令 INSTRUCTION_ATTR_COMM
//bit3        :扩展位 用于功能扩展
//bit2        :扩展位 用于功能扩展
//bit1        :指示本指令是否支持查询格式
//bit0        :指示本指令是否支持执行格式

//单级解析结构体 当指令只存在一级时 用此解析结构体定义指令  如*RST指令等
typedef struct TAIL_SCALE_PARSER_STRUCT    SingleScaleParserStruct;

//m_instructionAttribute    在SingleScaleParserStruct里含义如下
//bit   7   6   5   4   3   2   1   0
//bit7 - bit5 :指示指令级数  最大支持8级指令
//bit4        :指示本指令是否为公用命令 INSTRUCTION_ATTR_COMM
//bit3        :扩展位 用于功能扩展
//bit2        :扩展位 用于功能扩展
//bit1        :指示本指令是否支持查询格式
//bit0        :指示本指令是否支持执行格式

/******************************************************************************
 *                            指令解析容器声明
******************************************************************************/

//容器解析类型 为了方便以后HeadScaleParserStruct MiidleScaleParserStruct 和 TailScaleParserStruct的升级 
//所以使用了容器进行封装
typedef enum
{
    PARSER_SINGLE_SCALE = 0,
    //区别在于所携带的参数总容器中参数容器指向的一个参数容器指针数组 多了一层包装 提高了复用性
    //PCPA = PARAM CONTAINER PTR ARRAY
    PARSER_SINGLE_SCALE_PCPA,
    PARSER_HEAD_SCALE,
    PARSER_MIDDLE_SCALE,
    PARSER_TAIL_SCALE,
    //区别在于所携带的参数总容器中参数容器指向的一个参数容器指针数组 多了一层包装 提高了复用性
    PARSER_TAIL_SCALE_PCPA,                                                            

}ContainerInfo;

struct PARSER_CONTAINER
{
    void                                       *m_pparserStructAddr;
    ContainerInfo                               m_parserStructType;
};

typedef struct PARSER_CONTAINER    ParserContainer;

/******************************************************************************
 *                             参数容器类型声明
******************************************************************************/

//布尔类型与字符类型 在本程序中划分为一种类型 具体由说明书中说明 面向用户不面向程序
//参数最多划分15种 每种参数中可细化为15种 其中第16种参数为无效类型 
typedef enum
{
    //-----------------------
    //整型类型
    PARAM_TYPE_INTEGER          = 0x00,
    PARAM_TYPE_UINT8,
    PARAM_TYPE_INT8,
    PARAM_TYPE_UINT16,
    PARAM_TYPE_INT16,
    PARAM_TYPE_UINT32,
    PARAM_TYPE_INT32,
    //-----------------------
    //字符型类型
    PARAM_TYPE_CHARACTER        = 0x10, 
    //-----------------------
    //串型类型
    PARAM_TYPE_STRING           = 0x20,
    //-----------------------
    //实型类型
    PARAM_TYPE_REAL             = 0x30,
    PARAM_TYPE_REAL_UINT16,
    PARAM_TYPE_REAL_UINT32,
    //-----------------------
    //浮点型类型
    PARAM_TYPE_FLOAT            = 0x40,
    //-----------------------
    //无效的参数类型
    PARAM_TYPE_NONE             = 0xf0,                                                           

}ParamTypeInfo;

//参数类型获取
#define                         PARAM_TYPE_CLASS_GET(param)            (ParamTypeInfo)(param & 0xf0)

/******************************************************************************
 *                           参数容器信息、属性声明
******************************************************************************/

//----------------------------总参数容器属性宏定义----------------------------

//参数总容器属性：执行指令参数
#define                         CONTAINER_ATTR_EXECUTE                 (0x01)
//参数总容器属性：查询指令参数
#define                         CONTAINER_ATTR_QUERY                   (0x02)
//参数总容器属性：支持执行、查询参数
#define                         CONTAINER_ATTR_DOUBLE                  (0x04)
//参数总容器属性：支持执行、查询参数中的一个
#define                         CONTAINER_ATTR_SINGLE                  (0x00)

//--------------------------总参数容器参数信息宏定义---------------------------

//总参数个数信息
#define                         TOTAL_PARAM_INFO_TOT_CNTS(addr)        (GET_ARRAY_COUNT(addr) & (INSTRUCTION_MAX_PARAM-1))
#define                         TOTAL_PARAM_INFO_TOT_CNTS_GET(info)    (info & 0x0f)
//可选参数个数信息  存放于高16位
#define                         TOTAL_PARAM_INFO_OPT_CNTS(cnts)        ((cnts & (INSTRUCTION_MAX_PARAM-1)) << 4)
#define                         TOTAL_PARAM_INFO_OPT_CNTS_GET(info)    ((info & 0xf0) >> 4)

//---------------------------参数容器参数信息宏定义----------------------------

#define                         PARAM_INFO_NONE                        (0x00)
//参数支持DEF属性  在发送DEF或DEFault时 允许接收
#define                         PARAM_INFO_DEF                         (0x80)
//参数具有十进制输出属性则解析器输出参数值和其档位值 如电阻若不具有该属性输出9.000_Mohm 反之则输出9000_1
#define                         PARAM_INFO_DEC_OUT                     (0x40)
//参数略过属性 忽略对此参数的合法性判断 直接写入到参数队列
#define                         PARAM_INFO_SKIP                        (0x20)

//实数参数容器信息宏定义
//实数参数段配置个数信息 目前最多支持到16段 对于一般的应用来讲已经足够
#define                         REAL_PARAM_INFO_SECTION_CNTS(cnts)     ((GET_ARRAY_COUNT(cnts)) & 0x0f)
#define                         REAL_PARAM_INFO_SECTION_CNTS_GET(info) (info & 0x0f)
//当出现如：2482 V类似字符串时 是否自动转换到 2.482 kV上
#define                         REAL_PARAM_INFO_NO_AUTO_SIGN           (0x10)

/******************************************************************************
 *                                参数容器声明
******************************************************************************/

struct PARAM_CONTAINER;
//总容器
struct PARAM_TOTAL_CONTAINER
{
    struct PARAM_CONTAINER                     *m_pparamContainer;
    //容器信息 指示出是执行参数容器 还是 查询参数容器 若指令同时支持执行参数与查询参数时 则总参数容器中
    //具有两项信息 其值为CONTAINER_ATTR_SINGLE、CONTAINER_ATTR_EXECUTE、CONTAINER_ATTR_DOUBLE、CONTAINER_ATTR_QUERY
    //的组合
    uint8_t                                       m_containerAttr;
    //参数信息 分为高四位与低四位 最多可支持16位参数
    //高四位：可选参数个数
    //低四位：总参数个数
    uint8_t                                       m_paramInfo;

};
//总容器为一个最多具有两组原属的结构体数组 其中第一个对应执行参数总容器  第二个对应查询参数总容器
typedef struct PARAM_TOTAL_CONTAINER ParamTotalContainer;

struct PARAM_CONTAINER
{
    void                                       *m_pparamStructAddr;
    //参数类型信息 具体取值参考ParamTypeInfo类型
    ParamTypeInfo                               m_paramStructType;
    //对于不同的参数结构体类型 具有不同的含义
    //对于PARAM_TYPE_REAL_UINT16、PARAM_TYPE_REAL_UINT32类型
    //高四位
    uint8_t                                       m_paramStructInfo; 
};

typedef struct PARAM_CONTAINER    ParamContainer;

typedef struct 
{
    uint8_t                                       m_upperLimit;
    uint8_t                                       m_lowerLimit;

}ParamUint8Struct;

typedef struct 
{
     int8_t                                       m_upperLimit;
     int8_t                                       m_lowerLimit;

}ParamInt8Struct;

typedef struct 
{
    uint16_t                                      m_upperLimit;
    uint16_t                                      m_lowerLimit;

}ParamUint16Struct;

typedef struct 
{
     int16_t                                      m_upperLimit;
     int16_t                                      m_lowerLimit;

}ParamInt16Struct;

typedef struct 
{
    uint32_t                                      m_upperLimit;
    uint32_t                                      m_lowerLimit;

}ParamUint32Struct;

typedef struct 
{
     int32_t                                      m_upperLimit;
     int32_t                                      m_lowerLimit;

}ParamInt32Struct;

//字符单元
typedef struct
{
    uint8_t                                      *m_plabelStr;
    uint8_t                                       m_value;

}CharacterUnit;

//字符参数结构体
typedef struct 
{
     const CharacterUnit                       *m_pcharacterTable;
     uint8_t                                      m_characterTableNumbs;

}ParamCharacterStruct;

//布尔类型是字符类型的一个子集
typedef     ParamCharacterStruct    ParamBooleanStruct;

//串类型结构体
typedef struct 
{
     uint8_t                                     *const m_pinvalidStr;

}ParamStringStruct;

/******************************************************************************
 *                              单位符号索引声明
******************************************************************************/

//配置信息宏定义
//实数参数信息 -> 小数点位置
//无小数点
#define                         REAL_CFG_INFO_NO_DOT                   (0x00)
#define                         REAL_CFG_INFO_DOT_POS(dotPos)          (dotPos & 0x0f)
#define                         REAL_CFG_INFO_DOT_POS_GET(configInfo)  (configInfo & 0x0f)
//实数参数信息 -> 整体长度 所有段
#define                         REAL_CFG_INFO_STR_LEN(len)             (len << 4)
#define                         REAL_CFG_INFO_STR_LEN_GET(configInfo)  (configInfo >> 4)


//实型类型结构体 最大值为65535 最小值和最大值符号 小数点位置都相同
typedef struct
{
    uint16_t                                      m_lowerLimit;
    uint16_t                                      m_upperLimit;
    //配置信息   高4位为字符串除去小数点之后的长度  低4位为小数点在字符串内的偏移
    uint8_t                                       m_configInfo;
    //符号信息                                  
    uint8_t                                       m_signSymbolIndex;

}ParamRealUint16Struct;

//实型类型结构体 最大值为uint32类型 最小值和最大值符号 小数点位置都相同
typedef struct
{
    uint32_t                                      m_lowerLimit;
    uint32_t                                      m_upperLimit;
    //配置信息   高4位为字符串除去小数点之后的长度  低4位为小数点在字符串内的偏移
    uint8_t                                       m_configInfo;
    //符号信息                                  
    uint8_t                                       m_signSymbolIndex;

}ParamRealUint32Struct;

/******************************************************************************
 *                         解析执行组件结构体声明
******************************************************************************/

typedef enum
{
    STATUS_NO_ERROR  = 0,
    STATUS_SYNTAX_ERROR,
    STATUS_EXECUTE_NOT_ALLOWED,
    STATUS_PARAM_NOT_ALLOWED,
    STATUS_MISSING_PARAM,
    STATUS_UNDEFINED_HEADER,
    STATUS_PARAM_TYPE_ERROR,
    STATUS_PARAM_LENGTH_ERROR,
    STATUS_INVALID_STRING_DATA,
    STATUS_EXECUTE_TIME_OUT,
    STATUS_DATA_OUT_OF_RANGE,
    STATUS_OUTPUT_QUEUE_FULL,
    STATUS_SUM_CHECK_ERROR,

}ParserErrCode;



extern const ParserContainer                    t_ParserContainer_Root_CS5051[7];
/******************************************************************************
 *                             文件接口数据声明
******************************************************************************/ 

extern const uint8_t *const API_PEM_pParserExeModulerErrCodeArray[];

/******************************************************************************
 *                            文件接口函数声明
******************************************************************************/

//根解析容器地址、根解析容器容量、本机地址、结束符索引、应答控制
extern void API_PEMInit(ParserContainer *ptrootParserContainer, uint8_t prootParserContainerCapacity,
                        uint8_t endCodeIndex, uint8_t responsionCtrl, FP_pfOutputQueueSend *pfOutputQueueSend);
                                 
extern void API_PEMExecute(void);

extern void int_API_PEMRead(uint8_t recvMsg);

extern uint32_t API_PEMOutputQueueErrMsgPush(ParserErrCode parserErrCode, uint32_t appendSign);

extern uint32_t API_PEMExecuteErrCodeSet(ParserErrCode executeErrCode);

extern uint32_t API_PEMOutputQueueStrPush(const uint8_t *prscStr, uint32_t appendSign);

extern uint32_t API_PEMOutputQueueStrnPush(const uint8_t *prscStr, uint32_t len, uint32_t appendSign);

extern uint32_t API_PEMOutputQueueIntegerPush(uint32_t rscData, uint32_t len, uint32_t appendSign);

extern uint32_t API_PEMOutputQueueCharPush(uint32_t appendChar, uint32_t appendSign);

extern void API_PEMParamQueueIntegerWrite(uint32_t rscValue, uint8_t len);

extern void API_PEMParamQueueCharWrite(uint8_t rscChar);

extern void API_PEMParamQueueSplitSignWrite(uint8_t rscChar);

extern void API_PEMParamQueuePosAdjust(uint16_t len);

extern uint8_t *API_PEMParamQueueCurrentPtrGet(void);

extern uint8_t API_PEMItegerParamReadUint8(const uint8_t *rscAddr);

extern int8_t API_PEMItegerParamReadInt8(const uint8_t *rscAddr);

extern uint16_t API_PEMItegerParamReadUint16(const uint8_t *rscAddr);

extern int16_t API_PEMItegerParamReadInt16(const uint8_t *rscAddr);

extern uint32_t API_PEMItegerParamReadUint32(const uint8_t *rscAddr);

extern int32_t API_PEMItegerParamReadInt32(const uint8_t *rscAddr);

extern void API_PEMPrevValidityChkFuncInstall(FP_pfValidityChk *pfprevExecuteValidityChk);

extern void API_PEMPrevValidityChkFuncUninstall(void);

extern void API_PEMRealParamSplitFuncInstall(FP_pfRealParamSplit *pfRealParamSplit);

extern void API_PEMRealParamSplitFuncUninstall(void);

extern void _API_PEMInputQueueFlush(void);
 
extern void _API_PEMOutputQueueFlush(void);


//输出队列发送函数 此函数必须由外部实现 
extern void port_API_PEMOutputQueueSend(void);

#endif

