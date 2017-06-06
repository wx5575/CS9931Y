/*
*********************************************************************************************************
*	                                  
*	模块名称 : 按键驱动模块
*	文件名称 : bsp_button.c
*	版    本 : V1.0
*	说    明 : 实现按键的检测，具有软件滤波机制，可以检测如下事件：
*				(1) 按键按下
*				(2) 按键弹起
*				(3) 长按键
*				(4) 长按时自动连发
*				(5) 组合键
*
*	修改记录 :
*		版本号  日期       作者    说明
*		v0.1    2009-12-27 armfly  创建该文件，ST固件库版本为V3.1.2
*		v1.0    2011-01-11 armfly  ST固件库升级到V3.4.0版本。
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "stm32f4xx.h"
#include "bsp_button.h"
#include <rtthread.h>
#include <rtgui/event.h>
#include <rtgui/rtgui_server.h>

	/* 按键口对应的RCC时钟 */
	#define RCC_ALL_KEY 	(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOH | RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOG)

	#define GPIO_PORT_K1    GPIOI
	#define GPIO_PIN_K1	    GPIO_Pin_8

	#define GPIO_PORT_K2    GPIOC
	#define GPIO_PIN_K2	    GPIO_Pin_13

	#define GPIO_PORT_K3    GPIOI
	#define GPIO_PIN_K3	    GPIO_Pin_11

	#define GPIO_PORT_K4    GPIOH
	#define GPIO_PIN_K4	    GPIO_Pin_2

	#define GPIO_PORT_K5    GPIOH
	#define GPIO_PIN_K5	    GPIO_Pin_3

	#define GPIO_PORT_K6    GPIOF
	#define GPIO_PIN_K6	    GPIO_Pin_11

	#define GPIO_PORT_K7    GPIOG
	#define GPIO_PIN_K7	    GPIO_Pin_7

	#define GPIO_PORT_K8    GPIOH
	#define GPIO_PIN_K8	    GPIO_Pin_15


static BUTTON_T s_BtnUp;		/* 摇杆UP键 */
static BUTTON_T s_BtnDown;		/* 摇杆DOWN键 */
static BUTTON_T s_BtnLeft;		/* 摇杆LEFT键 */
static BUTTON_T s_BtnRight;		/* 摇杆RIGHT键 */

static KEY_FIFO_T s_Key;		/* 按键FIFO变量,结构体 */

static void bsp_InitButtonVar(void);
static void bsp_InitButtonHard(void);
static void bsp_DetectButton(BUTTON_T *_pBtn);

/*
	定义函数判断按键是否按下，返回值1 表示按下，0表示未按下
*/

static uint8_t IsKeyDownUp(void) 		{if ((GPIO_PORT_K1->IDR & GPIO_PIN_K1) == 0) return 1; return 0;}
static uint8_t IsKeyDownDown(void) 		{if ((GPIO_PORT_K2->IDR & GPIO_PIN_K2) == 0) return 1; return 0;}
static uint8_t IsKeyDownLeft(void) 		{if ((GPIO_PORT_K3->IDR & GPIO_PIN_K3) == 0) return 1; return 0;}
static uint8_t IsKeyDownRight(void) 	{if ((GPIO_PORT_K4->IDR & GPIO_PIN_K4) == 0) return 1; return 0;}


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
	bsp_InitButtonHard();		/* 初始化按键硬件 */
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
	bsp_PutKey(KEY_DOWN_EXIT);
}
	

