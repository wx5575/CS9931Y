
#include "stm32f4xx.h"
#include "bsp_button.h"
#include <rtthread.h>
#include "CS99xx.h"
#include "memorymanagement.h"
#include "Test_Sched.h"
#include "PLC.h"
// #include <rtgui/event.h>
// #include <rtgui/rtgui_server.h>

	/* 按键口对应的RCC时钟 */
	#define RCC_ALL_KEY 	(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOH)

	#define GPIO_PORT_EXT1  	GPIOA
	#define GPIO_PIN_EXT1	    GPIO_Pin_8

	#define GPIO_PORT_EXT2  	GPIOA
	#define GPIO_PIN_EXT2	    GPIO_Pin_9

	#define GPIO_PORT_EXT3  	GPIOA
	#define GPIO_PIN_EXT3	    GPIO_Pin_10

	#define GPIO_PORT_IN1    	GPIOA
	#define GPIO_PIN_IN1   	 	GPIO_Pin_12

	#define GPIO_PORT_IN2    	GPIOH
	#define GPIO_PIN_IN2   	 	GPIO_Pin_13
	
	#define GPIO_PORT_IN3    	GPIOH
	#define GPIO_PIN_IN3   	 	GPIO_Pin_14
	
	#define GPIO_PORT_IN4    	GPIOH
	#define GPIO_PIN_IN4   	 	GPIO_Pin_15



	#define	KEY_PORT_ST			GPIO_PORT_EXT2
	#define	KEY_PIN_ST			GPIO_PIN_EXT2
	
	#define KEY_PORT_CLK		GPIO_PORT_EXT1
	#define	KEY_PIN_CLK			GPIO_PIN_EXT1
	
	#define	KEY_PORT_DATA		GPIO_PORT_EXT3
	#define	KEY_PIN_DATA		GPIO_PIN_EXT3
	

#define		KEY_DLY				0

static BUTTON_T s_Btn;

static KEY_FIFO_T s_Key;		/* 按键FIFO变量,结构体 */

static void bsp_InitButtonVar(void);
// static void bsp_InitButtonHard(void);
static void bsp_DetectButton(BUTTON_T *_pBtn);

extern uint8_t Get_Test_Warning_State(void);

rt_uint8_t keym_disable;
u16 KEY_BUFFER=0;
u32 LED_SINGLE=0;
/* 邮箱控制块*/
struct rt_mailbox key_mb;
/* 用于放邮件的内存池*/
static char key_mb_pool[64];

static void key_delay(u32 t)
{
	while(t--);
}
/*
 * 函数名：key_write
 * 描述  ：写
 * 输入  ：无
 * 输出  ：无
 */
static void key_write(void)
{
	unsigned char i;
	unsigned int pos=0x8000;
	
	register rt_base_t level;

  /* disable interrupt */
  level = rt_hw_interrupt_disable();
	
//	rt_enter_critical();
	GPIO_ResetBits(KEY_PORT_ST,KEY_PIN_ST);
	for(i=0;i<16;i++)
	{	
		GPIO_ResetBits(KEY_PORT_CLK,KEY_PIN_CLK);
		if(KEY_BUFFER & pos)
		{
			GPIO_SetBits(KEY_PORT_DATA,KEY_PIN_DATA);
		}
		else
		{
			GPIO_ResetBits(KEY_PORT_DATA,KEY_PIN_DATA);
		}
		key_delay(KEY_DLY);
		GPIO_SetBits(KEY_PORT_CLK,KEY_PIN_CLK);	
		key_delay(KEY_DLY);
		pos=pos>>1;
	}
	GPIO_SetBits(KEY_PORT_ST,KEY_PIN_ST);
//	rt_exit_critical();
	rt_hw_interrupt_enable(level);
}

