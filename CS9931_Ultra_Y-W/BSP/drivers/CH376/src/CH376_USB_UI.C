
#include  "CH376_USB_UI.H"
// #include  "GUI.h"
// #include "stm32f4xx.h"
#define  ProgramPath            ("\\PROGRAM.BIN")

#include  "hid.H"


uint8_t USB_DEVICE_INIT(void)
{
	uint8_t stat;
	
	stat = mInitCH376Host(1 );  /* 初始化CH376 */
	if (stat == USB_INT_SUCCESS)
	{
		return  (uint8_t)SUCCESS;            /* 操作成功 */
	}
	else
	{
		return  (uint8_t)FAIL;               /* 操作失败 */
	}
}

uint8_t USB_DEVICE_CHECK(void)
{
	xWriteCH376Cmd( 0x16 );  /* 设备USB工作模式 */
	mDelayuS(10);
	return xReadCH376Data();
}

uint8_t WAIT_DEVICE_CONNECT(void)
{
	int8_t stat,i,ERRCount;
	ERRCount = 0;
__WaitUsbDevice:
	  if(ERRCount > MAX_RETERY) return 0;
		ERRCount++;                //错误计数加1
	
		while (CH376DiskConnect() != USB_INT_SUCCESS)
		{ 
			mDelaymS(100 );/* 检查U盘是否连接,等待U盘插入,对于SD卡,可以由单片机直接查询SD卡座的插拔状态引脚 */
		}
		mDelaymS(250);  /* 延时,可选操作,有的USB存储器需要几十毫秒的延时 */
		mDelaymS(250); 

		/* 对于检测到USB设备的,最多等待100*50mS,主要针对有些MP3太慢,对于检测到USB设备并且连接DISK_MOUNTED的,最多等待5*50mS,主要针对DiskReady不过的 */
		for (i = 0; i < 100; i ++ )
		{  /* 最长等待时间,100*50mS */
			stat = CH376DiskMount();  /* 初始化磁盘并测试磁盘是否就绪 */
			if (stat == USB_INT_SUCCESS)
			{
				break;  /* 准备好 */
			}
			else if (stat == ERR_DISK_DISCON)
			{
				break;  /* 检测到断开,重新检测并计时 */
			}
			if (CH376GetDiskStatus() >= DEF_DISK_MOUNTED && i >= 5)
			{
				break;  /* 有的U盘总是返回未准备好,不过可以忽略,只要其建立连接MOUNTED且尝试5*50mS */
			}
		}
		if (stat == ERR_DISK_DISCON)
		{  /* 检测到断开,重新检测并计时 */
			goto __WaitUsbDevice;
		}
		if (CH376GetDiskStatus( ) < DEF_DISK_MOUNTED)
		{  /* 未知USB设备,例如USB键盘、打印机等 */
			return UNKNOWDEVICE;
		}
		return 1;

}

uint8_t OPEN_FILE(char *fliepath)
{
	char	SrcName[64];
	
	strcpy(SrcName, fliepath);
	mDelaymS(80);
	return(CH376FileOpenPath((uint8_t *)SrcName));  /* 打开文件,该文件在根目录下 */
}



