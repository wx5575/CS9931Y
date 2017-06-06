#include "CS99xx.h"
#include "memory.h"
#include "bsp_ico.h"
#include "memorymanagement.h"
#include "bsp_listbox.h"
#include "sui_window.h"

extern rt_int32_t read_flash_to_step(rt_uint8_t n, rt_uint8_t step, UN_STR *pun);
extern rt_int16_t save_step_to_flash(rt_uint8_t n, rt_uint8_t step, UN_STR *pun);
//#define   Test_File_Name 		("\\PROGRAM.BIN")
//#define   Test_File_TITLE		" "
#include  "CH376_USB_UI.H"
static u8 file_export(void *arg)
{
	rt_uint32_t msg;
	struct panel_type *win;
	struct rect_type rect={190,140,200,300};
	struct font_info_t font={0,0xf000,0XE73C,1,0,16};
	struct rect_type rects={25,100,20,250};
	rt_uint32_t i=0;
	rt_uint8_t *p_data;
		win = sui_window_create(T_STR("文件导出","File Export"),&rect);

	font.panel = win;

	font_draw(94,44,&font,T_STR("确定导出吗？","Confirm Export ?"));
	font_draw(26,68,&font,T_STR("提示:将文件导出到USB存储设备中。","Tips:exported file into the USB"));

	clr_win(win,CL_orange,40,145,30,80);
	clr_win(win,CL_orange,180,145,30,80);
	
	font.fontcolor = 0X4208;
	
	font_draw(64,152,&font,T_STR("确 定","OK"));
	font_draw(204,152,&font,T_STR("返回","Return"));
	
	sui_window_update(win);
	USB_Device_Chg(USB_1);
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			switch(msg)
			{
				case KEY_ENTER | KEY_UP:

					msg = CH376DiskConnect();
					font.fontcolor = 0Xf000;
					
					
					if(msg != USB_INT_SUCCESS)
					{
						clr_win(win,0XE73C,2,32,165,295);
						
						font_draw(70,68,&font,T_STR("请插入USB存储设备...","Please insert the USB device"));
					
						sui_window_update(win);
						while (CH376DiskConnect() != USB_INT_SUCCESS)
						{ 
							if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/10) == RT_EOK)
							{
								switch(msg)
								{
									/* 返回 */
									case KEY_EXIT | KEY_UP:
									case KEY_F6 | KEY_UP:
										return 0;
								}
							}/* 检查U盘是否连接,等待U盘插入,对于SD卡,可以由单片机直接查询SD卡座的插拔状态引脚 */
						}
						clr_win(win,0XE73C,2,32,165,295);
						
						font_draw(70,68,&font,T_STR("USB存储设备已插入！","USB device already insert"));
					
						sui_window_update(win);
						rt_thread_delay(200);  /* 延时,可选操作,有的USB存储器需要几十毫秒的延时 */
					}
					
					clr_win(win,0XE73C,2,32,165,295);
					font_draw(70,68,&font,T_STR("等待就绪......","Wait for ready..."));
					
					
					sui_window_update(win);
					rt_thread_delay(100);  /* 延时,可选操作,有的USB存储器需要几十毫秒的延时 */
					
					/* 对于检测到USB设备的,最多等待100*50mS,主要针对有些MP3太慢,对于检测到USB设备并且连接DISK_MOUNTED的,最多等待5*50mS,主要针对DiskReady不过的 */
					for (i = 0; i < 100; i ++ )
					{  /* 最长等待时间,100*50mS */
						msg = CH376DiskMount();  /* 初始化磁盘并测试磁盘是否就绪 */
						if (msg == USB_INT_SUCCESS)
						{
							break;  /* 准备好 */
						}
						else if (msg == ERR_DISK_DISCON)
						{
							break;  /* 检测到断开,重新检测并计时 */
						}
						if (CH376GetDiskStatus() >= DEF_DISK_MOUNTED && i >= 5)
						{
							break;  /* 有的U盘总是返回未准备好,不过可以忽略,只要其建立连接MOUNTED且尝试5*50mS */
						}
					}
					if (msg == ERR_DISK_DISCON)
					{
						clr_win(win,0XE73C,2,32,165,295);
						font_draw(70,68,&font,T_STR("不识别的USB设备！","Cannot be identified USB"));
						
						sui_window_update(win);
						rt_thread_delay(500);  /* 延时,可选操作,有的USB存储器需要几十毫秒的延时 */
						return 1;
					}
					
					{
						float   sn=0,n=0;
						u32		__file_len= 0;
						u16		timeout = 500;
						char buf_path[40];
						PUINT8 File_Name = (PUINT8)buf_path;
						rt_sprintf(buf_path,"\\FILE%03d.BIN",0);
						while(CH376FileCreate(File_Name) != USB_INT_SUCCESS && timeout--)rt_thread_delay(10);
						if(timeout == 0){
							clr_win(win,0XE73C,2,32,165,295);
						
							font_draw(70,68,&font,T_STR("不识别的USB设备！","Cannot be identified USB"));
							sui_window_update(win);
							rt_thread_delay(500);  /* 延时,可选操作,有的USB存储器需要几十毫秒的延时 */
							return 1;
						}
						else 
						{
							msg = CH376FileClose(TRUE);
							if(msg != USB_INT_SUCCESS){
								clr_win(win,0XE73C,2,32,165,295);
								
								font_draw(70,68,&font,T_STR("文件出错！","File error"));
							
								sui_window_update(win);
								rt_thread_delay(500);  /* 延时,可选操作,有的USB存储器需要几十毫秒的延时 */
								return 1;
							}
						}
						
						clr_win(win,0XE73C,2,32,165,295);
						clr_win(win,0XC618,25,100,20,250);
						font_draw(70,68,&font,T_STR("开始导出...","Start export..."));
						
						sui_window_update(win);
						
						msg = CH376FileOpen(File_Name);  //打开文件
						
						
						
						msg = CH376ByteWrite((u8 *)&flash_info,sizeof(flash_info),NULL);  //写入FLASH信息
						
//						fd = open(("/system32/temp/file_bk.bin"), O_CREAT | O_WRONLY , 0);
//						write(fd, (u8 *)&flash_info, sizeof(flash_info));
//						close(fd);
						
						__file_len += sizeof(flash_info);
						msg = CH376FileClose(TRUE);
						rt_thread_delay(50);
						
						sn = 250.0 / (FILE_NUM * 2);
						n=0;
						p_data = rt_malloc(FILE_OFFSET);
						FLASH_CS_SET(1);	// 选择参数flash
						for(i=0;i<FILE_NUM;i++)
						{
							
							sf_ReadBuffer((uint8_t*)p_data,                                                    \
														FILE_BASE + (i/PER_P_FILES)*4096 + FILE_OFFSET*(i%PER_P_FILES),      \
														FILE_OFFSET);
							
//							msg = CH376FileOpen(File_Name);  //打开文件
//							msg = CH376ByteLocate(__file_len);
//							msg = CH376ByteWrite((u8 *)p_data,FILE_OFFSET,NULL);  //写入文件信息
//							msg = CH376FileClose(TRUE);
							while(CH376FileOpen(File_Name) != 0x14);  //打开文件
							while(CH376ByteLocate(__file_len) != 0x14);
							while(CH376ByteWrite((u8 *)p_data,FILE_OFFSET,NULL) != 0x14);  //写入文件信息
							while(CH376FileClose(TRUE) != 0x14);
							__file_len += FILE_OFFSET;
							
							
//							fd = open(("/system32/temp/file_bk.bin"), O_CREAT | O_WRONLY , 0);
//							lseek(fd,__file_len,0);
//							write(fd, (u8 *)&p_data, FILE_OFFSET);
//							close(fd);
							
							n+=sn;
							clr_win(win,CL_BLUE,25,100,20,n);
							window_updata(win,&rects);
						}
						rt_free(p_data);
						p_data = rt_malloc(STEP_OFFSET);
						for(i=0;i<FILE_NUM;i++)
						{
							rt_uint32_t j;
							
							for(j=0;j<TOTALSTEP_NUM;j++)
							{
//								p = j % PER_P_STEPS;
//								sf_ReadBuffer((rt_uint8_t*)(p_data),
//															GROUP_BASE + i*GROUP_OFFSET + p*F_PAGE_SIZE + STEP_OFFSET*(j / PER_P_STEPS),
//															STEP_OFFSET);
//								msg = CH376FileOpen(File_Name);  //打开文件
//								msg = CH376ByteLocate(__file_len);
//								msg = CH376ByteWrite((u8 *)p_data,STEP_OFFSET,NULL);  //写入文件信息
//								msg = CH376FileClose(TRUE);
								
								read_flash_to_step(i,j,(UN_STR *)p_data);
								
								while(CH376FileOpen(File_Name) != 0x14);  //打开文件
								while(CH376ByteLocate(__file_len) != 0x14);
								while(CH376ByteWrite((u8 *)p_data,STEP_OFFSET,NULL) != 0x14);  //写入文件信息
								while(CH376FileClose(TRUE) != 0x14);
//								fd = open(("/system32/temp/file_bk.bin"), O_CREAT | O_WRONLY , 0);
//								lseek(fd,__file_len,0);
//								write(fd, (u8 *)&p_data, STEP_OFFSET);
//								close(fd);
								
								__file_len += STEP_OFFSET;
								
								n += sn / TOTALSTEP_NUM;
								clr_win(win,CL_BLUE,25,100,20,n);
								window_updata(win,&rects);
							}
							
							
							
						}
						rt_free(p_data);
						
						msg = CH376FileClose(TRUE);      //关闭文件
					}
					
					clr_win(win,0XE73C,70,68,20,100);
					
					font_draw(70,68,&font,T_STR("导出完成！","Export finish !"));
					
					sui_window_update(win);
					rt_thread_delay(200);
					
					return 0;
				/* 返回 */
					case KEY_EXIT | KEY_UP:
					case KEY_F6 | KEY_UP:
					return 0;
			}
		}
	}
	
}

