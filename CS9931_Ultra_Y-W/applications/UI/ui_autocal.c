#include "CS99xx.h"
#include "bsp_listbox.h"

#include "sui_window.h"
#include "CS99xx_cal.h"

#include "Output_Control.h"
#include "Cal.h"
#include "GR.h"
#include "LC.h"
#include "Relay_Change.h"
#include "Relay.h"


static void      Push_Data_To_Buffer(uint8_t data);
static uint8_t   Get_Data_From_Buffer(uint8_t *data);
extern uint32_t  rt_uart_write(const void* buffer, uint32_t size);
extern void dyj_output(uint8_t mode, uint16_t value);
extern void dyj_save_ad(uint8_t mode, uint8_t item, uint8_t cal, uint32_t val);
extern void dyj_save_lc_selv_ad(uint32_t val);
extern void dyj_save_lc_cur_ad(uint8_t network, uint8_t dem, uint8_t cal, uint32_t val);
extern void dyj_lc_relay(uint8_t network, uint8_t dem, uint8_t cal);
static void Autocal_Save(uint8_t index,uint32_t value);

/*
		协议帧格式:
		协议头   地址码   功能码   数据区     CRC
		2 bytes  1 byte   1 bytes  4 bytes   2 bytes
*/

#define    MY_AUTOCAL_ADDR           (31)          //本机自动校准地址

#define    INSTURCT_ASK_LEN          (10)          //询问指令的长度
#define    INSTURCT_ACK_LEN          (10)          //应答指令的长度

#define    PROTOCOL_ASK_HEAD_HIGH    (0xAA)        //询问指令头的高八位
#define    PROTOCOL_ASK_HEAD_LOW     (0x55)        //询问指令头的低八位
#define    PROTOCOL_ACK_HEAD_HIGH    (0x55)        //应答指令头的高八位
#define    PROTOCOL_ACK_HEAD_LOW     (0xAA)        //应答指令头的低八位

//主机功能码
#define    PROTOCOL_ASK_STATE        (0x00)        //询问状态
#define    PROTOCOL_ASK_CALMODE      (0x01)        //询问校准模式
#define    PROTOCOL_ASK_RANGE        (0x02)        //询问数据范围
#define    PROTOCOL_OUTPUT_START     (0x03)        //启动输出
#define    PROTOCOL_OUTPUT_STOP      (0x04)        //停止输出
#define    PROTOCOL_CAL_DATA         (0x05)        //发送校准数据
#define    PROTOCOL_ASK_TOTAL_POINTS (0x07)        //询问校准点的总数

//从机应答码
#define    PROTOCOL_OK               (0x00)        //通讯OK
#define    PROTOCOL_CRC_ERR          (0x01)        //CRC校验错误
#define    PROTOCOL_TIMEOUT_ERR      (0x02)        //通讯超时错误
#define    PROTOCOL_RANGEOUT_ERR     (0x03)        //参数范围错误
#define    PROTOCOL_REPORT_STATUS    (0x04)        //汇报状态
#define    PROTOCOL_REPORT_MODE      (0x05)        //汇报模式
#define    PROTOCOL_REPORT_RANGE     (0x06)        //汇报范围
#define    PROTOCOL_NOTSUCHINS       (0x07)        //无法解析
#define    PROTOCOL_REPORT_POINTS    (0x08)        //汇报校准点个数

#define    AUTOCAL_AC_V              (0)           //交流电压
#define    AUTOCAL_AC_A              (2)           //交流电流
#define    AUTOCAL_DC_V              (3)           //直流电压
#define    AUTOCAL_DC_A              (4)           //直流电流
#define    AUTOCAL_RES               (5)           //电阻
#define    AUTOCAL_PASS              (6)           //此点不校准
#define    AUTOCAL_GR_A              (7)           //接地电流
#define    NO_SUCH_POINT             (255)         //无此模式

//从机状态码
#define    AUTOCAL_WAIT_CONNECT      (0)           //等待被连接
#define    AUTOCAL_CONNECTED         (1)           //已经连接
#define    AUTOCAL_WAIT_DATA         (2)           //等待校准数据
#define    AUTOCAL_OUTPUTING         (3)           //正在输出
#define    AUTOCAL_SAVEDATA          (4)           //保存数据
#define    AUTOCAL_STOP_OUTPUT       (5)           //停止输出
#define    AUTOCAL_WAIT_STOP         (6)           //等待停止

#define    AUTOCAL_PANEL             (9)           //当前的页

#define    ACTIVATE_CODE             (0xFF)        //自动校准的激活码
#define    ACTIVATE_COUNTER          (10)          //有效时激活的次数

#define    DATA_BUFFER_SIZE          (128)         //通讯缓冲的大小
#define    ACK_STATUS_INDEX          (3)           //从机应答状态在指令中的位置

#define    CAL_POINT_NUM             (17)           //校准点的个数


#define    ASK_FRAME                 (*pask_frame)
#define    ACK_FRAME                 (*pack_frame)

//指令的结构
typedef struct{
	uint8_t  ask_head[2];
	uint8_t  addr;
	uint8_t  function;
	uint32_t data;
	uint16_t crc;
}INSTRUCT_FRAME;

