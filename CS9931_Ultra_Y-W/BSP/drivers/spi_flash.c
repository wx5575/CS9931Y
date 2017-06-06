/*
 * Copyright(c) 2014,
 * All rights reserved
 * 文件名称：spi_flash.C
 * 摘  要  ：主程序
 * 当前版本：V1.0，编写
 * 修改记录：
 */

/******************* 加载文件 *********************/
#include "spi_flash.h"

#define			FLASH_CS1_EN()		chipselect=1
#define			FLASH_CS2_EN()		chipselect=2
#define			FLASH_CS3_EN()		chipselect=3
#define			FLASH_CS4_EN()		chipselect=4
#define			FLASH_WP_EN()		GPIO_ResetBits(GPIOF,GPIO_Pin_11)
#define			FLASH_WP_DIS()		GPIO_SetBits(GPIOF,GPIO_Pin_11)
#define			FLASH_CS1_DIS()		GPIO_SetBits(GPIOA,GPIO_Pin_7)
#define			FLASH_CS2_DIS()		GPIO_SetBits(GPIOC,GPIO_Pin_4)
#define			FLASH_CS3_DIS()		GPIO_SetBits(GPIOC,GPIO_Pin_5)
#define			FLASH_CS4_DIS()		GPIO_SetBits(GPIOH,GPIO_Pin_5)
#define			FLASH_DO_HIGH()		GPIO_SetBits(GPIOB,GPIO_Pin_1)
#define			FLASH_DO_LOW()		GPIO_ResetBits(GPIOB,GPIO_Pin_1)
#define			FLASH_CLK_HIGH()	GPIO_SetBits(GPIOB,GPIO_Pin_0)
#define			FLASH_CLK_LOW()		GPIO_ResetBits(GPIOB,GPIO_Pin_0)
#define			FLASH_DI()			GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_2)

#define CMD_AAI       0xAD  	/* AAI 连续编程指令(FOR SST25VF016B) */
#define CMD_DISWR	  0x04		/* 禁止写, 退出AAI状态 */
#define CMD_EWRSR	  0x50		/* 允许写状态寄存器的命令 */
#define CMD_WRSR      0x01  	/* 写状态寄存器命令 */
#define CMD_WREN      0x06		/* 写使能命令 */
#define CMD_READ      0x03  	/* 读数据区命令 */
#define CMD_RDSR      0x05		/* 读状态寄存器命令 */
#define CMD_RDID      0x9F		/* 读器件ID命令 */
#define CMD_SE        0x20		/* 擦除扇区命令 */
#define CMD_BE        0xC7		/* 批量擦除命令 */
#define DUMMY_BYTE    0xA5		/* 哑命令，可以为任意值，用于读操作 */

#define WIP_FLAG      0x01		/* 状态寄存器中的正在编程标志（WIP) */

SFLASH_T g_tSF;
char		chipselect=1;
static uint8_t s_spiBuf[4*1024];	/* 用于写函数，先读出整个page，修改缓冲区后，再整个page回写 */

static void sf_WaitForWriteEnd(void);
static void FLASH_CS_EN(void);
static void FLASH_CS_DIS(void);
// static void spi_gpio_init(void);
static void sf_WriteEnable(void);
static uint8_t sf_AutoWritePage(uint8_t *_ucpSrc, uint32_t _uiWrAddr, uint16_t _usWrLen);


void FLASH_CS_SET(char n)
{
	switch(n)
	{
		case 1:
			FLASH_CS1_EN();
			break;
		case 2:
			FLASH_CS2_EN();
			break;
		case 3:
			FLASH_CS3_EN();
			break;
		case 4:
			FLASH_CS4_EN();
			break;
	}
}

static void FLASH_CS_EN(void)
{
	switch(chipselect)
	{
		case 1:
			GPIO_ResetBits(GPIOA,GPIO_Pin_7);
			break;
		case 2:
			GPIO_ResetBits(GPIOC,GPIO_Pin_4);
			break;
		case 3:
			GPIO_ResetBits(GPIOC,GPIO_Pin_5);
			break;
		case 4:
			GPIO_ResetBits(GPIOH,GPIO_Pin_5);
			break;
	}
}