extern void refresh_file_list_box(void);

static u8 file_import(void *arg)
{
	rt_uint32_t msg;
	struct panel_type *win;
	struct rect_type rect={190,140,200,300};
	struct font_info_t font={0,0xf000,0XE73C,1,0,16};
	struct rect_type rects={25,100,20,250};
	rt_uint32_t i=0;
	rt_uint8_t *p_data;
	
	win = sui_window_create(T_STR("文件导入","File inport"),&rect);
	
	font.panel = win;
	
	font_draw(94,44,&font,T_STR("确定导入吗？","Confirm inport ?"));
	font_draw(26,68,&font,T_STR("提示:将USB存储设备导入到文件中。","Tips:exported USB into the file"));
	
// 	clr_win(win,0XC618,25,100,20,250);

	clr_win(win,CL_orange,40,145,30,80);
	clr_win(win,CL_orange,180,145,30,80);
	
	font.fontcolor = 0X4208;
	font_draw(64,152,&font,T_STR("确 定","OK"));
	font_draw(204,152,&font,T_STR("返回","Return"));
	
	sui_window_update(win);
	USB_Device_Chg(USB_1);
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			switch(msg)
			{
				case KEY_ENTER | KEY_UP:
					msg = CH376DiskConnect();
					font.fontcolor = 0Xf000;
					
					
					if(msg != USB_INT_SUCCESS)
					{
						clr_win(win,0XE73C,2,32,165,295);
						font_draw(70,68,&font,T_STR("请插入USB存储设备...","Please insert the USB device"));
						sui_window_update(win);
						
						while (CH376DiskConnect() != USB_INT_SUCCESS)
						{ 
							if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/10) == RT_EOK)
							{
								switch(msg)
								{
									/* 返回 */
									case KEY_EXIT | KEY_UP:
									case KEY_F6 | KEY_UP:
										return 0;
								}
							}/* 检查U盘是否连接,等待U盘插入,对于SD卡,可以由单片机直接查询SD卡座的插拔状态引脚 */
						}
						clr_win(win,0XE73C,2,32,165,295);
						font_draw(70,68,&font,T_STR("USB存储设备已插入！","USB device already insert"));
						sui_window_update(win);
						rt_thread_delay(200);  /* 延时,可选操作,有的USB存储器需要几十毫秒的延时 */
					}
					
					clr_win(win,0XE73C,2,32,165,295);
					font_draw(70,68,&font,T_STR("等待就绪......","Wait for ready..."));
					sui_window_update(win);
					rt_thread_delay(100);  /* 延时,可选操作,有的USB存储器需要几十毫秒的延时 */
					
					/* 对于检测到USB设备的,最多等待100*50mS,主要针对有些MP3太慢,对于检测到USB设备并且连接DISK_MOUNTED的,最多等待5*50mS,主要针对DiskReady不过的 */
					for (i = 0; i < 100; i ++ )
					{  /* 最长等待时间,100*50mS */
						msg = CH376DiskMount();  /* 初始化磁盘并测试磁盘是否就绪 */
						if (msg == USB_INT_SUCCESS)
						{
							break;  /* 准备好 */
						}
						else if (msg == ERR_DISK_DISCON)
						{
							break;  /* 检测到断开,重新检测并计时 */
						}
						if (CH376GetDiskStatus() >= DEF_DISK_MOUNTED && i >= 5)
						{
							break;  /* 有的U盘总是返回未准备好,不过可以忽略,只要其建立连接MOUNTED且尝试5*50mS */
						}
					}
					if (msg == ERR_DISK_DISCON)
					{
						clr_win(win,0XE73C,2,32,165,295);
						font_draw(70,68,&font,T_STR("不识别的USB设备！","Cannot be identified USB"));
						sui_window_update(win);
						rt_thread_delay(500);  /* 延时,可选操作,有的USB存储器需要几十毫秒的延时 */
						return 1;
					}
					
					{

						float   sn=0,n=0;
						u32		__file_len= 0;
						u16		timeout = 500;
						char buf_path[40];
						uint16_t  read_num;
						PUINT8 File_Name = (PUINT8)buf_path;
						rt_sprintf(buf_path,"\\FILE%03d.BIN",0);
						while(OPEN_FILE((char *)File_Name) != DEF_DISK_OPEN_FILE && timeout--)rt_thread_delay(10);
						if(timeout == 0){
								clr_win(win,0XE73C,2,32,165,295);
								font_draw(70,68,&font,T_STR("不识别的USB设备！","Cannot be identified USB"));
								sui_window_update(win);
								rt_thread_delay(500);  /* 延时,可选操作,有的USB存储器需要几十毫秒的延时 */
								return 1;
						}
						else 
						{
	
							msg = CH376FileClose(TRUE);
							if(msg != USB_INT_SUCCESS){
								clr_win(win,0XE73C,2,32,165,295);
								font_draw(70,68,&font,T_STR("文件出错！","File error"));
								sui_window_update(win);
								rt_thread_delay(500);  /* 延时,可选操作,有的USB存储器需要几十毫秒的延时 */
								return 1;
							}
						}
						
						clr_win(win,0XE73C,2,32,165,295);
						clr_win(win,0XC618,25,100,20,250);
						font_draw(70,68,&font,T_STR("开始导入...","Start inport..."));
						
						sui_window_update(win);
						
						msg = CH376FileOpen(File_Name);  //打开文件
						
						
						
//						msg = CH376ByteWrite((u8 *)&flash_info,sizeof(flash_info),NULL);  //写入FLASH信息
						CH376ByteRead((u8 *)&flash_info, sizeof(flash_info), &read_num);    //读出FLASH信息
						sf_WriteBuffer((uint8_t *)&flash_info,FLASH_BKP_ADDR,sizeof(flash_info));

						
						__file_len += sizeof(flash_info);
						msg = CH376FileClose(TRUE);
						rt_thread_delay(50);
						
						sn = 250.0 / (FILE_NUM * 2);
						n=0;
						p_data = rt_malloc(FILE_OFFSET);
						FLASH_CS_SET(1);	// 选择参数flash
						for(i=0;i<FILE_NUM;i++)
						{
							
							
						
							while(CH376FileOpen(File_Name) != 0x14);  //打开文件
							while(CH376ByteLocate(__file_len) != 0x14);
							while(CH376ByteRead((u8 *)p_data,FILE_OFFSET,&read_num) != 0x14);  //写入文件信息
							while(CH376FileClose(TRUE) != 0x14);
							__file_len += FILE_OFFSET;
							
							sf_WriteBuffer((uint8_t*)p_data,                                                    \
														 FILE_BASE + (i/PER_P_FILES)*4096 + FILE_OFFSET*(i%PER_P_FILES),      \
														 FILE_OFFSET);
//							rt_memcpy(&file_info[i],(uint8_t*)p_data,FILE_OFFSET);
							
							n+=sn;
							clr_win(win,CL_BLUE,25,100,20,n);
							window_updata(win,&rects);
						}
						rt_free(p_data);
						p_data = rt_malloc(STEP_OFFSET);
						for(i=0;i<FILE_NUM;i++)
						{
							rt_uint32_t j;
							
							for(j=0;j<TOTALSTEP_NUM;j++)
							{
								
								
								while(CH376FileOpen(File_Name) != 0x14);  //打开文件
								while(CH376ByteLocate(__file_len) != 0x14);
								while(CH376ByteRead((u8 *)p_data,STEP_OFFSET,&read_num) != 0x14);  //写入文件信息
								while(CH376FileClose(TRUE) != 0x14);	
								__file_len += STEP_OFFSET;
								
//								p = j % PER_P_STEPS;
//								sf_WriteBuffer((rt_uint8_t*)(p_data),
//															 GROUP_BASE + i*GROUP_OFFSET + p*F_PAGE_SIZE + STEP_OFFSET*(j / PER_P_STEPS),
//															 STEP_OFFSET);
								save_step_to_flash(i,j,(UN_STR *)p_data);
								
								n += sn / TOTALSTEP_NUM;
								clr_win(win,CL_BLUE,25,100,20,n);
								window_updata(win,&rects);
							}
							
							
							
						}
						rt_free(p_data);
						
						msg = CH376FileClose(TRUE);      //关闭文件
					}
					
					init_list();
					read_flash_to_files();
					read_flash_to_list(flash_info.current_file);
					
					clr_win(win,0XE73C,70,68,20,100);
					font_draw(70,68,&font,T_STR("导入完成！","Inport finish !"));
					sui_window_update(win);
					rt_thread_delay(200);
					refresh_file_list_box();
					
					return 0;
				/* 返回 */
					case KEY_EXIT | KEY_UP:
					case KEY_F6 | KEY_UP:
					return 0;
			}
		}
	}
	
}