//校准点的结构
typedef struct{
	uint8_t         index;
	uint8_t         mode;
	uint32_t        range;
}CAL_POINT_FRAME;

//校准点数据
typedef struct
{
	uint8_t   index;                    //校准点索引
	uint8_t   mode;                     //校准点的模式
	uint32_t  range;                    //校准点的范围
	uint32_t  cal_value;                //校准的数据
}CAL_POINT;

static const uint16_t CRC16_TABLE[256];            //CRC16CCITT(1021)的16字表长查表程序
static const CAL_POINT_FRAME Cal_Point[];
static CAL_POINT Cal_Point_Now;

static uint16_t ComputeCRC16(uint8_t *databuf, uint8_t length);
static void Protocol_ACK_CRC_Tail(void);
static void Protocol_Slave_Init (void);
static void AutoCal_Comm_Execute(void);
static void Protocol_ACK(void);
static uint8_t  AutoCal_Comm_Check(uint8_t data);
static uint8_t Protocol_CRC_Check(void);
static void    Autocal_Output_Start(uint8_t index);
static void    Autocal_Output_Stop(uint8_t index);

static uint8_t Insturct_ASK_Buffer[INSTURCT_ASK_LEN];    //接收指令缓冲
static uint8_t Insturct_ACK_Buffer[INSTURCT_ACK_LEN];    //应答缓冲

static uint8_t Protocol_ASK_index;   

static INSTRUCT_FRAME *pask_frame = (INSTRUCT_FRAME *)Insturct_ASK_Buffer;
static INSTRUCT_FRAME *pack_frame = (INSTRUCT_FRAME *)Insturct_ACK_Buffer;

//是否已经激活自动校准的标志
static  uint8_t  Is_apparatus_Autocal = 0;
static  uint8_t  AutoCal_Status       = AUTOCAL_WAIT_CONNECT;

char *autocal_title[1] = {"Auto Calibration Mode"};

void ui_autocal_thread(void)
{

	rt_uint32_t msg;

	struct font_info_t font={&panel_home,CL_YELLOW,0x0,1,0,24};

	uint8_t   data;
	if(panel_flag == AUTOCAL_PANEL)
	{
		ui_key_updata(0xF8);
		buzzer(50);
		clr_mem((u16 *)ExternSramHomeAddr,0x19f6,ExternSramHomeSize/2);
		font_draw((680-rt_strlen(autocal_title[0])*12)/2,10,&font,autocal_title[0]);
		
		/* 绘制水平线 */
		clr_win(&panel_home,0xf800,30,35,3,620);
		
		font.high = 16;
		font.fontcolor = 0xffff;
		
		font_draw(40,70+30,&font,T_STR("正在与自动校准装置进行通讯...","Communicate with automatic calibration..."));
		
		rt_mb_send(&screen_mb, UPDATE_HOME);
		
		//锁住键盘
		system_parameter_t.key_lock = 1;
		rt_mb_send(&screen_mb, UPDATE_STATUS | STATUS_KEYLOCK_EVENT | (system_parameter_t.key_lock));
		
		Protocol_Slave_Init();
		
		AutoCal_Status       = AUTOCAL_CONNECTED;              //自动校准通讯已经连接
		
		ACK_FRAME.function   = PROTOCOL_OK;   //汇报通讯OK
		Protocol_ACK();
		
		Cal_Point_Now.index      = 0;
		Cal_Point_Now.cal_value  = 0;
	}
	
	while(panel_flag == AUTOCAL_PANEL)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			switch(msg)
			{
				/* 返回 */
				case KEY_F6 | KEY_UP:
					Is_apparatus_Autocal = 0;
					panel_flag = 0;
					break;
				case KEY_ENTER | KEY_UP:
					break;
				default:
					break;
			}
		}
		else
		{
			while(Get_Data_From_Buffer(&data))         //如果有数据，进行校准的解析
			{
				if(AutoCal_Comm_Check(data) == 1)
				{
					AutoCal_Comm_Execute();
				}
			}
		}
	}
}

/**************************************************************
函数名:AutoCal_Comm_Check
功能:  通讯数据检查
参数:  data:数据
返回值:1:可以解析
       0:无法解析
***************************************************************/
static  uint8_t  AutoCal_Comm_Check(uint8_t data)
{
	if(Protocol_ASK_index == 0)
	{
		if(data != PROTOCOL_ASK_HEAD_HIGH)
		{
			Protocol_ASK_index = 0;			
		}
		else
		{
			Insturct_ASK_Buffer[Protocol_ASK_index++] = data;
		}
		return 0;
	}
	if(Protocol_ASK_index == 1)
	{
		if(data != PROTOCOL_ASK_HEAD_LOW)
		{
			Protocol_ASK_index = 0;		
		}
		else
		{
			Insturct_ASK_Buffer[Protocol_ASK_index++] = data;
		}
		return 0;
	}
	Insturct_ASK_Buffer[Protocol_ASK_index++] = data;
	if(Protocol_ASK_index < INSTURCT_ASK_LEN)
	{
		return 0;
	}
	else
	{
		Protocol_ASK_index = 0;
		if(Protocol_CRC_Check() != 1)  //CRC校验错误
		{ 
			ACK_FRAME.function = PROTOCOL_CRC_ERR;   //汇报CRC校验错误
			Protocol_ACK();
			return 0;
		}
		else
		{
		
		}
	}
	return 1;
}