static void FLASH_CS_DIS(void)
{
	FLASH_CS1_DIS();
	FLASH_CS2_DIS();
	FLASH_CS3_DIS();
	FLASH_CS4_DIS();
}

/*
 * 函数名：spi_gpio_init
 * 描述  ：初始化管脚
 * 输入  ：空
 * 输出  ：空
 */
// static void spi_gpio_init(void)
// {
// 	GPIO_InitTypeDef GPIO_InitStructure;

// 	/* 打开GPIO时钟 */
// 	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOH, ENABLE);

// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* 设为输出口 */
// 	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式 */
// 	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 上下拉电阻不使能 */
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO口最大速度 */

// 	
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
// 	GPIO_Init(GPIOA, &GPIO_InitStructure);
// 	
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
// 	GPIO_Init(GPIOC, &GPIO_InitStructure);
// 	
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
// 	GPIO_Init(GPIOH, &GPIO_InitStructure);
// 	
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);
// 	
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
// 	GPIO_Init(GPIOF, &GPIO_InitStructure);
// 	
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		/* 设为输入口 */
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);
// 	
// 	FLASH_CS_DIS();
// }

/*
 * 函数名：spi_flash_init
 * 描述  ：初始化flash
 * 输入  ：空
 * 输出  ：空
 */
void spi_flash_init(void)
{
// 	spi_gpio_init();
	flash_unlock();
	
	g_tSF.ChipID = sf_ReadID();
	if(g_tSF.ChipID == 0xEF4018)
	{
		strcpy(g_tSF.ChipName, "W25Q128BV");
		g_tSF.TotalSize = 16 * 1024 * 1024;	/* 总容量 = 16M */
		g_tSF.PageSize = 4 * 1024;			/* 页面大小 = 4K */
	}
	
	flash_lock();
}

/*
 * 函数名：sf_SendByte
 * 描述  ：往flash写命令，并返回一字节
 * 输入  ：命令
 * 输出  ：返回值
 */
int sf_SendByte(int cmd)
{
	int result=0;
	int i;
	int pos=0x80;
	
	for(i=0;i<8;i++)
	{
		FLASH_CLK_LOW();
		if(cmd & pos)FLASH_DO_HIGH();
		else FLASH_DO_LOW();
		pos >>= 1;
		result <<= 1;
		FLASH_CLK_HIGH();
		result |= FLASH_DI();
	}
	
	return result;
}

void flash_lock(void)
{
	FLASH_WP_EN();
}

void flash_unlock(void)
{
	FLASH_WP_DIS();
}


/*
*********************************************************************************************************
*	函 数 名: sf_ReadID
*	功能说明: 读取器件ID
*	形    参:  无
*	返 回 值: 32bit的器件ID (最高8bit填0，有效ID位数为24bit）
*********************************************************************************************************
*/
uint32_t sf_ReadID(void)
{
	uint32_t uiID;
	uint8_t id1, id2, id3;
	
	FLASH_CS_EN();
	sf_SendByte(CMD_RDID);								/* 发送读ID命令 */
	id1 = sf_SendByte(DUMMY_BYTE);					/* 读ID的第1个字节 */
	id2 = sf_SendByte(DUMMY_BYTE);					/* 读ID的第2个字节 */
	id3 = sf_SendByte(DUMMY_BYTE);					/* 读ID的第3个字节 */
	FLASH_CS_DIS();
	
	uiID = ((uint32_t)id1 << 16) | ((uint32_t)id2 << 8) | id3;

	return uiID;
}

/*
*********************************************************************************************************
*	函 数 名: sf_WriteEnable
*	功能说明: 向器件发送写使能命令
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sf_WriteEnable(void)
{
	FLASH_CS_EN();									/* 使能片选 */
	sf_SendByte(CMD_WREN);								/* 发送命令 */
	FLASH_CS_DIS();									/* 禁能片选 */
}