// void CH376_1_TEST(void)
// {
// 	static int8_t DiskConnectStat = 0;
// 	static int8_t i = 0;
// 	USB_Device_Chg(USB_1);
// 	if(DiskConnectStat == 0){
// 		if(CH376DiskConnect() != USB_INT_SUCCESS)
// 		{
// 			return;
// 		}else
// 		{
// 			DiskConnectStat = 1;
// 		}
// 	}
// 	
// 	if(USB_INT_SUCCESS == CH376DiskMount()){         //初始化磁盘并测试磁盘是否就绪 
// 		if(OPEN_FILE(ProgramPath) == DEF_DISK_OPEN_FILE)
// 		{
// 			GUI_DispString("OPEN FILE SUCCESS\n");
// 			GUI_DispString("FILE_SIZE:");
// 			GUI_DispDecMin(CH376GetFileSize());
// 		}else
// 		{
// 			GUI_DispString("OPEN FILE FAIL Init\n");
// 		}
// 		TIMER_CLOSE(9);
// 	}
// 	else
// 	{
// 		if (CH376GetDiskStatus() >= DEF_DISK_MOUNTED)i++;
// 		if(i>=5){
// 			if(OPEN_FILE(ProgramPath) == DEF_DISK_OPEN_FILE)
// 			{
// 				GUI_DispString("OPEN FILE SUCCESS\n");
// 				GUI_DispString("FILE_SIZE:");
// 				GUI_DispDecMin(CH376GetFileSize());
// 				
// 			}else
// 			{
// 				GUI_DispString("OPEN FILE FAIL\n");
// 				
// 			}
// 			TIMER_CLOSE(9);
// 		}
// 	}
// 	
// 	                  
// }
// void CH376_2_TEST(void)
// {
// 	static int8_t DiskConnectStat = 0;
// 	static int8_t i = 0;
// 	
// 	USB_Device_Chg(USB_2);
// 	if(DiskConnectStat == 0){
// 		if(CH376DiskConnect() != USB_INT_SUCCESS)
// 		{
// 			return;
// 		}else
// 		{
// 			DiskConnectStat = 1;
// 		}
// 	}
// 	
// 	if(USB_INT_SUCCESS == CH376DiskMount()){         //初始化磁盘并测试磁盘是否就绪 
// 		if(OPEN_FILE(ProgramPath) == DEF_DISK_OPEN_FILE)
// 		{
// 			GUI_DispString("OPEN FILE SUCCESS\n");
// 			GUI_DispString("FILE_SIZE:");
// 			GUI_DispDecMin(CH376GetFileSize());
// 		}else
// 		{
// 			GUI_DispString("OPEN FILE FAIL Init\n");
// 		}
// 		TIMER_CLOSE(10);
// 	}
// 	else
// 	{
// 		if (CH376GetDiskStatus() >= DEF_DISK_MOUNTED)i++;
// 		if(i>=5){
// 			if(OPEN_FILE(ProgramPath) == DEF_DISK_OPEN_FILE)
// 			{
// 				GUI_DispString("OPEN FILE SUCCESS\n");
// 				GUI_DispString("FILE_SIZE:");
// 				GUI_DispDecMin(CH376GetFileSize());
// 				
// 			}else
// 			{
// 				GUI_DispString("OPEN FILE FAIL\n");
// 				
// 			}
// 			TIMER_CLOSE(10);
// 		}
// 	}
// 	
// 	                  
// }



/* 获取HID 报告描述符 */
uint8_t SetupGetHidDes[] = {0x81,0x06,0x00,0x22,0x00,0x00,0x81,0x00};    
/* 设置报表 */
uint8_t SetupSetReport[] = {0x21,0x09,0x00,0x02,0x00,0x00,0x01,0x00};

uint8_t	Device_desc[18],	   //设备描述符
		Config_desc[9],      //配置描述符
		Interface_desc[9],   //接口描述符
 		Endpoint_desc[7],    //端点描述符
		Class_desc[9], 		   //群组描述符
    data_buf[180],       //报告描述符
/* 用于复合设备的描述符数组 */
		Interface_desc2[9],  //接口描述符
		Endpoint_desc2[7],   //端点描述符
		Class_desc2[9]; 		 //群组描述符
/* 事务同步标志模式 */
uint8_t receive_mode = 0x00,send_mode = 0x00;
/* 设备端点同步标志 */
uint8_t Endpoint_tog[2]={0,0};
uint8_t NumLock=0;//Key Num LED 
/*
 *函数名称：Set_USB_Mode
 *功能说明：设置CH376的工作模式 0x06 为主机模式
 *输入参数：模式代码
 *输出参数：返回错误代码 ：0成功，1失败
 */
uint8_t Set_USB_Mode(uint8_t mode) 
{  
	uint8_t TimeOver = 100;
	xWriteCH376Cmd( CMD11_SET_USB_MODE );  /* 设备USB工作模式 */
	xWriteCH376Data(mode);
	receive_mode = send_mode = 0x00;//主机端复位USB数据同步标志 
	while(TimeOver--)	              //等待设置模式操作完成,不超过30uS
	{
		mDelayuS(10);
		if(xReadCH376Data()==CMD_RET_SUCCESS) return( 0 );//成功
	}
	return(1);//CH376出错,例如芯片型号错或者处于串口方式或者不支持
}

UINT8	Wait376Interrupts( void )  /* 等待CH376中断(INT#低电平)，返回中断状态码, 超时则返回ERR_USB_UNKNOWN */
{
	UINT32	i;
	for ( i = 0; i < 100; i ++ ) {  /* 计数防止超时,默认的超时时间,与单片机主频有关 */
		if ( Query376Interrupt( ) ) return( CH376GetIntStatus( ) );  /* 检测到中断 */
/* 在等待CH376中断的过程中,可以做些需要及时处理的其它事情 */
	}
	return( ERR_USB_UNKNOWN );  /* 不应该发生的情况 */
}