/*******************************
函数名：  AutoCal_Comm_Execute
功  能：  自动校准指令的解析
参  数：  收到的数据

返回值：  无
********************************/

static  void  AutoCal_Comm_Execute(void)
{
	if(ASK_FRAME.addr != MY_AUTOCAL_ADDR)  return;     //不是本机的地址，直接返回
	switch(ASK_FRAME.function)	
	{
		case PROTOCOL_ASK_STATE:
			ACK_FRAME.function = PROTOCOL_REPORT_STATUS;   //汇报状态
		  ACK_FRAME.data     = AutoCal_Status;
		break;
		case PROTOCOL_ASK_CALMODE:
			ACK_FRAME.function = PROTOCOL_REPORT_MODE;     //汇报模式
			if(ASK_FRAME.data > CAL_POINT_NUM)             //此时传递过来的参数为目标校准点
			{
				ACK_FRAME.data  =  NO_SUCH_POINT;
			}
			else
			{
				Cal_Point_Now.index =  ASK_FRAME.data;
				ACK_FRAME.data      =  Cal_Point[Cal_Point_Now.index].mode;
			}
		break;
		case PROTOCOL_ASK_RANGE:
			ACK_FRAME.function = PROTOCOL_REPORT_RANGE;    //汇报范围
			ACK_FRAME.data  =  Cal_Point[Cal_Point_Now.index].range;
		break;
		
		case PROTOCOL_OUTPUT_START:
			Autocal_Output_Start(Cal_Point_Now.index);
			AutoCal_Status = AUTOCAL_OUTPUTING;
			ACK_FRAME.function = PROTOCOL_REPORT_STATUS;   //汇报状态
		  ACK_FRAME.data     = AutoCal_Status;
		break;
		case PROTOCOL_OUTPUT_STOP:
			Autocal_Output_Stop(Cal_Point_Now.index);
			AutoCal_Status = AUTOCAL_STOP_OUTPUT;
			ACK_FRAME.function = PROTOCOL_REPORT_STATUS;   //汇报状态
		  ACK_FRAME.data     = AutoCal_Status;
		break;
		case PROTOCOL_CAL_DATA:
			Cal_Point_Now.cal_value =  ASK_FRAME.data;
			Autocal_Save(Cal_Point_Now.index,Cal_Point_Now.cal_value);   //保存数据
			AutoCal_Status = AUTOCAL_SAVEDATA;
			ACK_FRAME.function = PROTOCOL_REPORT_STATUS;   //汇报状态
		  ACK_FRAME.data     = AutoCal_Status;
		break;
		case PROTOCOL_ASK_TOTAL_POINTS:
			ACK_FRAME.function = PROTOCOL_REPORT_POINTS;   //汇报校准点个数
		  ACK_FRAME.data     = CAL_POINT_NUM;
		break;
		default:
			ACK_FRAME.function = PROTOCOL_NOTSUCHINS;     //无法解析		
		break;
	}
	Protocol_ACK();
}

/*******************************
函数名：  Usart_Data_Pretreat
功  能：  串口数据的预处理，判断是否为需要的数据
参  数：  收到的数据

返回值：  无
********************************/
uint8_t  Usart_Data_Pretreat(uint8_t data)
{
	//激活码个数计数
	static uint8_t activate_code_count = 0;
	uint8_t IsneedSCPI = 0;
	if(Is_apparatus_Autocal)
	{
		Push_Data_To_Buffer(data);
		IsneedSCPI = 0;
	}
	else
	{
		if(data == ACTIVATE_CODE)
		{
			if(++activate_code_count >= ACTIVATE_COUNTER)  //激活自动校准
			{
				Is_apparatus_Autocal  =  1;	
				panel_flag = AUTOCAL_PANEL;
				activate_code_count = 0;
			}
			IsneedSCPI = 0;
		}
		else
		{
			activate_code_count = 0;
			IsneedSCPI = 1;
		}
	}
	return IsneedSCPI;
}


//定义缓冲区
static uint8_t Data_buf[DATA_BUFFER_SIZE];		
static struct{
	uint8_t rd_index;
	uint8_t wr_index;
	volatile uint8_t count;
	uint8_t buf_overflow;
	uint8_t *pData;
}Data_Flag = {0, 0, 0, 0, Data_buf};


/*******************************
函数名：  Push_Data_To_Buffer
功  能：  将要解析的数据压入缓冲
参  数：  

返回值：  无
********************************/
static void Push_Data_To_Buffer(uint8_t data)
{
	if(Data_Flag.count < DATA_BUFFER_SIZE){	
			Data_Flag.count++;
			*(Data_Flag.pData + Data_Flag.wr_index) = data;
			if(++Data_Flag.wr_index == DATA_BUFFER_SIZE)
				Data_Flag.wr_index = 0;
	}
}

