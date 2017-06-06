/*
 * Copyright(c) 2014,
 * All rights reserved
 * 文件名称：RA8875.C
 * 摘  要  ：主程序
 * 当前版本：V1.0，编写
 * 修改记录：
 */

/******************* 加载文件 *********************/
#include "RA8875.h"

/*
*********************************************************************************************************
*	函 数 名: RA8875_Delaly1us
*	功能说明: 延迟函数, 不准, 主要用于RA8875 PLL启动之前发送指令间的延迟
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void RA8875_Delaly1us(void)
{
	uint8_t i;

	for (i = 0; i < 10; i++);	/* 延迟, 不准 */
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_Delaly1ms
*	功能说明: 延迟函数.  主要用于RA8875 PLL启动之前发送指令间的延迟
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void RA8875_Delaly1ms(void)
{
	uint16_t i;

	for (i = 0; i < 5000; i++);	/* 延迟, 不准 */
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_WriteCmd
*	功能说明: 写RA8875指令寄存器
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void RA8875_WriteCmd(uint8_t _ucRegAddr)
{
	RA8875_REG = _ucRegAddr;	/* 设置寄存器地址 */
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_WriteData
*	功能说明: 写RA8875指令寄存器
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void RA8875_WriteData(uint8_t _ucRegValue)
{
	RA8875_RAM = _ucRegValue;	/* 设置寄存器地址 */
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_WriteReg
*	功能说明: 写RA8875寄存器. RA8875的寄存器地址和数据都是8bit的
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void RA8875_WriteReg(uint8_t _ucRegAddr, uint8_t _ucRegValue)
{
	RA8875_WriteCmd(_ucRegAddr);
	RA8875_WriteData(_ucRegValue);
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_SetCursor
*	功能说明: 设置写显存的光标位置（图形模式）
*	形    参:  _usX : X坐标; _usY: Y坐标
*	返 回 值: 无
*********************************************************************************************************
*/
static void RA8875_SetCursor(uint16_t _usX, uint16_t _usY)
{
	/* 设置内存写光标的坐标 【注意0x80-83 是光标图形的坐标】 */
	RA8875_WriteReg(0x46, _usX);
	RA8875_WriteReg(0x47, _usX >> 8);
	RA8875_WriteReg(0x48, _usY);
	RA8875_WriteReg(0x49, _usY >> 8);
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_SetDispWin
*	功能说明: 设置显示窗口，进入窗口显示模式。在窗口显示模式，连续写显存，光标会自动在设定窗口内进行递增
*	形    参:
*		_usX : 水平坐标
*		_usY : 垂直坐标
*		_usHeight: 窗口高度
*		_usWidth : 窗口宽度
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_SetDispWin(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth)
{

	uint16_t usTemp;

	/* 坐标系统示意图: （横屏）
			 -----------------------------
			|(0,0)                        |
			|     --------->              |
			|         |                   |
			|         |                   |
			|         |                   |
			|         V                   |
			|     --------->              |
			|                    (479,271)|
			 -----------------------------

		左上角是坐标零点, 扫描方向，先从左到右，再从上到下。

		如果需要做竖屏方式，你需要进行物理坐标和逻辑坐标的转换
	*/

	RA8875_WriteReg(0x30, _usX);
    RA8875_WriteReg(0x31, _usX >> 8);

	RA8875_WriteReg(0x32, _usY);
    RA8875_WriteReg(0x33, _usY >> 8);

	usTemp = _usWidth + _usX - 1;
	RA8875_WriteReg(0x34, usTemp);
    RA8875_WriteReg(0x35, usTemp >> 8);

	usTemp = _usHeight + _usY - 1;
	RA8875_WriteReg(0x36, usTemp);
    RA8875_WriteReg(0x37, usTemp >> 8);

	RA8875_SetCursor(_usX, _usY);

	/* 保存当前窗口信息，提高以后单色填充操作的效率.
	另外一种做法是通过读取0x30-0x37寄存器获得当前窗口，不过效率较低 */
// 	s_WinX = _usX;
// 	s_WinY = _usY;
// 	s_WinHeight = _usHeight;
// 	s_WinWidth = _usWidth;
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_ReadStatus
*	功能说明: 读RA8875状态寄存器
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
uint8_t RA8875_ReadStatus(void)
{
	uint8_t value;

	value = RA8875_REG;

	return value;
}

/*
*********************************************************************************************************
*	函 数 名: BTE_SetTarBlock
*	功能说明: 设置RA8875 BTE目标区块以及目标图层
*	形    参:
*			uint16_t _usX : 水平起点坐标
*			uint16_t _usY : 垂直起点坐标
*			uint16_t _usHeight : 区块高度
*			uint16_t _usWidth : 区块宽度
*			uint8_t _ucLayer : 0 图层1； 1 图层2
*	返 回 值: 无
*********************************************************************************************************
*/
static void BTE_SetTarBlock(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint8_t _ucLayer)
{
	/* 设置起点坐标 */
	RA8875_WriteReg(0x58, _usX);
	RA8875_WriteReg(0x59, _usX >> 8);

	RA8875_WriteReg(0x5A, _usY);
	if (_ucLayer == 0)	/* 图层2 */
	{
		RA8875_WriteReg(0x5B, _usY >> 8);
	}
	else
	{
		RA8875_WriteReg(0x5B, (1 << 7) | (_usY >> 8));	/* Bit7 表示图层， 0 图层1； 1 图层2*/
	}

	/* 设置区块宽度 */
	RA8875_WriteReg(0x5C, _usWidth);
	RA8875_WriteReg(0x5D, _usWidth >> 8);

	/* 设置区块高度 */
	RA8875_WriteReg(0x5E, _usHeight);
	RA8875_WriteReg(0x5F, _usHeight >> 8);
}

/*
*********************************************************************************************************
*	函 数 名: BTE_SetOperateCode
*	功能说明: 设定BTE 操作码和光栅运算码
*	形    参: _ucOperate : 操作码
*	返 回 值: 无
*********************************************************************************************************
*/
static void BTE_SetOperateCode(uint8_t _ucOperate)
{
	/*  设定BTE 操作码和光栅运算码  */
	RA8875_WriteReg(0x51, _ucOperate);
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_SetFrontColor
*	功能说明: 设定前景色
*	形    参: 颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_SetFrontColor(uint16_t _usColor)
{
	RA8875_WriteReg(0x63, (_usColor & 0xF800) >> 11);	/* R5  */
	RA8875_WriteReg(0x64, (_usColor & 0x07E0) >> 5);	/* G6 */
	RA8875_WriteReg(0x65, (_usColor & 0x001F));			/* B5 */
}

/*
*********************************************************************************************************
*	函 数 名: BTE_Start
*	功能说明: 启动BTE操作
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void BTE_Start(void)
{
// 	s_ucRA8875Busy = 1;
	/* RA8875_WriteReg(0x50, 0x80);  不能使用这个函数，因为内部已经操作了 s_ucRA8875Busy 标志 */
	RA8875_WriteCmd(0x50);	/* 设置寄存器地址 */
	RA8875_WriteData(0x80);	/* 写入寄存器值 */
}

/*
*********************************************************************************************************
*	函 数 名: BTE_Wait
*	功能说明: 等待BTE操作结束
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void BTE_Wait(void)
{
	while ((RA8875_ReadStatus() & 0x40) == 0x40);
// 	s_ucRA8875Busy = 0;
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_ClrScr
*	功能说明: 根据输入的颜色值清屏.RA8875支持硬件单色填充。该函数仅对当前激活的显示窗口进行清屏. 显示
*			 窗口的位置和大小由 RA8875_SetDispWin() 函数进行设置
*	形    参:  _usColor : 背景色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_ClrScr(uint16_t _usColor)
{
	/* 也可以通过0x30-0x37寄存器获得获得当前激活的显示窗口 */

	/* 单色填满功能, 中文pdf 第162页
	此功能使用于将选定特定区域画面清除或是塡入给定某种前景色，R8875 填入的单色设定为BTE 前景色。

	操作步骤:
		1. 设定目的图层和位置 REG[58h], [59h], [5Ah], [5Bh]
		2. 设定BTE 宽度和高度 REG[5Ch], [5Dh], [5Eh], [5Fh]
		3. 设定BTE 操作码和光栅运算码  REG[51h] Bit[3:0] = 0Ch
		4. 设定前景色  REG[63h], [64h], [65h]
		5. 开启BTE 功能  REG[50h] Bit7 = 1
		6. 检查状态缓存器 STSR Bit6，确认BTE 是否完成
	*/
	BTE_SetTarBlock(0, 0, 480, 800, 0);	/* 设置BTE位置和宽度高度以及目标图层（0或1） */
	BTE_SetOperateCode(0x0C);		/* 设定BTE 操作码和光栅运算码  REG[51h] Bit[3:0] = 0Ch */
	RA8875_SetFrontColor(_usColor);	/* 设置BTE前景色 */
	BTE_Start();					/* 开启BTE 功能 */
	BTE_Wait();						/* 等待操作结束 */
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_SetBackLight
*	功能说明: 配置RA8875芯片的PWM1相关寄存器，控制LCD背光
*	形    参:  _bright 亮度，0是灭，255是最亮
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_SetBackLight(uint8_t _bright)
{
	if (_bright == 0)
	{
		/* 关闭PWM, PWM1引脚缺省输出LOW  */
		RA8875_WriteReg(0x8A, 0 << 6);
	}
	else if (_bright == 255)	/* 最大亮度 */
	{
		/* 关闭PWM, PWM1引脚缺省输出HIGH */
		RA8875_WriteReg(0x8A, 1 << 6);
	}
	else
	{
		/* 使能PWM1, 进行占空比调节 */

		/* 	REG[8Ah] PWM1 Control Register (P1CR)

			Bit7 脉波宽度调变 (PWM1) 设定
				0 : 关闭，此状态下，PWM1 输出准位依照此缓存器Bit6 决定。
				1 : 开启。

			Bit6 PWM1 关闭时的准位
				0 : 当PWM 关闭或于睡眠模式时，PWM1 输出为”Low” 状态。
				1 : 当PWM 关闭或于睡眠模式时，PWM1 输出为”High” 状态。

			Bit5 保留

			Bit4 PWM1 功能选择
				0 : PWM1 功能。
				1 : PWM1 固定输出一频率为外部晶体振荡器Clock (Fin) 频率 1 /16 的Clock

			Bit3-0  PWM1 电路的频率来源选择PWM_CLK	【不是PWM输出频率】
				0000b : SYS_CLK / 1   , 1000b : SYS_CLK / 256
				0001b : SYS_CLK / 2   , 1001b : SYS_CLK / 512
				0010b : SYS_CLK / 4   , 1010b : SYS_CLK / 1024
				0011b : SYS_CLK / 8   , 1011b : SYS_CLK / 2048
				0100b : SYS_CLK / 16  , 1100b : SYS_CLK / 4096
				0101b : SYS_CLK / 32  , 1101b : SYS_CLK / 8192
				0110b : SYS_CLK / 64  , 1110b : SYS_CLK / 16384
				0111b : SYS_CLK / 128 , 1111b : SYS_CLK / 32768

				“SYS_CLK” 代表系统频率， 例如SYS_CLK 为20MHz， 当Bit[3:0] =0001b 时，PWM1 频率来源为10MHz。
				实际输出的PWM频率还需要除以 256，支持8位的分辨率。

				安富莱的4.3寸(480*272)模块，SYS_CLK =  68.75MHz
				安富莱的7.0寸(800*480)模块，SYS_CLK =  81.25MHz

				为了避免音频噪声，PWM频率可以选择
				（1） 低频100Hz
				（2） 高于 20KHz

				比如，Bit3-0为0011b时 SYS_CLK / 8，
					4.3寸 输出的PWM频率为 (68.75MHz / 8) / 256 = 33.56KHz
					7寸 输出的PWM频率为 (81.25MHz / 8) / 256 = 39.67KHz
		*/

		// RA8875_WriteReg(0x8A, (1 << 7) | 3);   5寸和7寸新屏可以用 3 ，高频PWM, 4.3寸不行
		RA8875_WriteReg(0x8A, (1 << 7) | 12);

		/* REG[8Bh] PWM1 Duty Cycle Register (P1DCR) */
		RA8875_WriteReg(0x8B, _bright);
	}
}


void RA8875_InitHard(void)
{
	u32 t;
	/* 等待硬件复位 */
	for(t=0;t<200;t++)
	{
		RA8875_Delaly1ms();
	}
	
	/* 初始化PLL.  晶振频率为25M */
	RA8875_WriteCmd(0x88);
	RA8875_Delaly1us();		/* 延迟1us */
	RA8875_WriteData(12);	/* PLLDIVM [7] = 0 ;  PLLDIVN [4:0] = 10 */

	RA8875_Delaly1ms();

	RA8875_WriteCmd(0x89);
	RA8875_Delaly1us();		/* 延迟1us */
	RA8875_WriteData(2);	/* PLLDIVK[2:0] = 2, 除以4 */

	/* RA8875 的内部系统频率 (SYS_CLK) 是结合振荡电路及PLL 电路所产生，频率计算公式如下 :
	  SYS_CLK = FIN * ( PLLDIVN [4:0] +1 ) / ((PLLDIVM+1 ) * ( 2^PLLDIVK [2:0] ))
			  = 25M * (12 + 1) / ((0 + 1) * (2 ^ 2))
			  = 81.25MHz
	*/

	/* REG[88h]或REG[89h]被设定后，为保证PLL 输出稳定，须等待一段「锁频时间」(< 100us)。*/
	RA8875_Delaly1ms();

	/*
		配置系统控制寄存器。 中文pdf 第18页:

		bit3:2 色彩深度设定 (Color Depth Setting)
			00b : 8-bpp 的通用TFT 接口， i.e. 256 色。
			1xb : 16-bpp 的通用TFT 接口， i.e. 65K 色。	 【选这个】

		bit1:0 MCUIF 选择
			00b : 8-位MCU 接口。
			1xb : 16-位MCU 接口。 【选这个】
	*/
	RA8875_WriteReg(0x10, (1 <<3 ) | (1 << 1));	/* 配置16位MCU并口，65K色 */

	/* REG[04h] Pixel Clock Setting Register (PCSR)
		bit7  PCLK Inversion
			0 : PDAT 是在PCLK 正缘上升 (Rising Edge) 时被取样。
			1 : PDAT 是在PCLK 负缘下降 (Falling Edge) 时被取样。
		bit1:0 PCLK 频率周期设定
			Pixel Clock ,PCLK 频率周期设定。
			00b: PCLK 频率周期= 系统频率周期。
			01b: PCLK 频率周期= 2 倍的系统频率周期。
			10b: PCLK 频率周期= 4 倍的系统频率周期。
			11b: PCLK 频率周期= 8 倍的系统频率周期。
	*/
	RA8875_WriteReg(0x04, 0x81);
	RA8875_Delaly1ms();

#if 1
	/* OTD9960 & OTA7001 设置 */
	RA8875_WriteReg(0x14, 0x63);
	RA8875_WriteReg(0x15, 0x00);
	RA8875_WriteReg(0x16, 0x03);
	RA8875_WriteReg(0x17, 0x03);
	RA8875_WriteReg(0x18, 0x0B);
	RA8875_WriteReg(0x19, 0xDF);
	RA8875_WriteReg(0x1A, 0x01);
	RA8875_WriteReg(0x1B, 0x1F);
	RA8875_WriteReg(0x1C, 0x00);
	RA8875_WriteReg(0x1D, 0x16);
	RA8875_WriteReg(0x1E, 0x00);
	RA8875_WriteReg(0x1F, 0x01);

	RA8875_WriteReg(0x20, 0x08);
#else	/* AT070TN92  setting */
	//Horizontal set
	//HDWR//Horizontal Display Width Setting Bit[6:0]
	//Horizontal display width(pixels) = (HDWR + 1)*8
	RA8875_WriteReg(0x14, 0x4F);
	RA8875_WriteReg(0x15, 0x05);

	//HNDR//Horizontal Non-Display Period Bit[4:0]
	//Horizontal Non-Display Period (pixels) = (HNDR + 1)*8
	RA8875_WriteReg(0x16, 0x0F);

	//HSTR//HSYNC Start Position[4:0]
	//HSYNC Start Position(PCLK) = (HSTR + 1)*8
	RA8875_WriteReg(0x17, 0x01);

	//HPWR//HSYNC Polarity ,The period width of HSYNC.
	//HSYNC Width [4:0]   HSYNC Pulse width(PCLK) = (HPWR + 1)*8
	RA8875_WriteReg(0x18, 0x00);

	//Vertical set
	//VDHR0 //Vertical Display Height Bit [7:0]
	//Vertical pixels = VDHR + 1
	RA8875_WriteReg(0x19, 0xDF);

	//VDHR1 //Vertical Display Height Bit [8]
	//Vertical pixels = VDHR + 1
	RA8875_WriteReg(0x1A, 0x01);

	//VNDR0 //Vertical Non-Display Period Bit [7:0]
	//Vertical Non-Display area = (VNDR + 1)
	RA8875_WriteReg(0x1B, 0x0A);

	//VNDR1 //Vertical Non-Display Period Bit [8]
	//Vertical Non-Display area = (VNDR + 1)
	RA8875_WriteReg(0x1C, 0x00);

	//VSTR0 //VSYNC Start Position[7:0]
	//VSYNC Start Position(PCLK) = (VSTR + 1)
	RA8875_WriteReg(0x1D, 0x0E);

	//VSTR1 //VSYNC Start Position[8]
	//VSYNC Start Position(PCLK) = (VSTR + 1)
	RA8875_WriteReg(0x1E, 0x00);

	//VPWR //VSYNC Polarity ,VSYNC Pulse Width[6:0]
	//VSYNC Pulse Width(PCLK) = (VPWR + 1)
	RA8875_WriteReg(0x1F, 0x01);
#endif


	/* 设置TFT面板的 DISP  引脚为高，使能面板. 安富莱TFT模块的DISP引脚连接到RA8875芯片的GP0X脚 */
	RA8875_WriteReg(0xC7, 0x01);	/* DISP = 1 */

	/* LCD 显示/关闭讯号 (LCD Display on) */
	RA8875_WriteReg(0x01, 0x80);

	/* 	REG[40h] Memory Write Control Register 0 (MWCR0)

		Bit 7	显示模式设定
			0 : 绘图模式。
			1 : 文字模式。

		Bit 6	文字写入光标/内存写入光标设定
			0 : 设定文字/内存写入光标为不显示。
			1 : 设定文字/内存写入光标为显示。

		Bit 5	文字写入光标/内存写入光标闪烁设定
			0 : 游标不闪烁。
			1 : 游标闪烁。

		Bit 4   NA

		Bit 3-2  绘图模式时的内存写入方向
			00b : 左 -> 右，然后上 -> 下。
			01b : 右 -> 左，然后上 -> 下。
			10b : 上 -> 下，然后左 -> 右。
			11b : 下 -> 上，然后左 -> 右。

		Bit 1 	内存写入光标自动增加功能设定
			0 : 当内存写入时光标位置自动加一。
			1 : 当内存写入时光标位置不会自动加一。

		Bit 0 内存读取光标自动增加功能设定
			0 : 当内存读取时光标位置自动加一。
			1 : 当内存读取时光标位置不会自动加一。
	*/
	RA8875_WriteReg(0x40, 0x00);	/* 选择绘图模式 */


	/* 	REG[41h] Memory Write Control Register1 (MWCR1)
		写入目的位置，选择图层1
	*/
	RA8875_WriteReg(0x41, 0x00);	/* 选择绘图模式, 目的为CGRAM */

	RA8875_SetDispWin(0, 0, 480, 800);

	RA8875_ClrScr(0);	/* 清屏，显示全黑 */
	
	RA8875_SetBackLight(255);	 /* 打开背光，设置为缺省亮度 */
	
}


void RA8875_DrawICO(u16 x,u16 y,u16 c,u16 bc,const unsigned char* data)
{
	rt_uint16_t i,j,chr;
	rt_uint16_t h=*(rt_uint16_t *)data,w=*((rt_uint16_t *)data+1);
	rt_uint8_t *p=(rt_uint8_t *)data+4,word_bytes = (w + 7) / 8;
    
	/* 设置图片的位置和大小， 即设置显示窗口 */
	RA8875_SetDispWin(x, y, h, w);
	RA8875_WriteCmd(0x02); 		/* 准备读写显存 */
	for (i = 0; i < h; i++)
    {
        rt_uint8_t *ptr = p + i * word_bytes;
        for (j = 0; j < w; j++)
        {
            if (j % 8 == 0)
                chr = *ptr++;
            if (chr & 0x80)
				RA8875_RAM = c;
            else
				RA8875_RAM = bc;
            chr <<= 1;
        }
    }
}


/*
*********************************************************************************************************
*	函 数 名: RA8875_DrawBMP
*	功能说明: 在LCD上显示一个BMP位图，位图点阵扫描次序:从左到右，从上到下
*	形    参:
*			_usX, _usY : 图片的坐标
*			_usHeight  :图片高度
*			_usWidth   :图片宽度
*			_ptr       :图片点阵指针
*	返 回 值: 无
*********************************************************************************************************
*/
u16 bmp_buf[800];
void RA8875_DrawBMP(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr)
{
#if 0
	uint32_t index = 0;
#endif
	const uint16_t *p;
	
	rt_enter_critical();
	
	/* 设置图片的位置和大小， 即设置显示窗口 */
	RA8875_SetDispWin(_usX, _usY, _usHeight, _usWidth);

// 	s_ucRA8875Busy = 1;

	RA8875_WriteCmd(0x02); 		/* 准备读写显存 */

	p = _ptr;
#if 1
	{
		u16 x,y;
		u16 *pbuf;
		for(y=0;y<_usHeight;y++)
		{
			pbuf=bmp_buf;
			for(x=0;x<_usWidth;x++)
				*pbuf++ = (*p++);
			pbuf=bmp_buf;
			for(x=0;x<_usWidth;x++)
				RA8875_RAM = (*pbuf++);
		}
				
	}
#else
	for (index = 0; index < _usHeight * _usWidth; index++)
	{
		/*
			armfly : 进行优化, 函数就地展开
			RA8875_WriteRAM(_ptr[index]);

			此处可考虑用DMA操作
		*/
		RA8875_RAM = (*p++);
	}
#endif
	
	rt_exit_critical();
// 	s_ucRA8875Busy = 0;

	/* 退出窗口绘图模式 */
// 	RA8875_QuitWinMode();
}




void window_updata(struct panel_type *parent,struct rect_type *rect)
{
	uint16_t i,j,offset;
	uint16_t *p,*pt;
		
	p = parent->data + rect->y*parent->w + rect->x;
	offset = parent->w - rect->w;
	
	rt_enter_critical();
	
	/* 设置图片的位置和大小， 即设置显示窗口 */
	RA8875_SetDispWin(parent->x+rect->x, parent->y+rect->y, rect->h, rect->w);
	RA8875_WriteCmd(0x02); 		/* 准备读写显存 */
	
	for(i=0;i<rect->h;i++)
	{
		pt = bmp_buf;
		for(j=0;j<rect->w;j++)
			*pt++ = *p++;
		pt = bmp_buf;
		for(j=0;j<rect->w;j++)
			RA8875_RAM = (*pt++);
		p += offset;
	}
	
	rt_exit_critical();
}