/*
 *函数名称：Reset_Device
 *功能说明：复位USB设备
 *输入参数：无
 *输出参数：返回错误代码 ：0成功，1超时
 */
uint8_t Reset_Device(void)
{	
	uint8_t TimeOver = 100;
	Set_USB_Mode( 7 );  									//复位USB设备,CH376向USB信号线的D+和D-输出低电平
	mDelaymS(10);
	Set_USB_Mode( 6 );  									//结束复位，将CH376设置成主机模式
	while(TimeOver--)
	{
		if(Wait376Interrupt()==USB_INT_CONNECT)return 0;//等待复位之后的设备端再次连接上来
	}
	return 1;
}

/*
 *函数名称：Wait376InterruptCMD
 *功能说明：获取指定命令的中断状态并清除中断
 *输入参数：命令
 *输出参数：返回错误代码 ：0无错误，其他->请查看中断返回值表对应的错误代码含义
 */
uint8_t Wait376InterruptCMD(uint8_t cmd)
{
	xWriteCH376Cmd(cmd);
	return Wait376Interrupt();
}

/*
 *函数名称：WR_USB_DATA
 *功能说明：往CH376的端点缓冲区写入数据块
 *输入参数：要写入数据块的长度，写入数据缓冲区的地址
 *输出参数：无
 */
void WR_USB_DATA( uint8_t len, uint8_t *buf ) 
{   
	xWriteCH376Cmd(CMD10_WR_HOST_DATA);//向CH376的端点缓冲区写入准备发送的数据, 后续数据长度, len不能大于64
	xWriteCH376Data(len);
	while(len--) xWriteCH376Data(*buf++);
}

/*
 *函数名称：RD_USB_DATA
 *功能说明：从CH376的端点缓冲区读取接收到的数据
 *输入参数：数据缓冲区的地址
 *输出参数：返回接收的数据长度
 */
uint8_t RD_USB_DATA( uint8_t *buf ) 
{  
	uint8_t i, len;
	xWriteCH376Cmd(CMD01_RD_USB_DATA0);   // 从CH37X读取数据块 
	len = xReadCH376Data();           // 后续数据长度 
	for (i=0;i<len;i++) *buf++=xReadCH376Data();
	return( len );
}

/*
 *函数名称：Issue_Token
 *功能说明：执行USB事务
 *输入参数：同步标志，端点号和令牌
 *输出参数：无
 */
void Issue_Token(uint8_t endptog, uint8_t endp_and_pid ) 
{  
	xWriteCH376Cmd( CMD2H_ISSUE_TKN_X );//CMD4e
	xWriteCH376Data( endptog );
	xWriteCH376Data( endp_and_pid );
	mDelaymS(2);  
}

/*
 *函数名称：Get_Descr
 *功能说明：获取设备的描述符
 *输入参数：inf接口号，len数据长度
 *输出参数：错误代码 ：0成功，1数据建立阶段错误，2执行OUT事务错误
 */
uint8_t Get_Descr(uint8_t inf,uint16_t len)
{
	uint8_t i;
	uint8_t *p=data_buf;
	SetupGetHidDes[4] = inf;
	SetupGetHidDes[6] = len;
	WR_USB_DATA(8,SetupGetHidDes);
  send_mode=0x00;
	Issue_Token(send_mode,DEF_USB_PID_SETUP);       //发起控制传输，发送建立数据 
	if(Wait376Interrupt() != 0x14)return 1;
	receive_mode=0x00;
  for(i=0;i<len/8;i++)
	{
	  receive_mode^=0x80;
		Issue_Token(receive_mode,DEF_USB_PID_IN);	    //执行 IN 	务，接收数据
		if(Wait376Interrupt() != 0x14)return 2;
		RD_USB_DATA(p+=8);//接收数据
	}
	return 0;
}

/*
 *函数名称：Set_Report
 *功能说明：对于键盘设备，则可以设置报表；可以通过设置报表，点亮键盘指示灯
 *输入参数：LED状态
 *输出参数：错误代码 ：0成功，1数据建立阶段错误，2执行OUT事务错误，3执行IN事务错误，4接收错误
 */