/*******************************
函数名：  Get_Data_From_Buffer
功  能：  从缓冲中获取数据
参  数：  

返回值：  1获取成功
          0无数据
********************************/
static uint8_t Get_Data_From_Buffer(uint8_t *pdata)
{
	if(Data_Flag.count){
	 	*pdata = *(Data_Flag.pData + Data_Flag.rd_index);
		if(++Data_Flag.rd_index == DATA_BUFFER_SIZE)
			Data_Flag.rd_index = 0;
		--Data_Flag.count;
		return 1;
	}else{
		return 0;
	}
}


/**************************************************************
函数名:Protocol_Slave_Init
功能:  初始化通讯解析
参数:  无
返回值:无
***************************************************************/
static void  Protocol_Slave_Init (void)
{
	uint8_t i;
	for(i=0;i<INSTURCT_ACK_LEN;i++)Insturct_ASK_Buffer[i] = 0;
	for(i=0;i<INSTURCT_ACK_LEN;i++)Insturct_ACK_Buffer[i] = 0;
	Insturct_ACK_Buffer[0] = PROTOCOL_ACK_HEAD_HIGH;
  Insturct_ACK_Buffer[1] = PROTOCOL_ACK_HEAD_LOW;
	Insturct_ACK_Buffer[2] = MY_AUTOCAL_ADDR;
	Protocol_ASK_index = 0;

}

/**************************************************************
函数名:Protocol_ACK_CRC_Tail
功能:  应答包尾部添加CRC
参数:  无
返回值:无
***************************************************************/
static void Protocol_ACK_CRC_Tail(void)
{
	uint16_t crc;
	crc = ComputeCRC16(Insturct_ACK_Buffer,INSTURCT_ACK_LEN - 2);
	Insturct_ACK_Buffer[INSTURCT_ACK_LEN - 1] = crc >> 8;
	Insturct_ACK_Buffer[INSTURCT_ACK_LEN - 2] = crc;
}






/**************************************************************
函数名:Protocol_CRC_Check
功能:  协议的CRC校验
参数:  无
返回值:1:校验成功
       0:校验失败
***************************************************************/
static uint8_t Protocol_CRC_Check(void)
{
	uint16_t crc_rec,crc_cal;
	//计算CRC
	crc_cal = ComputeCRC16(Insturct_ASK_Buffer,INSTURCT_ASK_LEN - 2);
	crc_rec = (uint16_t)Insturct_ASK_Buffer[INSTURCT_ASK_LEN-1]<<8 | Insturct_ASK_Buffer[INSTURCT_ASK_LEN-2];
	if(crc_cal != crc_rec)                                   //CRC和计算值不等
	{
		return 0;
	}
	return 1;
}


/**************************************************************
函数名:Protocol_ACK
功能:  发送应答
参数:  无
返回值:无
***************************************************************/
static void Protocol_ACK(void)
{
	Protocol_ACK_CRC_Tail();
	rt_uart_write(Insturct_ACK_Buffer,INSTURCT_ACK_LEN);
	
}

