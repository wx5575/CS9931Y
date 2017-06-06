
#include "stdint.h"
#include "string.h"
// #include "App_config.h"
#include "library.h"
// #include "macro.h"
// #include "ui_config.h"
#include "CS99xx.h"


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
 *  附    注 : 不为可重入函数 当主循环和中断服务函数都调用此函数时 极可能造成                                                               
 *             共享资源的破坏
 *             The maximal possible delay is 768 us / F_CPU in MHz                                                               
 *                                                                            
******************************************************************************/

void LIB_DelayUs(uint16_t delayUs)
{
    uint8_t			i;

	for (i = 0; i < delayUs; i++)
	{
		__nop();
	}
	//调用此函数进行准确延时
	//_delay_us(delayUs);


}

void LIB_SoftDelayUs(uint16_t delayUs)
{
    LIB_DelayUs(delayUs);
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
 *  附    注 : 软件延时 延时值不准确 依据具体要求而定                                                               
 *             The maximal possible delay is 262.14 ms / F_CPU in MHz                                                               
 *                                                                            
******************************************************************************/

void LIB_DelayMs(uint16_t delayMs)
{
    uint32_t      interger                                = delayMs/20;
//	uint32_t      remain                                  = delayMs%20;
	uint32_t      i                                       = 0;
	
	for (; i < interger; i++)
	{
		//_delay_ms(20);
	}
	//_delay_ms(remain);
}

void LIB_SoftDelayMs(uint16_t delayMs)
{
	uint8_t delayi;
	while(--delayMs)
	{
		for(delayi=0;delayi<124;delayi++);
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

uint32_t LIB_MonitorUs(uint32_t monitorUs)
{
    //调用此函数进行监控
	//return (__HAL_MonitorTmrStart(monitorUs));
	return 1;
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 获取10的n次幂                                                               
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
 *  附    注 : 没有采用标志函数是因为标准函数采用浮点运算 不划算                                                               
 *                                                                            
 *                                                                            
******************************************************************************/

uint32_t LIB_Get10nData(uint32_t n)
{
    uint32_t rt 					                                = 1;
	
    for (; n > 0; n--)
    {
        rt 					                                   *= 10;    
    }
    return rt;
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 数据拆分                                                              
 *                                                                           
 *  入口参数 : rscData:要拆分的数据 splitLen：拆分数据的长度 pdestSaveAddr：拆分后的数据存放地址                                                               
 *                                                                             
 *  出口参数 : 无                                                               
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 : 范例：LIB_DataSplit(4096, 4, saveData);                                                            
 *                   4096 拆分后存放在saveData数组中  saveData[0] = 4 saveData[1] = 0
 *                                                    saveData[2] = 9 saveData[3] = 6          
 *                                                                            
******************************************************************************/

void LIB_DataSplit(uint32_t rscData, uint32_t splitLen, uint8_t *pdestSaveAddr)
{
    uint32_t TmpFor10n     = 0;
    splitLen--;

    for (; splitLen > 0; splitLen--)
    {
        TmpFor10n        = LIB_Get10nData(splitLen);						//取10的n次幂
        *pdestSaveAddr   = rscData/TmpFor10n;
        rscData         %= TmpFor10n; 
        pdestSaveAddr++;   
    }

    *pdestSaveAddr = rscData;
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 数据计算                                                              
 *                                                                           
 *  入口参数 : 输入为字符串                                                              
 *                                                                             
 *  出口参数 : 无                                                               
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
******************************************************************************/

uint32_t LIB_DataCalculate(const uint8_t *rscStr, uint32_t calculateLen)
{
    uint32_t i           = 0;
	uint32_t returnValue = 0;

	for (; i < calculateLen; i++)
	{
		returnValue += ((rscStr[calculateLen - i - 1] - 0x30)*LIB_Get10nData(i));	
	}
	
	return returnValue;
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 查找字符串中某个字符的位置                                                              
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 : 若找到 则返回位置索引号 若找不到 则返回-1                                                               
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

int8_t LIB_Strpos(const uint8_t *rscStr, uint8_t c)
{
    uint32_t i = 0;
	
    for(; i < strlen((const char *)rscStr); i++)
    {
        if (rscStr[i] == c)
        {
            return i;
        }
        else
        {
            continue;
        }
    } 

    return -1;
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 向字符串中某一位置处插入一字符串                                                              
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

uint32_t LIB_StrInsert(uint8_t *rscStr, uint8_t insertPosIndex, uint8_t insertChar)
{
    uint32_t   i             = 0;
    uint32_t   j             = strlen((const char *)rscStr) - 1;              //转换为索引号

    if (insertPosIndex > j)
	{
		return 0;
	}
	
	for (i = j; i >= insertPosIndex; i--)
    {
        rscStr[i + 1]             = rscStr[i];
    }
    rscStr[insertPosIndex]        = insertChar;
	
	return 1;
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 反向查找字符串中某个字符的位置                                                              
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 : 若找到 则返回位置索引号 若找不到 则返回-1                                                               
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

int8_t LIB_StrBkpos(const uint8_t *rscStr, uint32_t c)
{
    uint32_t   i = 0;
    uint32_t   j = strlen((const char *)rscStr);

    for(; i < j; i++)
    {
        if (rscStr[j-i-1] == c)
        {
            return j-i-1;
        }
        else
        {
            continue;
        }
    } 

    return -1;
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 查找字符串中前n字符中某个字符的位置                                                              
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 : 若找到 则返回位置索引号 若找不到 则返回-1                                                               
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
void LIB_Set_Zero(uint16_t *rscStr,uint32_t len)
{
	uint32_t 	i 			= 0;
	
    for(; i < len; i++)
    {
        if (rscStr[i] != 0)
        {
            rscStr[i] = 0;
        }
	}
}


int8_t LIB_Strnpos(const uint8_t *rscStr, uint32_t c, uint32_t len)
{
    uint32_t 	i 			= 0;
	
    for(; i < len; i++)
    {
        if (rscStr[i] == c)
        {
            return i;
        }
        else
        {
            continue;
        }
    } 

    return -1;
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 将数字转换为字符                                                              
 *                                                                           
 *  入口参数 : prscSaveAddr:数字存放地址 也是转换后字符存放地址 convertLen：转换长度                                                               
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
 *  附    注 : 范例：uint8_t numb[4] = {4, 0, 9, 6}; LIB_ConvertNmubToChar(numb, 4);                                                               
 *                   转换后：numb[0] = '4' numb[1] = '0' numb[2] = '9' numb[3] = '6'                                                      
 *                                                                            
******************************************************************************/

void LIB_ConvertNmubToChar(uint8_t *prscSaveAddr, uint32_t convertLen)
{
    uint32_t 	i 				= 0;
	
    for (; i < convertLen; i++)
    {
        *(prscSaveAddr + i) 	   += '0';    
    }        
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 暂时未做考虑                                                               
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

int8_t LIB_SearchNoneNumbIndex(uint8_t *rscStr, uint32_t strLen)
{
    uint32_t   i  		= 0;
	
    for (; i < strLen; i++)
    {
        if (rscStr[i] == '0')
        {
            continue;
        }
        else if (rscStr[i] == '.')
        {
            //第一个字符不允许为.s
            return (i - 1);
        }
        else
        {
            return (i);
        }
    }
    
    return -1;
}

void LIB_HideZeroForString(uint8_t *rscStr, uint32_t strLen)
{
    uint32_t i   		= 0;
    int8_t 		  len  		= LIB_SearchNoneNumbIndex(rscStr, strLen);
    
    if (-1 == len)
    {
        len = strLen - 1;
    }
    //消隐前面的零
    //最后一位不参与消隐
    for (; i < len; i++)
    {            
        if ('0' == rscStr[i])
        {
            rscStr[i] = ' ';
        }
        else
        {
            break;
        }
    }
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 把数字转换为字符串 并消去前面的零                                                                
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
 *  附    注 : 入口参数 rscData只能是整形数                                                              
 *                                                                            
 *                                                                            
******************************************************************************/

void LIB_ConvertNmubToCharWithSpace(uint32_t rscData, uint32_t splitLen, uint8_t *pdestSaveAddr)
{
	LIB_ConvertNmubToString(rscData, splitLen, pdestSaveAddr);
    LIB_HideZeroForString(pdestSaveAddr, splitLen);
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 把数字转换为字符串                                                                 
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
 *  附    注 : 入口参数 rscData只能是整形数                                                              
 *                                                                            
 *                                                                            
******************************************************************************/

void LIB_ConvertNmubToString(uint32_t rscData, uint32_t splitLen, uint8_t *pdestSaveAddr)
{
    LIB_DataSplit(rscData, splitLen, pdestSaveAddr);
    LIB_ConvertNmubToChar(pdestSaveAddr, splitLen);
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 获取一字节和校验值  大多用于文件系统部分获得和校验值                                                             
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

uint8_t LIB_GetChkSum(uint8_t *rscAddr, uint32_t len)
{
	/*
    typedef struct
    {
        uint8_t           m_chkLowSum;
        uint8_t           m_chkHighSum;

    }tChkSum;
    typedef union
    {
        uint16_t          u_chkSum;
        tChkSum         u_tchkSum;

    }uChkSum;
    
    uint32_t   i            = 0;
    uChkSum 		u_chkSum     = {0}; 

	for (; i < len; i++)
	{
        u_chkSum.u_tchkSum.m_chkHighSum ^= *(rscAddr + i);
        u_chkSum.u_tchkSum.m_chkLowSum  += *(rscAddr + i);	
	}

	return u_chkSum.u_chkSum;
    */
    uint8_t           chkSum                                      = 0;
    uint32_t          i                                           = 0;  
    for (; i < len; i++)
		{
        chkSum                                                 += *(rscAddr + i);
    }
    return chkSum|0x80;
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 验证日期字符串合法                                                              
 *                                                                           
 *  入口参数 : 输入为日期字符串                                                              
 *                                                                             
 *  出口参数 : 无                                                               
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
******************************************************************************/

#ifdef			LIB__CHK_DATA_STR_VALID

uint32_t LIB_ChkDateStrValid(uint8_t *rscDateStr)
{
	uint8_t       year                                            = LIB_DataCalculate((void *)&rscDateStr[0], 4);
    uint8_t       month                                           = LIB_DataCalculate((void *)&rscDateStr[5], 2);
    uint8_t       day                                             = LIB_DataCalculate((void *)&rscDateStr[8], 2);
    uint8_t       maxValue                                        = 31;

	//判断年
    if (year > 4095)
    {
    	return FALSE;
    }
    //判断月
    if ((month > 12) || (month < 1))
    {
        return FALSE;
    }
            
    //判断日  
    if ((month == 4) || (month == 6) || (month == 9) || (month == 11))
	{
		maxValue                                                = 30;
	} 
    //如果月份为2月份 且 是闰年
	else if (month == 2)
	{
	    if ((year % 4) == 0)
		{
			maxValue                                            = 29;
		}
		else
		{
		    maxValue                                            = 28;
		}
	}
	//判断日
	if ((day > maxValue) || (day < 1))
	{
		return FALSE;
	}
	
	return TRUE;
}
#endif

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 验证时间字符串合法                                                              
 *                                                                           
 *  入口参数 : 输入为时间字符串                                                              
 *                                                                             
 *  出口参数 : 无                                                               
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
******************************************************************************/

#ifdef			LIB__CHK_DATA_STR_VALID

uint32_t LIB_ChkTimeStrValid(uint8_t *rscTimeStr)
{
	uint8_t       hour        		                            = LIB_DataCalculate((void *)&rscTimeStr[0], 2);;
    uint8_t       minute      		                            = LIB_DataCalculate((void *)&rscTimeStr[3], 2);;
    uint8_t       second      		                            = LIB_DataCalculate((void *)&rscTimeStr[6], 2);;
	
	if ((hour > 23) || (minute > 59) || (second > 59))
	{
		return FALSE;
	}

	return TRUE;
}
#endif

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 字符串循环右移                                                              
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
******************************************************************************/

void LIB_StringRor(uint8_t *rscAddr, uint32_t len, uint32_t rorCnt)
{
    uint32_t       i           = 0;
    uint32_t       j           = 0;
    uint32_t       tmp         = 0;

    for (; i < rorCnt; i++)
    {
        tmp                         = rscAddr[len - 1];
        for (j = 1; j < len; j++)
        {
            rscAddr[len - j]        = rscAddr[len - j - 1];
        }
        rscAddr[0]                  = tmp;
    }
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 字符串循环左移                                                              
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
******************************************************************************/

void LIB_StringRol(uint8_t *rscAddr, uint32_t len, uint32_t rolCnt)
{
    uint32_t       i           = 0;
    uint32_t       j           = 0;
    uint32_t       tmp         = 0;

    for (; i < rolCnt; i++)
    {
        tmp                         = rscAddr[0];
        for (j = 0; j < len - 1; j++)
        {
            rscAddr[j]              = rscAddr[j + 1];
        }
        rscAddr[j]                  = tmp;
    }
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 字符串逻辑右移                                                              
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
******************************************************************************/

void LIB_StringLsr(uint8_t *rscAddr, uint32_t len, uint32_t lsrCnt)
{
    uint32_t       i           = 0;
    uint32_t       j           = 0;

    for (; i < lsrCnt; i++)
    {
        for (j = 1; j < len; j++)
        {
            rscAddr[len - j]        = rscAddr[len - j - 1];
        }
        rscAddr[0]                  = '0';
    }
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 字符串逻辑左移                                                              
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
******************************************************************************/

void LIB_StringLsl(uint8_t *rscAddr, uint32_t len, uint32_t lsrCnt)
{
    uint32_t       i           = 0;
    uint32_t       j           = 0;

    for (; i < lsrCnt; i++)
    {
        for (j = 0; j < len; j++)
        {
            rscAddr[j]              = rscAddr[j + 1];
        }
        rscAddr[len]                = 0;
    }
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 字符串中字符替代                                                              
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
******************************************************************************/

void LIB_StringReplace(uint8_t *rscAddr, uint32_t len, uint32_t rscChar, uint32_t destChar)
{
    uint32_t       i           = 0;

    for (; i < len; i++)
    {
        if (rscAddr[i] == rscChar)
        {
            rscAddr[i]              = destChar;
        }
    }
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 字符串比较函数 比较所限定的长度                                                             
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
******************************************************************************/

uint32_t LIB_Strncmp(const char *prscStr, const char *pcmpStr, uint32_t count)
{
    register signed char result = 0;
    
    while (count)
	{
		if ((result = *prscStr - *pcmpStr++) != 0 || !*prscStr++)
			break;
		count--;
	}

	return result;
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 获取数据位长度                                                             
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

uint8_t LIB_DataBitLenGet(uint32_t rscData)
{
    uint8_t                 dataBitLen                            = 1;

    //获取数据所对应的位数
    while (rscData/10 >= 1)
    {                                                           
        rscData                                                 = rscData/10;
        dataBitLen++;
    }
    return dataBitLen;
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 数据拆分函数                                                              
 *                                                                           
 *  入口参数 : rscData--要拆分的数据 length--拆分数据的长度 pdestAddr--拆分后的数据存放地址                                                               
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
void Util_DataSplit(uint32_t rscData, uint8_t length, int8_t *pdestAddr)
{
    uint32_t    temp    = 0;
    

    length--;

	for (; length > 0; length--)
    {
        temp          = LIB_Get10nData(length);
       *pdestAddr     = rscData/temp;
        rscData      %= temp; 
        pdestAddr++;   
    }

    *pdestAddr = rscData;
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 拆分数据 转化为字符 长度可以指定 添加小数点 隐藏不需要显示的零                                                              
 *                                                                           
 *  入口参数 : rscData--要拆分的数据 length--拆分数据的长度 pdestAddr--拆分后的数据存放地址                                                               
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

void Util_MultifunctionalDataSplit(uint32_t rscData, uint8_t length, int8_t *pdestAddr, uint8_t dotPos)
{
    int8_t    temp[10] = {0};


    if (dotPos >= length)
    {
        return;
    }

    Util_DataSplit(rscData, length, pdestAddr);
    LIB_ConvertNmubToChar((uint8_t *)pdestAddr, length);

    if (0 != dotPos)
    {
        strcpy((void *)temp, (void *)(&pdestAddr[dotPos]));
        strcpy((void *)(&pdestAddr[dotPos]), ".");
        strcpy((void *)(&pdestAddr[dotPos + 1]), (void *)temp);
    }
    //else
    //{
    //    strcat((void *)pdestAddr, " ");
    //}
}
/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 : 锁定按键函数                                                               
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
 *  日    期 :2012.5.29                                                                
 *                                                                                                                                       
 *  附    注 :                                                              
 *                                                                        
 *                                                                            
******************************************************************************/
extern char lock_flag;
void LIB_LockKeyFuction(void)
{
	
	rt_mb_send(&screen_mb, UPDATE_STATUS | STATUS_STATUS_EVENT | (1));
	system_parameter_t.Com_lock = 1;
	
}

void LIB_OpenKeyFuction(void)
{

	rt_mb_send(&screen_mb, UPDATE_STATUS | STATUS_STATUS_EVENT | (0));
	system_parameter_t.Com_lock = 0;
	
}


/*
 *函数介绍：Int类型到Str的转换
 *输入参数：字符指针, Int_num:整数位数, Dec_num:小数位数, value:数值
 *输出参数：无
 *备    注：无
 */
void IntToStr(char *pc, u8 Int_num, u8 Dec_num, u32 value)
{
	char temp[10];
	u8  i=0;
	u32 base = 1;
	for(i=0; i<(Int_num+Dec_num); i++)
	{
		temp[i] = (value / base)%10+'0';
		base *= 10;
	}
	for(i=(Int_num+Dec_num);i>0;i--)
	{
		*pc++ = temp[i-1];
		if(Dec_num>0 && i==(Dec_num+1))*pc++ = '.';
	}		
	*pc = '\0';
}

void stritem(char *str)
{
	u8 i;
	char strt[100];
	char *pstr=strt;
	strcpy(strt,str);
	for(i=0;i<strlen(strt);i++)
	{
		if(*pstr != ' ')
		*str++=*pstr;
		pstr++;
	}
	*str = 0;
}