/*
*********************************************************************************************************
*	函 数 名: sf_EraseChip
*	功能说明: 擦除整个芯片
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void sf_EraseChip(void)
{
	sf_WriteEnable();								/* 发送写使能命令 */

	/* 擦除扇区操作 */
	FLASH_CS_EN();									/* 使能片选 */
	sf_SendByte(CMD_BE);							/* 发送整片擦除命令 */
	FLASH_CS_DIS();									/* 禁能片选 */

	FLASH_CS_EN();								/* 使能片选 */
	sf_SendByte(CMD_RDSR);							/* 发送命令， 读状态寄存器 */
	
	
}
char sf_wait_find(void)
{
	if((sf_SendByte(DUMMY_BYTE) & WIP_FLAG) == SET) return 1;
	else return 0;
}
void sf_wait_find_dis(void)
{
	FLASH_CS_DIS();									/* 禁能片选 */
}

/*
*********************************************************************************************************
*	函 数 名: sf_EraseSector
*	功能说明: 擦除指定的扇区
*	形    参:  _uiSectorAddr : 扇区地址
*	返 回 值: 无
*********************************************************************************************************
*/
void sf_EraseSector(uint32_t _uiSectorAddr)
{
	sf_WriteEnable();								/* 发送写使能命令 */

	/* 擦除扇区操作 */
	FLASH_CS_EN();									/* 使能片选 */
	sf_SendByte(CMD_SE);								/* 发送擦除命令 */
	sf_SendByte((_uiSectorAddr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
	sf_SendByte((_uiSectorAddr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
	sf_SendByte(_uiSectorAddr & 0xFF);				/* 发送扇区地址低8bit */
	FLASH_CS_DIS();									/* 禁能片选 */

	sf_WaitForWriteEnd();							/* 等待串行Flash内部写操作完成 */
}

/*
*********************************************************************************************************
*	函 数 名: sf_PageWrite
*	功能说明: 向一个page内写入若干字节。字节个数不能超出页面大小（4K)
*	形    参:  	_pBuf : 数据源缓冲区；
*				_uiWriteAddr ：目标区域首地址
*				_usSize ：数据个数，不能超过页面大小
*	返 回 值: 无
*********************************************************************************************************
*/
void sf_PageWrite(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize)
{
	uint32_t i, j;

	for (j = 0; j < _usSize / 256; j++)
	{
		sf_WriteEnable();								/* 发送写使能命令 */

		FLASH_CS_EN();									/* 使能片选 */
		sf_SendByte(0x02);								/* 发送AAI命令(地址自动增加编程) */
		sf_SendByte((_uiWriteAddr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
		sf_SendByte((_uiWriteAddr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
		sf_SendByte(_uiWriteAddr & 0xFF);				/* 发送扇区地址低8bit */

		for (i = 0; i < 256; i++)
		{
			sf_SendByte(*_pBuf++);					/* 发送数据 */
		}

		FLASH_CS_DIS();								/* 禁止片选 */

		sf_WaitForWriteEnd();						/* 等待串行Flash内部写操作完成 */

		_uiWriteAddr += 256;
	}

	/* 进入写保护状态 */
	FLASH_CS_EN();
	sf_SendByte(CMD_DISWR);
	FLASH_CS_DIS();

	sf_WaitForWriteEnd();							/* 等待串行Flash内部写操作完成 */
}

/*
*********************************************************************************************************
*	函 数 名: sf_NeedErase
*	功能说明: 判断写PAGE前是否需要先擦除。
*	形    参:   _ucpOldBuf ： 旧数据
*			   _ucpNewBuf ： 新数据
*			   _uiLen ：数据个数，不能超过页面大小
*	返 回 值: 0 : 不需要擦除， 1 ：需要擦除
*********************************************************************************************************
*/
static uint8_t sf_NeedErase(uint8_t * _ucpOldBuf, uint8_t *_ucpNewBuf, uint16_t _usLen)
{
	uint16_t i;
	uint8_t ucOld;

	/*
	算法第1步：old 求反, new 不变
	      old    new
		  1101   0101
	~     1111
		= 0010   0101

	算法第2步: old 求反的结果与 new 位与
		  0010   old
	&	  0101   new
		 =0000

	算法第3步: 结果为0,则表示无需擦除. 否则表示需要擦除
	*/

	for (i = 0; i < _usLen; i++)
	{
		ucOld = *_ucpOldBuf++;
		ucOld = ~ucOld;

		/* 注意错误的写法: if (ucOld & (*_ucpNewBuf++) != 0) */
		if ((ucOld & (*_ucpNewBuf++)) != 0)
		{
			return 1;
		}
	}
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: sf_CmpData
*	功能说明: 比较Flash的数据.
*	形    参:  	_ucpTar : 数据缓冲区
*				_uiSrcAddr ：Flash地址
*				_uiSize ：数据个数, 可以大于PAGE_SIZE,但是不能超出芯片总容量
*	返 回 值: 0 = 相等, 1 = 不等
*********************************************************************************************************
*/
static uint8_t sf_CmpData(uint32_t _uiSrcAddr, uint8_t *_ucpTar, uint32_t _uiSize)
{
	uint8_t ucValue;

	/* 如果读取的数据长度为0或者超出串行Flash地址空间，则直接返回 */
	if ((_uiSrcAddr + _uiSize) > g_tSF.TotalSize)
	{
		return 1;
	}

	if (_uiSize == 0)
	{
		return 0;
	}

	FLASH_CS_EN();									/* 使能片选 */
	sf_SendByte(CMD_READ);							/* 发送读命令 */
	sf_SendByte((_uiSrcAddr & 0xFF0000) >> 16);		/* 发送扇区地址的高8bit */
	sf_SendByte((_uiSrcAddr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
	sf_SendByte(_uiSrcAddr & 0xFF);					/* 发送扇区地址低8bit */
	while (_uiSize--)
	{
		/* 读一个字节 */
		ucValue = sf_SendByte(DUMMY_BYTE);
		if (*_ucpTar++ != ucValue)
		{
			FLASH_CS_DIS();
			return 1;
		}
	}
	FLASH_CS_DIS();
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: sf_AutoWritePage
*	功能说明: 写1个PAGE并校验,如果不正确则再重写两次。本函数自动完成擦除操作。
*	形    参:  	_pBuf : 数据源缓冲区；
*				_uiWriteAddr ：目标区域首地址
*				_usSize ：数据个数，不能超过页面大小
*	返 回 值: 0 : 错误， 1 ： 成功
*********************************************************************************************************
*/
static uint8_t sf_AutoWritePage(uint8_t *_ucpSrc, uint32_t _uiWrAddr, uint16_t _usWrLen)
{
	uint16_t i;
	uint16_t j;					/* 用于延时 */
	uint32_t uiFirstAddr;		/* 扇区首址 */
	uint8_t ucNeedErase;		/* 1表示需要擦除 */
	uint8_t cRet;

	/* 长度为0时不继续操作,直接认为成功 */
	if (_usWrLen == 0)
	{
		return 1;
	}

	/* 如果偏移地址超过芯片容量则退出 */
	if (_uiWrAddr >= g_tSF.TotalSize)
	{
		return 0;
	}

	/* 如果数据长度大于扇区容量，则退出 */
	if (_usWrLen > g_tSF.PageSize)
	{
		return 0;
	}

	/* 如果FLASH中的数据没有变化,则不写FLASH */
	sf_ReadBuffer(s_spiBuf, _uiWrAddr, _usWrLen);
	if (memcmp(s_spiBuf, _ucpSrc, _usWrLen) == 0)
	{
		return 1;
	}

	/* 判断是否需要先擦除扇区 */
	/* 如果旧数据修改为新数据，所有位均是 1->0 或者 0->0, 则无需擦除,提高Flash寿命 */
	ucNeedErase = 0;
	if (sf_NeedErase(s_spiBuf, _ucpSrc, _usWrLen))
	{
		ucNeedErase = 1;
	}

	uiFirstAddr = _uiWrAddr & (~(g_tSF.PageSize - 1));

	if (_usWrLen == g_tSF.PageSize)		/* 整个扇区都改写 */
	{
		for	(i = 0; i < g_tSF.PageSize; i++)
		{
			s_spiBuf[i] = _ucpSrc[i];
		}
	}
	else						/* 改写部分数据 */
	{
		/* 先将整个扇区的数据读出 */
		sf_ReadBuffer(s_spiBuf, uiFirstAddr, g_tSF.PageSize);

		/* 再用新数据覆盖 */
		i = _uiWrAddr & (g_tSF.PageSize - 1);
		memcpy(&s_spiBuf[i], _ucpSrc, _usWrLen);
	}

	/* 写完之后进行校验，如果不正确则重写，最多3次 */
	cRet = 0;
	for (i = 0; i < 3; i++)
	{

		/* 如果旧数据修改为新数据，所有位均是 1->0 或者 0->0, 则无需擦除,提高Flash寿命 */
		if (ucNeedErase == 1)
		{
			sf_EraseSector(uiFirstAddr);		/* 擦除1个扇区 */
		}

		/* 编程一个PAGE */
		sf_PageWrite(s_spiBuf, uiFirstAddr, g_tSF.PageSize);

		if (sf_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == 0)
		{
			cRet = 1;
			break;
		}
		else
		{
			if (sf_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == 0)
			{
				cRet = 1;
				break;
			}

			/* 失败后延迟一段时间再重试 */
			for (j = 0; j < 10000; j++);
		}
	}

	return cRet;
}

/*
*********************************************************************************************************
*	函 数 名: sf_WriteBuffer
*	功能说明: 写1个扇区并校验,如果不正确则再重写两次。本函数自动完成擦除操作。
*	形    参:  	_pBuf : 数据源缓冲区；
*				_uiWrAddr ：目标区域首地址
*				_usSize ：数据个数，不能超过页面大小
*	返 回 值: 1 : 成功， 0 ： 失败
*********************************************************************************************************
*/
uint8_t sf_WriteBuffer(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize)
{
	uint16_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

	Addr = _uiWriteAddr % g_tSF.PageSize;
	count = g_tSF.PageSize - Addr;
	NumOfPage =  _usWriteSize / g_tSF.PageSize;
	NumOfSingle = _usWriteSize % g_tSF.PageSize;

	
	FLASH_WP_DIS();
	
	
	if (Addr == 0) /* 起始地址是页面首地址  */
	{
		if (NumOfPage == 0) /* 数据长度小于页面大小 */
		{
			if (sf_AutoWritePage(_pBuf, _uiWriteAddr, _usWriteSize) == 0)
			{
				return 0;
			}
		}
		else 	/* 数据长度大于等于页面大小 */
		{
			while (NumOfPage--)
			{
				if (sf_AutoWritePage(_pBuf, _uiWriteAddr, g_tSF.PageSize) == 0)
				{
					return 0;
				}
				_uiWriteAddr +=  g_tSF.PageSize;
				_pBuf += g_tSF.PageSize;
			}
			if (sf_AutoWritePage(_pBuf, _uiWriteAddr, NumOfSingle) == 0)
			{
				return 0;
			}
		}
	}
	else  /* 起始地址不是页面首地址  */
	{
		if (NumOfPage == 0) /* 数据长度小于页面大小 */
		{
			if (NumOfSingle > count) /* (_usWriteSize + _uiWriteAddr) > SPI_FLASH_PAGESIZE */
			{
				temp = NumOfSingle - count;

				if (sf_AutoWritePage(_pBuf, _uiWriteAddr, count) == 0)
				{
					return 0;
				}

				_uiWriteAddr +=  count;
				_pBuf += count;

				if (sf_AutoWritePage(_pBuf, _uiWriteAddr, temp) == 0)
				{
					return 0;
				}
			}
			else
			{
				if (sf_AutoWritePage(_pBuf, _uiWriteAddr, _usWriteSize) == 0)
				{
					return 0;
				}
			}
		}
		else	/* 数据长度大于等于页面大小 */
		{
			_usWriteSize -= count;
			NumOfPage =  _usWriteSize / g_tSF.PageSize;
			NumOfSingle = _usWriteSize % g_tSF.PageSize;

			if (sf_AutoWritePage(_pBuf, _uiWriteAddr, count) == 0)
			{
				return 0;
			}

			_uiWriteAddr +=  count;
			_pBuf += count;

			while (NumOfPage--)
			{
				if (sf_AutoWritePage(_pBuf, _uiWriteAddr, g_tSF.PageSize) == 0)
				{
					return 0;
				}
				_uiWriteAddr +=  g_tSF.PageSize;
				_pBuf += g_tSF.PageSize;
			}

			if (NumOfSingle != 0)
			{
				if (sf_AutoWritePage(_pBuf, _uiWriteAddr, NumOfSingle) == 0)
				{
					return 0;
				}
			}
		}
	}
	
	
	FLASH_WP_EN();
	
	
	return 1;	/* 成功 */
}

/*
*********************************************************************************************************
*	函 数 名: sf_ReadBuffer
*	功能说明: 连续读取若干字节。字节个数不能超出芯片容量。
*	形    参:  	_pBuf : 数据源缓冲区；
*				_uiReadAddr ：首地址
*				_usSize ：数据个数, 可以大于PAGE_SIZE,但是不能超出芯片总容量
*	返 回 值: 无
*********************************************************************************************************
*/
void sf_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
	/* 如果读取的数据长度为0或者超出串行Flash地址空间，则直接返回 */
	if ((_uiSize == 0) ||(_uiReadAddr + _uiSize) > g_tSF.TotalSize)
	{
		return;
	}

	/* 擦除扇区操作 */
	FLASH_CS_EN();									/* 使能片选 */
	sf_SendByte(CMD_READ);							/* 发送读命令 */
	sf_SendByte((_uiReadAddr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
	sf_SendByte((_uiReadAddr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
	sf_SendByte(_uiReadAddr & 0xFF);				/* 发送扇区地址低8bit */
	while (_uiSize--)
	{
		*_pBuf++ = sf_SendByte(DUMMY_BYTE);			/* 读一个字节并存储到pBuf，读完后指针自加1 */
	}
	FLASH_CS_DIS();									/* 禁能片选 */
}

/*
*********************************************************************************************************
*	函 数 名: sf_WaitForWriteEnd
*	功能说明: 采用循环查询的方式等待器件内部写操作完成
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void sf_WaitForWriteEnd(void)
{
	FLASH_CS_EN();								/* 使能片选 */
	sf_SendByte(CMD_RDSR);							/* 发送命令， 读状态寄存器 */
	while((sf_SendByte(DUMMY_BYTE) & WIP_FLAG) == SET);	/* 判断状态寄存器的忙标志位 */
	FLASH_CS_DIS();									/* 禁能片选 */
}


// struct rt_device _flash_device;
// static rt_err_t flash_init(rt_device_t dev)
// {
//     return RT_EOK;
// }

// static rt_err_t flash_open(rt_device_t dev, rt_uint16_t oflag)
// {
//     return RT_EOK;
// }

// static rt_err_t flash_close(rt_device_t dev)
// {
//     return RT_EOK;
// }

// static rt_size_t flash_read(rt_device_t dev,
//                           rt_off_t    pos,
//                           void       *buffer,
//                           rt_size_t   size)
// {
// 	sf_ReadBuffer((uint8_t *)buffer,pos,size);
//     return size;
// }

// static rt_size_t flash_write(rt_device_t dev,
//                           rt_off_t    pos,
//                           const void *buffer,
//                           rt_size_t   size)
// {
// 	flash_unlock();
// 	sf_WriteBuffer((uint8_t *)buffer,pos,size);
// 	flash_lock();
//     return size;
// }

// static rt_err_t flash_control(rt_device_t dev, rt_uint8_t cmd, void *args)
// {
//     switch (cmd)
//     {
//     case RTGRAPHIC_CTRL_GET_INFO:
//     {
//         
//     }
//     break;


//     default:
//         break;
//     }

//     return RT_EOK;
// }

// void rt_hw_flash_init(void)
// {
//     /* register flash device */
//     _flash_device.type  = RT_Device_Class_Block;
//     _flash_device.init  = flash_init;
//     _flash_device.open  = flash_open;
//     _flash_device.close = flash_close;
//     _flash_device.control = flash_control;
//     _flash_device.read  = flash_read;
//     _flash_device.write = flash_write;

//     _flash_device.user_data = RT_NULL;
// 	spi_flash_init();
// 	
//     rt_device_register(&_flash_device, "flash", RT_DEVICE_FLAG_RDWR);
// }
/********************************************************************************************/