/**************************************************************
函数名:Autocal_Output_Start
功能:  校准输出函数
参数:  index  校准点的索引
返回值:无
***************************************************************/
static void Autocal_Output_Start(uint8_t index)
{
	switch(index)
	{
		// ACW --------------------------------------------------------->
		case 1:// ACW 100V
			dyj_output(ACW,DA_ACW_100V);
			Relay_ON(ACW_DCW_IR);
			
			break;
		case 2:// ACW 2000V
			dyj_output(ACW,DA_ACW_2000V);
			Relay_ON(ACW_DCW_IR);

			break;
		case 3:// ACW 5000V
			dyj_output(ACW,DA_ACW_5000V);
			Relay_ON(ACW_DCW_IR);

			break;
		case 4:// 300uA
			Sampling_Relay_State_CHange(DC_200uA);
			AC_Output_Enable();
			AC_SetVoltage(100,60);
			Relay_ON(ACW_DCW_IR);
		
			break;
		case 5:// 3mA
			Sampling_Relay_State_CHange(DC_2mA);
			AC_Output_Enable();
			AC_SetVoltage(DA_ACW_CURCAL_VOL,60);
			Relay_ON(ACW_DCW_IR);

			break;
		case 6:// 30mA
			Sampling_Relay_State_CHange(DC_20mA);
			AC_Output_Enable();
			AC_SetVoltage(DA_ACW_CURCAL_VOL,60);
			Relay_ON(ACW_DCW_IR);
		
			break;
		case 7:// 100mA
			Sampling_Relay_State_CHange(DC_100mA);
			AC_Output_Enable();
			AC_SetVoltage(DA_ACW_CURCAL_VOL,60);
			Relay_ON(ACW_DCW_IR);

			break;
		case 8:// ARC

			break;
		
		
		// DCW --------------------------------------------------------->
		case 9: // 100v
			dyj_output(DCW,DA_DCW_100V);
			Relay_ON(ACW_DCW_IR);

			break;
		case 10: // 2000v
			dyj_output(DCW,DA_DCW_2000V);
			Relay_ON(ACW_DCW_IR);

			break;
		case 11: // 6000v
			dyj_output(DCW,DA_DCW_6000V);
			Relay_ON(ACW_DCW_IR);

			break;
		case 12:// 3uA
			Sampling_Relay_State_CHange(DC_2uA);
			DC_Output_Enable();
			DC_SetVoltage(DA_DCW_CURCAL_VOL);
			Relay_ON(ACW_DCW_IR);

			break;
		case 13: // 30uA
			Sampling_Relay_State_CHange(DC_20uA);
			DC_Output_Enable();
			DC_SetVoltage(DA_DCW_CURCAL_VOL);
			Relay_ON(ACW_DCW_IR);

			break;
		case 14:// 300uA
			Sampling_Relay_State_CHange(DC_200uA);
			DC_Output_Enable();
			DC_SetVoltage(100);
			Relay_ON(ACW_DCW_IR);

			break;
		case 15:// 3mA
			Sampling_Relay_State_CHange(DC_2mA);
			DC_Output_Enable();
			DC_SetVoltage(DA_DCW_CURCAL_VOL);
			Relay_ON(ACW_DCW_IR);

			break;
		case 16:// 30mA
			Sampling_Relay_State_CHange(DC_20mA);
			DC_Output_Enable();
			DC_SetVoltage(DA_DCW_CURCAL_VOL);
			Relay_ON(ACW_DCW_IR);

			break;
		case 17:// 100mA
			Sampling_Relay_State_CHange(DC_100mA);
			DC_Output_Enable();
			DC_SetVoltage(DA_DCW_CURCAL_VOL);
			Relay_ON(ACW_DCW_IR);

			break;
		case 18://ARC

			break;
			
		
		// GR --------------------------------------------------------->
		case 19: // 3A
			Relay_ON(EXT_DRIVER_O7);
			Relay_ON(AMP_RELAY_1);
			dyj_output(GR,DA_GR_3A);
	
			break;
		case 20:
			Relay_ON(EXT_DRIVER_O7);
			Relay_ON(AMP_RELAY_1);
			dyj_output(GR,DA_GR_10A);

			break;
		case 21:
			Relay_ON(EXT_DRIVER_O7);
			Relay_ON(AMP_RELAY_1);
			dyj_output(GR,DA_GR_30A);

			break;
		

		
		// LC --------------------------------------------------------->
		case 22:
			dyj_output(LC,DA_LC_30V);

			break;
		case 23:
			dyj_output(LC,DA_LC_150V);

			break;
		case 24:
			dyj_output(LC,DA_LC_300V);

			break;
		case 25:
			dyj_output(LC_ASSIST,DA_LC_30VS);

			break;
		case 26:
			dyj_output(LC_ASSIST,DA_LC_150VS);

			break;
		case 27:
			dyj_output(LC_ASSIST,DA_LC_300VS);

			break;
		
		// MD_E
		case 28: // 300uA
			dyj_lc_relay(MD_E,0,1);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		case 29:
			dyj_lc_relay(MD_E,0,2);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		case 30:
			dyj_lc_relay(MD_E,0,3);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		
		case 31:
			dyj_lc_relay(MD_E,1,1);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		case 32:
			dyj_lc_relay(MD_E,1,2);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		case 33:
			dyj_lc_relay(MD_E,1,3);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		
		case 34:
			dyj_lc_relay(MD_E,2,1);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		case 35:
			dyj_lc_relay(MD_E,2,2);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		case 36:
			dyj_lc_relay(MD_E,2,3);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		
		case 37:
			dyj_lc_relay(MD_E,3,1);
			LC_Relay_Control(LC_OUT6,1,1);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);
	
			break;
		case 38:
			dyj_lc_relay(MD_E,3,2);
			LC_Relay_Control(LC_OUT6,1,1);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);
			
			break;
		case 39:
			dyj_lc_relay(MD_E,3,3);
			LC_Relay_Control(LC_OUT6,1,1);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		
		// MD_F
		case 40: // 300uA
			dyj_lc_relay(MD_F,0,1);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		case 41:
			dyj_lc_relay(MD_F,0,2);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);
			
			break;
		case 42:
			dyj_lc_relay(MD_F,0,3);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		
		case 43:
			dyj_lc_relay(MD_F,1,1);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		case 44:
			dyj_lc_relay(MD_F,1,2);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);
		
			break;
		case 45:
			dyj_lc_relay(MD_F,1,3);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		
		case 46:
			dyj_lc_relay(MD_F,2,1);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		case 47:
			dyj_lc_relay(MD_F,2,2);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		case 48:
			dyj_lc_relay(MD_F,2,3);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		
		case 49:
			dyj_lc_relay(MD_F,3,1);
			LC_Relay_Control(LC_OUT6,1,1);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		case 50:
			dyj_lc_relay(MD_F,3,2);
			LC_Relay_Control(LC_OUT6,1,1);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		case 51:
			dyj_lc_relay(MD_F,3,3);
			LC_Relay_Control(LC_OUT6,1,1);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		
		// MD-G
		case 52: // 300uA
			dyj_lc_relay(MD_G,0,1);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		case 53:
			dyj_lc_relay(MD_G,0,2);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		case 54:
			dyj_lc_relay(MD_G,0,3);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		
		case 55:
			dyj_lc_relay(MD_G,1,1);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		case 56:
			dyj_lc_relay(MD_G,1,2);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		case 57:
			dyj_lc_relay(MD_G,1,3);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		
		case 58:
			dyj_lc_relay(MD_G,2,1);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		case 59:
			dyj_lc_relay(MD_G,2,2);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
			
		case 60:
			dyj_lc_relay(MD_G,2,3);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		
		case 61:
			dyj_lc_relay(MD_G,3,1);
			LC_Relay_Control(LC_OUT6,1,1);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);

			break;
		case 62:
			dyj_lc_relay(MD_G,3,2);
			LC_Relay_Control(LC_OUT6,1,1);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);

			break;
		case 63:
			dyj_lc_relay(MD_G,3,3);
			LC_Relay_Control(LC_OUT6,1,1);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);	

			break;
		
		case 64:

			dyj_lc_relay(MD_G,3,3);
			LC_Relay_Control(LC_OUT6,1,1);
			LC_4051_D15_SELECT(1);
			LC_Main_Output_Enable();
			LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
			Relay_ON(EXT_DRIVER_O8);	

			break;
	}
}