uint8_t Set_Report(uint8_t len_S)
{	
	len_S = len_S ? 1:0;
	WR_USB_DATA(8,SetupSetReport);    //设置报表指令
  send_mode=0x00;	                               	
	Issue_Token(send_mode,DEF_USB_PID_SETUP);	     //数据建立阶段，发送建立数据
	if(Wait376Interrupt() != 0x14)return 1;
	xWriteCH376Cmd(0x2c);    //写数据命令	//点亮LED
	xWriteCH376Data(0x01);    //数据长度
	xWriteCH376Data(len_S);   //发数据
  send_mode ^= 0x40;                             	
	Issue_Token(send_mode,DEF_USB_PID_OUT);				 //执行 OUT 事务，发送数据
	if(Wait376Interrupt() != 0x14)return 2;
  receive_mode=0x80;
	Issue_Token(receive_mode,DEF_USB_PID_IN);			 //执行 IN 事务，接收数据
	if(Wait376Interrupt() != 0x14)return 3;
	xWriteCH376Cmd(0x27);	  //接收数据命令0x27
	if(xReadCH376Data() != 0)return 4; //如果数据长度不为0，返回错误
	return(0);
}

/*
 *函数名称：CH376_Interrupte
 *功能说明：CH376中断服务程序，初始化USB设备 
 *输入参数：无
 *输出参数：错误代码 ：0成功，1错误
 */
uint8_t CH376_Interrupte(void)
{
	uint8_t s=0,i=0,len=0;
	mDelaymS(50);
	s=Reset_Device();	                 //复位usb设备
	mDelaymS(50);
	xWriteCH376Cmd(CMD11_GET_DEV_RATE);   //主机方式: 获取当前连接的USB设备的数据速率类型
	xWriteCH376Data(0x07);
	if((xReadCH376Data()&0x10) != 0)  //第4位为1表示低速设备
	{
		xWriteCH376Cmd(CMD10_SET_USB_SPEED);//设置USB总线速度:02H=1.5Mbps低速LowSpeed
		xWriteCH376Data(0x02);
		mDelaymS(20);
		xWriteCH376Cmd(CMD0H_AUTO_SETUP);
		if(Wait376Interrupt() != USB_INT_SUCCESS) //自动配置ＵＳＢ设备
		{
			return 11;
		}
		xWriteCH376Cmd(CMD1H_GET_DESCR);    //读设备描述符
		xWriteCH376Data(0x01);
		if(Wait376Interrupt() != USB_INT_SUCCESS)//是否成功接收
		{
			return 12;
		}
		xWriteCH376Cmd(0x27);
		len = xReadCH376Data();
		if(len != 18)                  //数据长度是否正确
		{
			return 13;
		}
		for(i=0;i<len;i++)
		{
			 Device_desc[i] = xReadCH376Data();
		}
		//读配置描述符、接口描述符、群组描述符、端点描述符
		xWriteCH376Cmd(CMD1H_GET_DESCR);
		xWriteCH376Data(0x02);
		if(Wait376Interrupt() != USB_INT_SUCCESS)//是否成功接收
		{
			return 14;
		}
		xWriteCH376Cmd(0x27);
		len = xReadCH376Data();
		for(i=0;i<len;i++)
		{
			 s = xReadCH376Data();//printf("%02X,",s);
			 if(i<9)       Config_desc[i]       =s;
			 else if(i<18) Interface_desc[i-9]  =s;
			 else if(i<27) Class_desc[i-18]     =s;
			 else if(i<34) Endpoint_desc[i-27]  =s;
			 else if(i<43) Interface_desc2[i-34]=s;
			 else if(i<52) Class_desc2[i-43]    =s;
			 else if(i<59) Endpoint_desc2[i-52] =s;
		}
		if(((PUSB_ITF_DESCR)Interface_desc)->bInterfaceProtocol == 1)
		{
			//printf("\t---<键盘>\n\r");
		}
		else if(((PUSB_ITF_DESCR)Interface_desc)->bInterfaceProtocol == 2)
		{
			//printf("\t---<鼠标>\n\r");
		}
		s=Get_Descr(0,((PUSB_HID_CLASS_DESCR)Class_desc)->wDescriptorLength);//获取报表描述符
    //printf("\n\n\r报告描述符：（长度：%d）\n\r",((PUSB_HID_CLASS_DESCR)Class_desc)->wDescriptorLength);
    //for(i=0;i<((PUSB_HID_CLASS_DESCR)Class_desc)->wDescriptorLength;i++)printf("%02X,",data_buf[i]);
//设置报表
		if(((PUSB_ITF_DESCR)Interface_desc)->bInterfaceProtocol == 1)//如果为键盘，点灯
		{
			s = Set_Report(1);
			if(s!=0)
			{
				return 15;
			}
		}
	/******** 复合设备初始化 ********/		 
		if(((PUSB_CFG_DESCR)Config_desc)->bNumInterfaces == 2)//接口数目
		{
			Get_Descr(1,((PUSB_HID_CLASS_DESCR)Class_desc2)->wDescriptorLength);//获取报表描述符		
			//printf("\n\n\r报告描述符2：（长度：%d）\n\r",((PUSB_HID_CLASS_DESCR)Class_desc2)->wDescriptorLength);
			//for(i=0;i<((PUSB_HID_CLASS_DESCR)Class_desc2)->wDescriptorLength;i++)printf("%02X,",data_buf[i]);
	  //设置报表
			s = Set_Report(1);
			if(s!=0)
			{
	 			return 16;
			}
		}
	 /**********************************/
		xWriteCH376Cmd(0x0b);  //设置CH376重试次数
		xWriteCH376Data(0x25);  //数据1
		xWriteCH376Data(0x00);  //重试次数 
		//    为了保证兼容性对于部分鼠标键盘需要设置有限次重试(0xc0),而对于复合设备，
	  //比如USB转PS2的设备，复合键盘，则不需要重试(0x00)
	}
	return 0;
}