void bsp_display(u32 cmd,u8 status)
{
	u32 cmd_bk   = cmd;
	u8 status_bk = status;
	switch(status)
	{
		case 2:
			if(cmd & 0xffff0000)
			{
				LED_SINGLE ^= cmd;
				if(LED_SINGLE & LED_TEST)				
					GPIO_SetBits(GPIOI,GPIO_Pin_2);
				else
					GPIO_ResetBits(GPIOI,GPIO_Pin_2);
			}
			cmd &= 0xffff;
			if(cmd)
			{
				KEY_BUFFER ^= cmd;
				key_write();
			}
			break;
		default:
			if(cmd & 0xffff0000)
			{
				LED_SINGLE = status!=0 ? (LED_SINGLE | cmd):(LED_SINGLE & ~cmd);
				if(LED_SINGLE & LED_TEST)				
					GPIO_SetBits(GPIOI,GPIO_Pin_2);
				else
					GPIO_ResetBits(GPIOI,GPIO_Pin_2);
			}
			cmd &= 0xffff;
			if(cmd)
			{
				KEY_BUFFER = status!=0 ? (KEY_BUFFER | cmd):(KEY_BUFFER & ~cmd);
				key_write();
			}
			break;
	}
	//在此处添加PLC的相关操作
	{
		switch(cmd_bk){
		
			case LED_TEST:
			{
				switch(status_bk){
					case 0:
						PLC_Testing_Out(0);
					break;
					
					case 1:
						//没有操作，已经在开始时处理过
					break;
					case 2:
//						PLC_Testing_Out(1);
					break;
					default:
						
					break;
				}
			}
			break;
			
			case LED_PASS:
			{
				switch(status_bk){
					case 0:
						PLC_Pass_Out(0);
					break;
					
					case 1:
						PLC_Pass_Out(1);
					break;
					default:
						
					break;
				}
			}
			break;
			
			case LED_FAIL:
			{
				switch(status_bk){
					case 0:
						PLC_Fail_Out(0);
					break;
					
					case 1:
						PLC_Fail_Out(1);
					break;
					default:
						
					break;
				}
			}
			break;
			default:
			
			break;
		}
	}
}
/*
 * 函数名：key_scan_read
 * 描述  ：写
 * 输入  ：无
 * 输出  ：无
 */
static u8 key_scan_read(void)
{
	unsigned char key=0;
	key|=(GPIO_ReadInputDataBit(GPIO_PORT_IN1,GPIO_PIN_IN1)!=0?0x01:0);
	key|=(GPIO_ReadInputDataBit(GPIO_PORT_IN2,GPIO_PIN_IN2)!=0?0x02:0);
	key|=(GPIO_ReadInputDataBit(GPIO_PORT_IN3,GPIO_PIN_IN3)!=0?0x04:0);
	key|=(GPIO_ReadInputDataBit(GPIO_PORT_IN4,GPIO_PIN_IN4)!=0?0x08:0);
	return key;
}

/*
 * 函数名：key_scan
 * 描述  ：写
 * 输入  ：无
 * 输出  ：无
 */