/**************************************************************
函数名:Autocal_Output_Stop
功能:  校准关闭输出函数
参数:  index  校准点的索引
返回值:无
***************************************************************/
static void Autocal_Output_Stop(uint8_t index)
{
	switch(index)
	{
		// ACW --------------------------------------------------------->
		case 1:// ACW 100V
			dyj_output(ACW,0);
			Relay_OFF(ACW_DCW_IR);
	
			break;
		case 2:// ACW 2000V
			dyj_output(ACW,0);
			Relay_OFF(ACW_DCW_IR);

			break;
		case 3:// ACW 5000V
			dyj_output(ACW,0);
			Relay_OFF(ACW_DCW_IR);

			break;
		case 4:// 300uA
			AC_SetVoltage(0,60);
			AC_Output_Disable();
			Relay_OFF(ACW_DCW_IR);
		
			break;
		case 5:// 3mA
			AC_SetVoltage(0,60);
			AC_Output_Disable();
			Relay_OFF(ACW_DCW_IR);

			
			break;
		case 6:// 30mA
			AC_SetVoltage(0,60);
			AC_Output_Disable();
			Relay_OFF(ACW_DCW_IR);


			break;
		case 7:// 100mA
			AC_SetVoltage(0,60);
			AC_Output_Disable();
			Relay_OFF(ACW_DCW_IR);

			break;
		case 8:// ARC

			break;
		
		
		// DCW --------------------------------------------------------->
		case 9: // 100v
			dyj_output(DCW,0);
			Relay_OFF(ACW_DCW_IR);

			break;
		case 10: // 2000v
			dyj_output(DCW,0);
			Relay_OFF(ACW_DCW_IR);
			break;
		case 11: // 6000v
			dyj_output(DCW,0);
			Relay_OFF(ACW_DCW_IR);

			break;
		case 12:// 3uA
			DC_SetVoltage(0);
			DC_Output_Disable();
			Relay_OFF(ACW_DCW_IR);


			break;
		case 13: // 30uA
			DC_SetVoltage(0);
			DC_Output_Disable();
			Relay_OFF(ACW_DCW_IR);

			break;
		case 14:// 300uA
			DC_SetVoltage(0);
			DC_Output_Disable();
			Relay_OFF(ACW_DCW_IR);


			break;
		case 15:// 3mA
			DC_SetVoltage(0);
			DC_Output_Disable();
			Relay_OFF(ACW_DCW_IR);
		
			break;
		case 16:// 30mA
			DC_SetVoltage(0);
			DC_Output_Disable();
			Relay_OFF(ACW_DCW_IR);

	
			break;
		case 17:// 100mA
			DC_SetVoltage(0);
			DC_Output_Disable();
			Relay_OFF(ACW_DCW_IR);
			
			break;
		case 18://ARC

			break;
			
		
		// GR --------------------------------------------------------->
		case 19: // 3A
			dyj_output(GR,0);
			Relay_OFF(EXT_DRIVER_O7);
			Relay_OFF(AMP_RELAY_1);
		
			break;
		case 20:
			dyj_output(GR,0);
			Relay_OFF(EXT_DRIVER_O7);
			Relay_OFF(AMP_RELAY_1);

			break;
		case 21:
			dyj_output(GR,0);
			Relay_OFF(EXT_DRIVER_O7);
			Relay_OFF(AMP_RELAY_1);

			break;
		

		
		// LC --------------------------------------------------------->
		case 22:
			dyj_output(LC,0);

			
			break;
		case 23:
			dyj_output(LC,0);

			break;
		case 24:
			dyj_output(LC,0);
			
			break;
		case 25:
			dyj_output(LC_ASSIST,0);

			break;
		case 26:
			dyj_output(LC_ASSIST,0);

			break;
		case 27:
			dyj_output(LC_ASSIST,0);

			break;
		
		// MD_E
		case 28: // 300uA
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 29:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 30:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		
		case 31:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 32:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 33:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		
		case 34:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 35:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 36:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);

			
			break;
		
		case 37:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 38:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 39:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		
		// MD_F
		case 40: // 300uA
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 41:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 42:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		
		case 43:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 44:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 45:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		
		case 46:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 47:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 48:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		
		case 49:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 50:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 51:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		
		// MD-G
		case 52: // 300uA
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 53:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 54:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);

			
			break;
		
		case 55:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 56:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 57:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		
		case 58:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 59:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
			
		case 60:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		
		case 61:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);

			
			break;
		case 62:
			LC_Main_Voltage_Set(0,60);
			Relay_OFF(RET_GND_SELECT);	
			LC_Main_Output_Disable();
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		case 63:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
		
		case 64:
			LC_Main_Voltage_Set(0,60);
			LC_Main_Output_Disable();
			Relay_OFF(RET_GND_SELECT);
			Relay_OFF(EXT_DRIVER_O8);
			
			break;
	}
}