/*
*********************************************************************************************************
*	函 数 名: bsp_InitButtonHard
*	功能说明: 初始化按键硬件
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void bsp_InitButtonHard(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
			
	/* 第1步：打开GPIOA GPIOC GPIOD GPIOF GPIOG的时钟
	   注意：这个地方可以一次性全打开
	*/
	RCC_AHB1PeriphClockCmd(RCC_ALL_KEY, ENABLE);
	/* 第2步：配置所有的按键GPIO为浮动输入模式(实际上CPU复位后就是输入状态) */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		/* 设为输入口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 无需上下拉电阻 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/* IO口最大速度 */

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_K1;
	GPIO_Init(GPIO_PORT_K1, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_K2;
	GPIO_Init(GPIO_PORT_K2, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_K3;
	GPIO_Init(GPIO_PORT_K3, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_K4;
	GPIO_Init(GPIO_PORT_K4, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_K5;
	GPIO_Init(GPIO_PORT_K5, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_K6;
	GPIO_Init(GPIO_PORT_K6, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_K7;
	GPIO_Init(GPIO_PORT_K7, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_K8;
	GPIO_Init(GPIO_PORT_K8, &GPIO_InitStructure);
}
	
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
	s_BtnUp.IsKeyDownFunc	= IsKeyDownUp;				/* 判断按键按下的函数 */
	s_BtnUp.FilterTime		= BUTTON_FILTER_TIME;		/* 按键滤波时间 */
	s_BtnUp.LongTime		= BUTTON_LONG_TIME;			/* 长按时间 */
	s_BtnUp.Count			= s_BtnUp.FilterTime / 2;	/* 计数器设置为滤波时间的一半 */
	s_BtnUp.State			= 0;						/* 按键缺省状态，0为未按下 */
	s_BtnUp.KeyCodeDown		= KEY_DOWN_JOY_UP;			/* 按键按下的键值代码 */
	s_BtnUp.KeyCodeUp		= 0;						/* 按键弹起的键值代码 */
	s_BtnUp.KeyCodeLong		= 0;						/* 按键被持续按下的键值代码 */
	s_BtnUp.RepeatSpeed		= 10;						/* 按键连发的速度，0表示不支持连发 */
	s_BtnUp.RepeatCount		= 0;						/* 连发计数器 */		

	/* 初始化Down按键变量，支持按下、弹起、长按 */
	s_BtnDown.IsKeyDownFunc	= IsKeyDownDown;			/* 判断按键按下的函数 */
	s_BtnDown.FilterTime	= BUTTON_FILTER_TIME;		/* 按键滤波时间 */
	s_BtnDown.LongTime		= BUTTON_LONG_TIME;			/* 长按时间 */
	s_BtnDown.Count			= s_BtnDown.FilterTime / 2;	/* 计数器设置为滤波时间的一半 */
	s_BtnDown.State			= 0;						/* 按键缺省状态，0为未按下 */
	s_BtnDown.KeyCodeDown	= KEY_DOWN_JOY_DOWN;		/* 按键按下的键值代码 */
	s_BtnDown.KeyCodeUp		= 0;						/* 按键弹起的键值代码 */
	s_BtnDown.KeyCodeLong	= 0;						/* 按键被持续按下的键值代码 */
	s_BtnDown.RepeatSpeed	= 10;						/* 按键连发的速度，0表示不支持连发 */
	s_BtnDown.RepeatCount	= 0;						/* 连发计数器 */

	/* 初始化Left按键变量，支持按下、弹起、长按 */
	s_BtnLeft.IsKeyDownFunc	= IsKeyDownLeft;			/* 判断按键按下的函数 */
	s_BtnLeft.FilterTime	= BUTTON_FILTER_TIME;		/* 按键滤波时间 */
	s_BtnLeft.LongTime		= BUTTON_LONG_TIME;			/* 长按时间 */
	s_BtnLeft.Count			= s_BtnLeft.FilterTime / 2;	/* 计数器设置为滤波时间的一半 */
	s_BtnLeft.State			= 0;						/* 按键缺省状态，0为未按下 */
	s_BtnLeft.KeyCodeDown	= KEY_DOWN_JOY_LEFT;		/* 按键按下的键值代码 */
	s_BtnLeft.KeyCodeUp		= 0;						/* 按键弹起的键值代码 */
	s_BtnLeft.KeyCodeLong	= 0;						/* 按键被持续按下的键值代码 */
	s_BtnLeft.RepeatSpeed	= 0;						/* 按键连发的速度，0表示不支持连发 */
	s_BtnLeft.RepeatCount	= 0;						/* 连发计数器 */		

	/* 初始化Right按键变量，支持按下、弹起、长按 */
	s_BtnRight.IsKeyDownFunc= IsKeyDownRight;			/* 判断按键按下的函数 */
	s_BtnRight.FilterTime	= BUTTON_FILTER_TIME;		/* 按键滤波时间 */
	s_BtnRight.LongTime		= BUTTON_LONG_TIME;			/* 长按时间 */
	s_BtnRight.Count		= s_BtnRight.FilterTime / 2;/* 计数器设置为滤波时间的一半 */
	s_BtnRight.State		= 0;						/* 按键缺省状态，0为未按下 */
	s_BtnRight.KeyCodeDown	= KEY_DOWN_JOY_RIGHT;		/* 按键按下的键值代码 */
	s_BtnRight.KeyCodeUp	= 0;						/* 按键弹起的键值代码 */
	s_BtnRight.KeyCodeLong	= 0;						/* 按键被持续按下的键值代码 */
	s_BtnRight.RepeatSpeed	= 0;						/* 按键连发的速度，0表示不支持连发 */
	s_BtnRight.RepeatCount	= 0;						/* 连发计数器 */		

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
	/* 如果没有初始化按键函数，则报错 */
// 	if (_pBtn->IsKeyDownFunc == 0)
// 	{
// 		return;//"Fault : DetectButton(), _pBtn->IsKeyDownFunc undefine";
// 	}

	if (_pBtn->IsKeyDownFunc())
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
					bsp_PutKey(_pBtn->KeyCodeDown);
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
						bsp_PutKey(_pBtn->KeyCodeLong);						
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
							bsp_PutKey(_pBtn->KeyCodeDown);														
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
					bsp_PutKey(_pBtn->KeyCodeUp);			
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
	bsp_DetectButton(&s_BtnUp);			/* 摇杆UP键 */
	bsp_DetectButton(&s_BtnDown);		/* 摇杆DOWN键 */
	bsp_DetectButton(&s_BtnLeft);		/* 摇杆LEFT键 */
	bsp_DetectButton(&s_BtnRight);		/* 摇杆RIGHT键 */
}

static void key_thread_entry(void *parameter)
{
    struct rtgui_event_kbd kbd_event;
	uint8_t rtKeyCode;	/* 按键代码 */
	uint8_t rtTemp;
      

    /* init keyboard event */
    RTGUI_EVENT_KBD_INIT(&kbd_event);
    kbd_event.mod  = RTGUI_KMOD_NONE;
    kbd_event.unicode = 0;

    while (1)
    {
		bsp_KeyPro();  /* 按键处理程序，必须每10ms调用1次 */

		rtTemp = bsp_GetKey();
		if (rtTemp > 0)
		{
			rtKeyCode = rtTemp;
			switch(rtKeyCode)
			{
				case KEY_DOWN_JOY_UP:
					kbd_event.key  = RTGUIK_UP;
					kbd_event.type = RTGUI_KEYDOWN;
					break;
				case KEY_DOWN_JOY_DOWN:
					kbd_event.key  = RTGUIK_DOWN;
					kbd_event.type = RTGUI_KEYDOWN;
					break;
				case KEY_DOWN_JOY_LEFT:
					kbd_event.key  = RTGUIK_LEFT;
					kbd_event.type = RTGUI_KEYDOWN;
					break;
				case KEY_DOWN_JOY_RIGHT:
					kbd_event.key  = RTGUIK_RIGHT;
					kbd_event.type = RTGUI_KEYDOWN;
					break;
				default:
					kbd_event.key = RTGUIK_UNKNOWN;
					kbd_event.type = RTGUI_KEYDOWN;
					break;
			}
			/* post down event */
            rtgui_server_post_event(&(kbd_event.parent), sizeof(kbd_event));
		}
		else
		{
			kbd_event.key = RTGUIK_UNKNOWN;
			kbd_event.type = RTGUI_KEYDOWN;
		}		
        /* wait next key press */
        rt_thread_delay(1);
    }
}

static rt_thread_t key_tid;
void rt_hw_key_init(void)
{
	bsp_InitButton();
    key_tid = rt_thread_create("key",
                               &key_thread_entry, RT_NULL,
                               512, 30, 10);

    if (key_tid != RT_NULL) rt_thread_startup(key_tid);
}