void ui_file_port_thread(void)
{
	rt_uint8_t i,exporten = 0;
	rt_uint32_t msg;
	
	for(i=1;i<FILE_NUM;i++)
	{
		if(file_info[i].en != 0)
		{
			exporten = 1;
			break;
		}
		
	}
	exporten = 1;
	if(panel_flag == 7)
	{

	
		if(exporten == 0)
			ui_key_updata(0x78);
		else
			ui_key_updata(0x38);
	}	
	while(panel_flag == 7)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			switch(msg)
			{
				/* 导入 */
				case KEY_F1 | KEY_UP:
					file_import(0);
					rt_mb_send(&screen_mb, UPDATE_HOME);
					break;
				/* 导出 */
				case KEY_F2 | KEY_UP:
					file_export(0);
					rt_mb_send(&screen_mb, UPDATE_HOME);
					break;
				/* 无效 */
				case KEY_F3 | KEY_UP:
					
					break;
				/* 无效 */
				case KEY_F4 | KEY_UP:
					
					break;
				/* 无效 */
				case KEY_F5 | KEY_UP:
					
					break;
				/* 返回 */
				case KEY_F6 | KEY_UP:
					panel_flag = 0;
					break;
				case CODE_RIGHT:
				case KEY_U | KEY_DOWN:
					
					break;
				case CODE_LEFT:
				case KEY_D | KEY_DOWN:
					
					break;
				case KEY_L | KEY_DOWN:
					
					break;
				case KEY_R | KEY_DOWN:
					
					break;
				default:
					break;
			}
		}
		else
		{
			
		}
	}
}