/**************************************************************
函数名:Autocal_Save
功能:  保存校准数据
参数:  index  校准点的索引
       value  校准数据 
返回值:无
***************************************************************/

static void Autocal_Save(uint8_t index,uint32_t value)
{
	switch(index)
	{
		// ACW --------------------------------------------------------->
		case 1:// ACW 100V
			dyj_save_ad(ACW,CAL_VOL,1,value);
		
			break;
		case 2:// ACW 2000V
			dyj_save_ad(ACW,CAL_VOL,2,value);
			
			break;
		case 3:// ACW 5000V
			dyj_save_ad(ACW,CAL_VOL,3,value);
			
			break;
		case 4:// 300uA
			dyj_save_ad(ACW,CAL_CUR,1,value);
			
			break;
		case 5:// 3mA
			dyj_save_ad(ACW,CAL_CUR,2,value/10);

			break;
		case 6:// 30mA
			dyj_save_ad(ACW,CAL_CUR,3,value/100);

			break;
		case 7:// 100mA
			dyj_save_ad(ACW,CAL_CUR,4,value/100);

			break;
		case 8:// ARC
			//dyj_save_ad(ACW,CAL_ARC,4,value);
			break;

		
		// DCW --------------------------------------------------------->
		case 9: // 100v
			dyj_save_ad(DCW,CAL_VOL,1,value);
		
			break;
		case 10: // 2000v
			dyj_save_ad(DCW,CAL_VOL,2,value);
			
			break;
		case 11: // 6000v
			dyj_save_ad(DCW,CAL_VOL,3,value);
			
			break;
		case 12:// 3uA
			dyj_save_ad(DCW,CAL_CUR,1,value*100);

			break;
		case 13: // 30uA
			dyj_save_ad(DCW,CAL_CUR,2,value*10);

			break;
		case 14:// 300uA
			dyj_save_ad(DCW,CAL_CUR,3,value);

			break;
		case 15:// 3mA
			dyj_save_ad(DCW,CAL_CUR,4,value/10);

			break;
		case 16:// 30mA
			dyj_save_ad(DCW,CAL_CUR,5,value/100);

			break;
		case 17:// 100mA
			dyj_save_ad(DCW,CAL_CUR,6,value/100);

			break;
		case 18://ARC
			//dyj_save_ad(DCW,CAL_ARC,1,value);
			break;
		
		
		// GR --------------------------------------------------------->
		case 19: // 3A
			dyj_save_ad(GR,CAL_VOL,1,value);
		
			break;
		case 20:
			dyj_save_ad(GR,CAL_VOL,2,value);
			
			break;
		case 21:
			dyj_save_ad(GR,CAL_VOL,3,value);
		
			break;
		
		// LC --------------------------------------------------------->
		case 22:
			dyj_save_ad(LC,CAL_VOL,1,value);
			
			break;
		case 23:
			dyj_save_ad(LC,CAL_VOL,2,value);

			break;
		case 24:
			dyj_save_ad(LC,CAL_VOL,3,value);
			
			break;
		case 25:
			dyj_save_ad(LC_ASSIST,CAL_VOLS,1,value);

			break;
		case 26:
			dyj_save_ad(LC_ASSIST,CAL_VOLS,2,value);

			break;
		case 27:
			dyj_save_ad(LC_ASSIST,CAL_VOLS,3,value);

			break;
		
		// MD_E
		case 28: // 300uA
			dyj_save_lc_cur_ad(MD_E,0,1,value);
			
			break;
		case 29:
			dyj_save_lc_cur_ad(MD_E,0,2,value);
			
			break;
		case 30:
			dyj_save_lc_cur_ad(MD_E,0,3,value);

			break;
		
		case 31:
			dyj_save_lc_cur_ad(MD_E,1,1,value);

			break;
		case 32:
			dyj_save_lc_cur_ad(MD_E,1,2,value);

			break;
		case 33:
			dyj_save_lc_cur_ad(MD_E,1,3,value);

			break;
		
		case 34:
			dyj_save_lc_cur_ad(MD_E,2,1,value);

			break;
		case 35:
			dyj_save_lc_cur_ad(MD_E,2,2,value);

			break;
		case 36:
			dyj_save_lc_cur_ad(MD_E,2,3,value);

			break;
		
		case 37:
			dyj_save_lc_cur_ad(MD_E,3,1,value);

			break;
		case 38:
			dyj_save_lc_cur_ad(MD_E,3,2,value);

			break;
		case 39:
			dyj_save_lc_cur_ad(MD_E,3,3,value);

			break;
		
		// MD_F
		case 40: // 300uA
			dyj_save_lc_cur_ad(MD_F,0,1,value);

			break;
		case 41:
			dyj_save_lc_cur_ad(MD_F,0,2,value);

			break;
		case 42:
			dyj_save_lc_cur_ad(MD_F,0,3,value);

			break;
		
		case 43:
			dyj_save_lc_cur_ad(MD_F,1,1,value);

			break;
		case 44:
			dyj_save_lc_cur_ad(MD_F,1,2,value);

			break;
		case 45:
			dyj_save_lc_cur_ad(MD_F,1,3,value);
			
			break;
		
		case 46:
			dyj_save_lc_cur_ad(MD_F,2,1,value);
			
			break;
		case 47:
			dyj_save_lc_cur_ad(MD_F,2,2,value);
			
			break;
		case 48:
			dyj_save_lc_cur_ad(MD_F,2,3,value);

			break;
		
		case 49:
			dyj_save_lc_cur_ad(MD_F,3,1,value);

			break;
		case 50:
			dyj_save_lc_cur_ad(MD_F,3,2,value);

			break;
		case 51:
			dyj_save_lc_cur_ad(MD_F,3,3,value);

			break;
		
		// MD-G
		case 52: // 300uA
			dyj_save_lc_cur_ad(MD_G,0,1,value);

			break;
		case 53:
			dyj_save_lc_cur_ad(MD_G,0,2,value);

			break;
		case 54:
			dyj_save_lc_cur_ad(MD_G,0,3,value);

			break;
		
		case 55:
			dyj_save_lc_cur_ad(MD_G,1,1,value);

			break;
		case 56:
			dyj_save_lc_cur_ad(MD_G,1,2,value);

			break;
		case 57:
			dyj_save_lc_cur_ad(MD_G,1,3,value);

			break;
		
		case 58:
			dyj_save_lc_cur_ad(MD_G,2,1,value);

			break;
		case 59:
			dyj_save_lc_cur_ad(MD_G,2,2,value);

			break;
			
		case 60:
			dyj_save_lc_cur_ad(MD_G,2,3,value);

			break;
		
		case 61:
			dyj_save_lc_cur_ad(MD_G,3,1,value);

			break;
		case 62:
			dyj_save_lc_cur_ad(MD_G,3,2,value);
	
			break;
		case 63:
			dyj_save_lc_cur_ad(MD_G,3,3,value);

			break;
		
		case 64:
			dyj_save_lc_selv_ad(value);

			break;
	}
}