static u32 key_scan(void)
{
	unsigned char i;
	unsigned char pos=0x80;
	u32 key=0;
	
	for(i=0;i<8;i++)
	{
		KEY_BUFFER &= 0xff00;
		KEY_BUFFER |= ((~pos)&0x00ff);
		key_write();
		key |= (key_scan_read() << (i*4));
		pos >>= 1;
	}
	
	switch(key)
	{
		case 0xefffffff:
			return 1;
		
		case 0xfeffffff:
			return 2;
		case 0xffefffff:
			return 3;
		case 0xfffeffff:
			return 4;
		case 0xffffefff:
			return 5;
		case 0xfffffeff:
			return 6;
		case 0xffffffef:
			return 7;
		
		case 0xfffffffe:
			return 8;
		case 0xdfffffff:
			return 9;
		case 0xfdffffff:
			return 10;
		case 0xffdfffff:
			return 11;
		
		case 0xfffdffff://0
			return 12;
		case 0xffffdfff://1
			return 13;
		case 0xbfffffff://2
			return 14;
		case 0xffffbfff://3
			return 15;	
		case 0xfffffdff://4
			return 16;
		case 0xfbffffff://5
			return 17;
		case 0xfffffbff://6
			return 18;
		case 0xffffffdf://7
			return 19;
		case 0xffbfffff://8
			return 20;
		case 0xffffffbf://9
			return 21;
		
		case 0xfffffffd://pos
			return 22;
		case 0xfffbffff://shift
			return 23;
		
		
		
		case 0xfffffffb://left
			return 24;
		case 0x7fffffff://up
			return 25;
		case 0xf7ffffff://down
			return 26;
		case 0xff7fffff://right
			return 27;
		
		default:
			return 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_InitButton
*	功能说明: 初始化按键
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitButton(void)
{
	bsp_InitButtonVar();		/* 初始化按键变量 */
// 	bsp_InitButtonHard();		/* 初始化按键硬件 */
}

/*
*********************************************************************************************************
*	函 数 名: bsp_PutKey
*	功能说明: 将1个键值压入按键FIFO缓冲区。可用于模拟一个按键。
*	形    参：_KeyCode : 按键代码
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_PutKey(uint8_t _KeyCode)
{
	s_Key.Buf[s_Key.Write] = _KeyCode;

	if (++s_Key.Write  >= KEY_FIFO_SIZE)
	{
		s_Key.Write = 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_GetKey
*	功能说明: 从按键FIFO缓冲区读取一个键值。
*	形    参：无
*	返 回 值: 按键代码
*********************************************************************************************************
*/
uint8_t bsp_GetKey(void)
{
	uint8_t ret;

	if (s_Key.Read == s_Key.Write)
	{
		return KEY_NONE;
	}
	else
	{
		ret = s_Key.Buf[s_Key.Read];

		if (++s_Key.Read >= KEY_FIFO_SIZE)
		{
			s_Key.Read = 0;
		}
		return ret;
	}
}


void SoftSendEXITKey(void)
{
	/* 键值放入按键FIFO */
// 	bsp_PutKey(KEY_DOWN_EXIT);
}
	

/*
*********************************************************************************************************
*	函 数 名: bsp_InitButtonHard
*	功能说明: 初始化按键硬件
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
// static void bsp_InitButtonHard(void)
// {
// 	GPIO_InitTypeDef GPIO_InitStructure;
// 			
// 	/* 第1步：打开GPIOA GPIOC GPIOD GPIOF GPIOG的时钟
// 	   注意：这个地方可以一次性全打开
// 	*/
// 	RCC_AHB1PeriphClockCmd(RCC_ALL_KEY, ENABLE);
// 	/* 第2步：配置所有的按键GPIO为浮动输入模式(实际上CPU复位后就是输入状态) */
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		/* 设为输入口 */
// 	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		/* 设为推挽模式 */
// 	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	/* 无需上下拉电阻 */
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/* IO口最大速度 */

// 	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_IN1;
// 	GPIO_Init(GPIO_PORT_IN1, &GPIO_InitStructure);
// 	
// 	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_IN2;
// 	GPIO_Init(GPIO_PORT_IN2, &GPIO_InitStructure);
// 	
// 	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_IN3;
// 	GPIO_Init(GPIO_PORT_IN3, &GPIO_InitStructure);
// 	
// 	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_IN4;
// 	GPIO_Init(GPIO_PORT_IN4, &GPIO_InitStructure);
// 	
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
// 	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式 */
// 	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 无需上下拉电阻 */
// 	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_EXT1;
// 	GPIO_Init(GPIO_PORT_EXT1, &GPIO_InitStructure);

// 	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_EXT2;
// 	GPIO_Init(GPIO_PORT_EXT2, &GPIO_InitStructure);
// 	
// 	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_EXT3;
// 	GPIO_Init(GPIO_PORT_EXT3, &GPIO_InitStructure);
// }
	
/*
*********************************************************************************************************
*	函 数 名: bsp_InitButtonVar
*	功能说明: 初始化按键变量
*	形    参：strName : 例程名称字符串
*			  strDate : 例程发布日期
*	返 回 值: 无
*********************************************************************************************************
*/
static void bsp_InitButtonVar(void)
{
	/* 对按键FIFO读写指针清零 */
	s_Key.Read = 0;
	s_Key.Write = 0;
	

	/* 初始化Up按键变量，支持按下、弹起、长按 */
	s_Btn.IsKeyDownFunc		= key_scan;					/* 判断按键按下的函数 */
	s_Btn.FilterTime		= BUTTON_FILTER_TIME;		/* 按键滤波时间 */
	s_Btn.LongTime			= 400;			             /* 长按时间 */
	s_Btn.Count				= s_Btn.FilterTime / 2;		/* 计数器设置为滤波时间的一半 */
	s_Btn.State				= KEY_NONE;					/* 按键缺省状态，0为未按下 */
	s_Btn.KeyCodeDown		= KEY_DOWN;					/* 按键按下的键值代码 */
	s_Btn.KeyCodeUp			= KEY_UP;					/* 按键弹起的键值代码 */
	s_Btn.KeyCodeLong		= KEY_LONG;					/* 按键被持续按下的键值代码 */
	s_Btn.RepeatSpeed		= 0;						/* 按键连发的速度，0表示不支持连发 */
	s_Btn.RepeatCount		= 0;						/* 连发计数器 */		

}

/*
*********************************************************************************************************
*	函 数 名: bsp_DetectButton
*	功能说明: 检测一个按键。非阻塞状态，必须被周期性的调用。
*	形    参：按键结构变量指针
*	返 回 值: 无
*********************************************************************************************************
*/
static void bsp_DetectButton(BUTTON_T *_pBtn)
{
	u8	key;
	static u32 key_mem;
	/* 如果没有初始化按键函数，则报错 */
// 	if (_pBtn->IsKeyDownFunc == 0)
// 	{
// 		return;//"Fault : DetectButton(), _pBtn->IsKeyDownFunc undefine";
// 	}

	key = _pBtn->IsKeyDownFunc();
	if(key == 0xff)return;
	if (key != 0)
	{
		if (_pBtn->Count < _pBtn->FilterTime)
		{
			_pBtn->Count = _pBtn->FilterTime;
		}
		else if(_pBtn->Count < 2 * _pBtn->FilterTime)
		{
			_pBtn->Count++;
		}
		else
		{
			if (_pBtn->State == 0)
			{
				_pBtn->State = 1;

				/* 发送按钮按下的消息 */
				if (_pBtn->KeyCodeDown > 0)
				{
					/* 键值放入按键FIFO */
					bsp_PutKey(_pBtn->KeyCodeDown | key);
					key_mem = key;
					
					buzzer(5);
				}
			}

			if (_pBtn->LongTime > 0)
			{
				if (_pBtn->LongCount < _pBtn->LongTime)
				{
					/* 发送按钮持续按下的消息 */
					if (++_pBtn->LongCount == _pBtn->LongTime)
					{
						/* 键值放入按键FIFO */
						bsp_PutKey(_pBtn->KeyCodeLong | key);						
					}
				}
				else
				{
					if (_pBtn->RepeatSpeed > 0)
					{
						if (++_pBtn->RepeatCount >= _pBtn->RepeatSpeed)
						{
							_pBtn->RepeatCount = 0;
							/* 常按键后，每隔10ms发送1个按键 */
							bsp_PutKey(_pBtn->KeyCodeDown | key);														
						}
					}
				}
			}
		}
	}
	else
	{
		if(_pBtn->Count > _pBtn->FilterTime)
		{
			_pBtn->Count = _pBtn->FilterTime;
		}
		else if(_pBtn->Count != 0)
		{
			_pBtn->Count--;
		}
		else
		{
			if (_pBtn->State == 1)
			{
				_pBtn->State = 0;

				/* 发送按钮弹起的消息 */
				if (_pBtn->KeyCodeUp > 0)
				{
					/* 键值放入按键FIFO */
					bsp_PutKey(_pBtn->KeyCodeUp | key_mem);			
				}
			}
		}

		_pBtn->LongCount = 0;
		_pBtn->RepeatCount = 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_KeyPro
*	功能说明: 检测所有按键。非阻塞状态，必须被周期性的调用。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_KeyPro(void)
{
// 	KEY_TEMP = key_scan();
	if(Test_Sched_Param.Test_Sched_State == TEST_SCHED_STATE_RUNNING)return;
	bsp_DetectButton(&s_Btn);	
}

















u8 keyf_down_flag=0;

/* language , panel(panel_flag) , Fn */
const char * keyf_name[2][10][10]={
	{
/*   主页  -> */		{"文~件"	,"步~骤"	,"系~统"	,"结~果"	,"帮~助"	,"关~于"},
/*   文件  -> */		{"存~贮"	,"新~建"	,"读~取"	,"编~辑"	,"删~除"	,"更~多"},
/*   步骤  -> */		{"详~细"	,"新~建"	,"删~除"	,"前~移"	,"后~移"	,"返~回"},
/*   系统  -> */		{"~"		,"~"		,"~"		,"~"		,"~"		,"返~回"},
/*   结果  -> */		{"详~情"	,"删~除"	,"统~计"	,"导~出"	,"跳~转"	,"返~回"},
/*   帮助  -> */		{"~"		,"~"		,"~"		,"~"		,"~"		,"返~回"},
/*   更多  -> */		{"~"		,"~"		,"~"		,"~"		,"~"		,"返~回"},
/* 导出文件-> */		{"导~入"	,"导~出"	,"~"	,"~"	,"~"	  ,"返~回"},
                    {"~"		,"~"		,"~"		,"~"		,"~"		,"返~回"},
/* 自动校准-> */    {"~"		,"~"		,"~"		,"~"		,"~"		,"返~回"},
	},
	{
					{"File",	"Step",	"System",	"Result",	"Help",	"About"},
					{"File",	"New",	"Read",		"Edit",	"Delete",	"More"},
					{"Detail",	"New",	"Delete",	"Prev",	"Next",	"Back"},
					{"~",		"~",	"~",		"~",	"~",	"Back"},
					{"Detail","Delete","Stat  ","Export","Jump  ","Back  "},
					{"~",		"~",	"~",		"~",	"~",	"Back"},
					{"~",		"~",	"~",		"~",	"~",	"Back"},
					{"Import",		"Export",	"~",		"~",	"~",	"Back"},
					{"~"		,"~"		,"~"		,"~"		,"~"		,"Back"},
					{"~"		,"~"		,"~"		,"~"		,"~"		,"Back"},
	}
};

u8		buzzer_timer = 10;
static void key_thread_entry(void *parameter)
{

	uint8_t rtKeyCode;	/* 按键代码 */
	uint8_t rtTemp;
	struct font_info_t font={&panel_key,BUTTON_BACKCOLOR,0x0,1,0,24};
		
	/* 初始化一个mailbox */
	rt_mb_init(&key_mb,"key_mb", key_mb_pool, sizeof(key_mb_pool)/4, RT_IPC_FLAG_FIFO); /* 采用FIFO方式进行线程等待*/
	
    while (1)
    {
		if(system_parameter_t.Com_lock == 0 && Get_Test_Warning_State() == 0)
		{
		bsp_KeyPro();  /* 按键处理程序，必须每10ms调用1次 */
		
		rtTemp = bsp_GetKey();
		if (rtTemp > 0)
		{
			rtKeyCode = rtTemp & 0x1f;
			
			if(rtTemp == (KEY_LOCK | KEY_DOWN))
			{
				system_parameter_t.key_lock ^= 1;
				rt_mb_send(&screen_mb, UPDATE_STATUS | STATUS_KEYLOCK_EVENT | (system_parameter_t.key_lock));
				if(system_parameter_t.psd.keylockmem_en !=0)
				{
					memory_systems_save();
				}
			}
			if(system_parameter_t.key_lock==0)
			{
				if(rtKeyCode >= KEY_F1 && rtKeyCode <= KEY_F6)
				{
	// 				rt_enter_critical();
	// 				font_info_set(&panel_key,BUTTON_BACKCOLOR,0x0,1,24);
					if((rtTemp & KEY_DOWN)&&(keyf_down_flag==0)&&(keym_disable&(0x80>>(rtKeyCode-KEY_F1)))==0)
					{
						keyf_down_flag = 1;
						rt_memcpy((void *)((u8 *)panel_key.data+(rtKeyCode-KEY_F1)*ExternSramKeySize),(void *)ExternSramKeyDownAddr,ExternSramKeySize);
						font_draw((120-rt_strlen(keyf_name[language][panel_flag][rtKeyCode-KEY_F1])*12)/2,
							23+(rtKeyCode-KEY_F1)*67,&font,
							keyf_name[language][panel_flag][rtKeyCode-KEY_F1]);
						rt_mb_send(&screen_mb, UPDATE_KEY);
						rt_mb_send(&key_mb, rtTemp);
					}
					else if((keyf_down_flag == 1) &&(keym_disable&(0x80>>(rtKeyCode-KEY_F1)))==0)
					{
						keyf_down_flag = 0;
						rt_memcpy((void *)((u8 *)panel_key.data+(rtKeyCode-KEY_F1)*ExternSramKeySize),(void *)ExternSramKeyUpAddr,ExternSramKeySize);
						font_draw((120-rt_strlen(keyf_name[language][panel_flag][rtKeyCode-KEY_F1])*12)/2,
							21+(rtKeyCode-KEY_F1)*67,&font,
							keyf_name[language][panel_flag][rtKeyCode-KEY_F1]);
						rt_mb_send(&screen_mb, UPDATE_KEY);
						rt_mb_send(&key_mb, rtTemp);
					}
	// 				rt_exit_critical();
				}else{
					rt_mb_send(&key_mb, rtTemp);
				}
				
			}
		}	
		}
		/* 蜂鸣器借用按键的10ms时间片 */
		if(buzzer_timer > 0)
			if(--buzzer_timer == 0)bsp_display(FMQ,0);
		/* wait next key press */
		rt_thread_delay(RT_TICK_PER_SECOND/100);
		key_write();
    }
}

void buzzer(u8 timer)
{
	if(Test_Sched_Param.Test_Sched_State == TEST_SCHED_STATE_RUNNING)return;
	bsp_display(FMQ,1);
	buzzer_timer = timer;
}

void buzzer_test(u8 timer)
{
	if(Test_Sched_Param.Test_Sched_State != TEST_SCHED_STATE_RUNNING)return;
	if(timer == 0)
	{
		bsp_display(FMQ,0);
		buzzer_timer = 0;
		return;
	}
	bsp_display(FMQ,1);
	buzzer_timer = timer;
}


void ui_key_updata(rt_uint8_t key_disable)
{
	u8 i;
	struct font_info_t font={&panel_key,BUTTON_BACKCOLOR,0x0,1,0,24};

	keym_disable = key_disable;
	for(i=0;i<6;i++)
		rt_memcpy((void *)(ExternSramKeyAddr+ExternSramKeySize*i),(void *)ExternSramKeyUpAddr,ExternSramKeySize);
// 	font_info_set(&panel_key,BUTTON_BACKCOLOR,0x0,1,24);
	
// 	rt_enter_critical();
	for(i=0;i<6;i++)
	{
		if(key_disable & (0x80>>i))font.fontcolor = 0XA514;
		else font.fontcolor = 0xffff;
		font_draw((120-rt_strlen(keyf_name[language][panel_flag][i])*12)/2,
						21+(i)*67,&font,
						keyf_name[language][panel_flag][i]);
	}
// 	rt_exit_critical();
	rt_mb_send(&screen_mb, UPDATE_KEY);
}

static rt_thread_t key_tid;
void rt_hw_key_init(void)
{
	bsp_InitButton();
    key_tid = rt_thread_create("key",
                               &key_thread_entry, RT_NULL,
                               512, 10, 10);

    if (key_tid != RT_NULL) rt_thread_startup(key_tid);
}