/*
 *函数名称：Get_Int_In
 *功能说明：获取HID类设备的数据	 <支持复合设备的操作> 
 *输入参数：该设备的同步标志，端点号
 *输出参数：错误代码 ：0成功，1错误
 */
uint8_t scan_buf[64],scan_buf_w=0,scan_buf_r=0;

uint8_t Get_Int_In(uint8_t tog,uint8_t endp_int)
{
	uint8_t s;
	tog = tog ? 0x80 : 0x00;
	Issue_Token(tog,( endp_int << 4 ) | DEF_USB_PID_IN);//执行 IN 	务，接收数据
	if(Wait376Interrupt() == USB_INT_SUCCESS)
	{
	  RD_USB_DATA(data_buf); //键盘中断端点数据长度一般为8字节，鼠标为4字节
		s = data_buf[2];   //printf("%x ",data_buf[0]);printf("%x ",data_buf[1]);
		if(s != 0)
		{
		  if(s == 0x28 || s == 0x58)
			{
// 				printf("\n\r");
// 				strcat(p,"\n\r")
				scan_buf[scan_buf_w++] = '\n';
				if(scan_buf_w>63)scan_buf_w=0;
				scan_buf[scan_buf_w++] = '\r';
				if(scan_buf_w>63)scan_buf_w=0;
//					Speaker(1,50);
			}				
			else if(s>=0x1e && s<=0x27)
			{
				//printf("%d",((s-0x1e+1)%10));//键盘上的数字键
				scan_buf[scan_buf_w++] = '0'+((s-0x1e+1)%10);
				if(scan_buf_w>63)scan_buf_w=0;
			}
			else if(s>=0x59 && s<=0x62)
			{
				scan_buf[scan_buf_w++] = '0'+((s-0x59+1)%10);
				if(scan_buf_w>63)scan_buf_w=0;
// 				printf("%d",((s-0x59+1)%10));//数字小键盘上的数字键
			}
			else if(s == 0x2c)
			{
				scan_buf[scan_buf_w++] = ' ';
				if(scan_buf_w>63)scan_buf_w=0;
// 				printf(" ");
			}
			else if(s>=0x04 && s<=0x1d)
			{
				if(data_buf[0]==0x02 || data_buf[0]==0x20)
				{
					scan_buf[scan_buf_w++] = ('A'+s-4);
					if(scan_buf_w>63)scan_buf_w=0;
// 					printf("%c",('A'+s-4));
				}
				else 
				{
					scan_buf[scan_buf_w++] = ('a'+s-4);
					if(scan_buf_w>63)scan_buf_w=0;
// 					printf("%c",('a'+s-4));
				}
			}
			else if(s==0x53)
			{
				Set_Report(NumLock);
				NumLock ^= 1;
			}
// 			else 
// 				printf("%x ",s);			
		}
			

		if( endp_int == ((PUSB_ENDP_DESCR)Endpoint_desc)->bEndpointAddress )//如果为端点0地址	
			Endpoint_tog[0] = Endpoint_tog[0] ? 0:1;//端点0的同步标志取反
		else 	//为端点1地址
			Endpoint_tog[1] = Endpoint_tog[1] ? 0:1;//端点1的同步标志取反
		return 0;
	}
	return s;
}

uint8_t Get_Scan_Data(void )
{
	//获取设备1的数据
	Get_Int_In( Endpoint_tog[0],Endpoint_desc[2]);			
	//如果该设备是符合设备，则需要获取设备2的数据
	if(((PUSB_CFG_DESCR)Config_desc)->bNumInterfaces == 2)
				Get_Int_In( Endpoint_tog[1],Endpoint_desc2[2]);
	return 0;
}

