//CRC16CCITT(1021)的16字表长查表程序
static const uint16_t CRC16_TABLE[256]={
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
	0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
	0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
	0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
	0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
	0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
	0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
	0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
	0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
	0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
	0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
	0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
	0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
	0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
	0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
	0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
	0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
	0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
	0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
	0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
	0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
	0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
	0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

/**************************************************** 
**函数原型:   INT16U ComputeCRC16(INT8U *, INT8U)  	*
**功    能:   CRC计算函数                	  				*
**入口参数:   databuf:	数据指针          					*
*			        length:	  数据长度          					*
**返 回 值:     		                  							*
*             CRC16值               								*
****************************************************/
static uint16_t ComputeCRC16(uint8_t *databuf, uint8_t length)
{
	uint16_t crc = 0;
	uint8_t indexTab;
	while(length--)
	{
		indexTab = *databuf ^ (crc >> 8);		
		databuf++;
		crc <<= 8;
		crc ^= CRC16_TABLE[indexTab];			
	}
	return crc;
}


static const CAL_POINT_FRAME Cal_Point[] = {
	{0  ,   AUTOCAL_AC_V  ,      0},                                        //校准点从1开始
	{1  ,   AUTOCAL_AC_V  ,    100},
	{2  ,   AUTOCAL_AC_V  ,   2000},
	{3  ,   AUTOCAL_AC_V  ,   5000},
	{4  ,   AUTOCAL_AC_A  ,   1000},
	{5  ,   AUTOCAL_AC_A  ,   10000},
	{6  ,   AUTOCAL_AC_A  ,   100000},
	{7  ,   AUTOCAL_PASS  ,   1000000},
	{8  ,   AUTOCAL_PASS  ,   0},
	{9  ,   AUTOCAL_DC_V  ,   100},
	{10 ,   AUTOCAL_DC_V  ,   2000},
	{11 ,   AUTOCAL_DC_V  ,   6000},
	{12  ,  AUTOCAL_PASS  ,   19},
	{13 ,   AUTOCAL_PASS  ,   199},
	{14 ,   AUTOCAL_DC_A  ,   1000},
	{15  ,  AUTOCAL_DC_A  ,   19999},
	{16 ,   AUTOCAL_DC_A  ,   199999},
	{17 ,   AUTOCAL_PASS  ,   1000000},
};
