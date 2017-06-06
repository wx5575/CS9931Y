#include "CS99xx.h"
#include "bsp_listbox.h"

#include "sui_window.h"
#include "memorymanagement.h"
#include "CS99xx_cal.h"

#include "JJ98.H"
#include   "Output_Control.h"
#include   "Cal.h"
#include   "GR.h"
#include   "LC.h"
#include   "Relay_Change.h"
#include   "Relay.h"
// #include   "mb.h"
#include "Usart1.h"
#include   "spi_cpld.h"
#include   "CS9931_Config.h"
#include   "AD_DA.h"

#define	_system_password		(system_parameter_t.psd)
#define	_system_environment		(system_parameter_t.env)
#define	_system_communication	(system_parameter_t.com)
#define	_system_operationlog	(system_parameter_t.ope)


extern rt_uint32_t	num_input(struct font_info_t *font,struct rect_type *rect,struct num_format *num);
extern void RA8875_SetBackLight(uint8_t _bright);

static void system_item_draw(struct font_info_t *font,rt_uint16_t index,rt_uint16_t x,rt_uint16_t y);
static u8 system_time_setting(void *arg);
static u8 system_password_setting(void *arg);
static u8 system_environment_setting(void *arg);
static u8 system_communication_setting(void *arg);
static u8 system_run_setting(void *arg);
static u8 system_default_setting(void *arg);
static u8 system_mode_setting(void *arg);
static u8 system_arc_setting(void *arg);
//static void change_curdetection(uint8_t curdetection);
void dyj_save_IR_ad(uint32_t *p_Vol,uint16_t *p_ADValue,uint8_t gear);

static u8 system_calibration(void *arg);
struct 
{
    u16 x;
	u16 y;
	const char *data_en;
	const char *data_ch;
}system_title_name={10,10,"SystemParameter:","系统设置:"};

static const struct rect_type list_box_rect={190,65,26,300};
struct rtgui_listctrl system_list_box={
	&panel_home,
	7,
	7,
	0,0,
	(struct rect_type *)&list_box_rect,
	system_item_draw
};

static struct list_item
{
	char name[2][22];
} items[8] =
{
	{"时间参数","Time Parameter"},
	{"密码参数","Password Parameter"},
	{"环境参数","Environment Parameter"},
// 	{"泄漏人体模拟网络","Leak Met Work"},
	{"通讯配置","Communication config"},
	{"运行日志","Run Diary"},
	{"默认设置","Default Set"},
	{"模式校准","Mode Calibrate"},
// 	{"电弧校准","ARC Calibrate"},
};




static void cal_Delay_ms(unsigned int dly_ms)
{
  unsigned int dly_i;
  while(dly_ms--)
    for(dly_i=0;dly_i<18714;dly_i++);
}

void ui_system_thread(void)
{
// 	u8	i;
	rt_uint32_t msg;
	struct font_info_t font={&panel_home,CL_YELLOW,0x0,1,0,20};

__system_loop:
	if(panel_flag == 3)
	{
		clr_mem((u16 *)ExternSramHomeAddr,0x19f6,ExternSramHomeSize/2);
// 		rt_memcpy((void *)ExternSramHomeAddr,(void *)ExternSramNullAddr,ExternSramHomeSize);
		/* 标题 */
			/* 语言判断 */
			if(language==1)
				font_draw(system_title_name.x,system_title_name.y,&font,system_title_name.data_en);
			else
				font_draw(system_title_name.x,system_title_name.y,&font,system_title_name.data_ch);
		listbox_draw(&system_list_box);
		rt_mb_send(&screen_mb, UPDATE_HOME);
		ui_key_updata(0);
	}
	while(panel_flag == 3)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			switch(msg)
			{
				/* 返回 */
				case KEY_F6 | KEY_UP:
					panel_flag = 0;
					break;
				case CODE_RIGHT:
				case KEY_U | KEY_DOWN:
					if(system_list_box.current_item>0)system_list_box.current_item --;
					else system_list_box.current_item = system_list_box.total_items-1;
					listbox_draw(&system_list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
					break;
				case CODE_LEFT:
				case KEY_D | KEY_DOWN:
					if(system_list_box.current_item<system_list_box.total_items-1)system_list_box.current_item ++;
					else system_list_box.current_item = 0;
					listbox_draw(&system_list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
					break;
				case KEY_ENTER | KEY_UP:
					switch(system_list_box.current_item)
					{
						/* 时间设置 */
						case 0:
							system_time_setting(0);
							break;
						case 1:
							system_password_setting(0);
							break;
						case 2:
							system_environment_setting(0);
							break;
						case 3:
							system_communication_setting(0);
							break;
						case 4:
							system_run_setting(0);
							break;
						case 5:
							system_default_setting(0);
							break;
						case 6:
							if(0==system_mode_setting(0))
							{
								system_calibration(0);
								goto __system_loop;
							}
							break;
						case 7:
							system_arc_setting(0);
							break;
			
					}
					rt_mb_send(&screen_mb, UPDATE_HOME);
// 					goto __system_loop;
// 					listbox_draw(&system_list_box);
// 					rt_mb_send(&screen_mb, UPDATE_HOME);
// 					break;
				default:
					break;
			}
		}
		else
		{
			
		}
	}
}



void dyj_output(u8 mode, u16 value);
void dyj_save_ad(u8 mode, u8 item, u8 cal, u32 val);
void dyj_save_lc_selv_ad(u32 val);
void dyj_save_lc_cur_ad(u8 network, u8 dem, u8 cal, u32 val);
void dyj_lc_relay(u8 network, u8 dem, u8 cal);
static void cal_item_draw(struct font_info_t *font,rt_uint16_t index,rt_uint16_t x,rt_uint16_t y);
static struct rect_type list_box_rects={10,40,26,660};
static struct rtgui_listctrl cal_list_box={
	&panel_home,
	13,
	70,//总数
	0,0,
	&list_box_rects,
	cal_item_draw
};
enum{
	CAL_ACW_VOL_1,
	CAL_ACW_VOL_2,
	CAL_ACW_VOL_3,
	CAL_ACW_CUR_1,
	CAL_ACW_CUR_2,
	CAL_ACW_CUR_3,
	CAL_ACW_CUR_4,
	CAL_ACW_ARC,
	CAL_DCW_VOL_1,
	CAL_DCW_VOL_2,
	CAL_DCW_VOL_3,
	CAL_DCW_CUR_1,
	CAL_DCW_CUR_2,
	CAL_DCW_CUR_3,
	CAL_DCW_CUR_4,
	CAL_DCW_CUR_5,
	CAL_DCW_CUR_6,
	CAL_DCW_ARC,
	CAL_GR_CUR_1,
	CAL_GR_CUR_2,
	CAL_GR_CUR_3,
	CAL_LC_MVOL_1,
	CAL_LC_MVOL_2,
	CAL_LC_MVOL_3,
	CAL_LC_MCUR,
	CAL_LC_SVOL_1,
	CAL_LC_SVOL_2,
	CAL_LC_SVOL_3,
	CAL_LC_EAC_CUR_1,
	CAL_LC_EAC_CUR_2,
	CAL_LC_EAC_CUR_3,
	CAL_LC_EAC_DC_CUR_1,
	CAL_LC_EAC_DC_CUR_2,
	CAL_LC_EAC_DC_CUR_3,
	CAL_LC_EPEAK_CUR_1,
	CAL_LC_EPEAK_CUR_2,
	CAL_LC_EPEAK_CUR_3,
	CAL_LC_EDC_CUR_1,
	CAL_LC_EDC_CUR_2,
	CAL_LC_EDC_CUR_3,
	CAL_LC_FAC_CUR_1,
	CAL_LC_FAC_CUR_2,
	CAL_LC_FAC_CUR_3,
	CAL_LC_FAC_DC_CUR_1,
	CAL_LC_FAC_DC_CUR_2,
	CAL_LC_FAC_DC_CUR_3,
	CAL_LC_FPEAK_CUR_1,
	CAL_LC_FPEAK_CUR_2,
	CAL_LC_FPEAK_CUR_3,
	CAL_LC_FDC_CUR_1,
	CAL_LC_FDC_CUR_2,
	CAL_LC_FDC_CUR_3,
	CAL_LC_GAC_CUR_1,
	CAL_LC_GAC_CUR_2,
	CAL_LC_GAC_CUR_3,
	CAL_LC_GAC_DC_CUR_1,
	CAL_LC_GAC_DC_CUR_2,
	CAL_LC_GAC_DC_CUR_3,
	CAL_LC_GPEAK_CUR_1,
	CAL_LC_GPEAK_CUR_2,
	CAL_LC_GPEAK_CUR_3,
	CAL_LC_GDC_CUR_1,
	CAL_LC_GDC_CUR_2,
	CAL_LC_GDC_CUR_3,
	CAL_LC_SELF_VOL,
	CAL_IR_RES_1,
	CAL_IR_RES_2,
	CAL_IR_RES_3,
	CAL_IR_RES_4,
	CAL_IR_RES_5,
};
static char *cal_item_name[]={
	"001       ACW           电压0.100kV            -----          -----",
	"002       ACW           电压2.000kV            -----          -----",
	"003       ACW           电压5.000kV            -----          -----",
	"004       ACW           电流300uA档            -----          -----",
	"005       ACW           电流  3mA档            -----          -----",
	"006       ACW           电流 30mA档            -----          -----",
	"007       ACW           电流100mA档            -----          -----",
	"008       ACW            电弧侦测              -----          -----",
	"009       DCW           电压0.100kV            -----          -----",
	"010       DCW           电压2.000kV            -----          -----",
	"011       DCW           电压6.000kV            -----          -----",
	"012       DCW           电流  3uA档            -----          -----",
	"013       DCW           电流 30uA档            -----          -----",
	"014       DCW           电流300uA档            -----          -----",
	"015       DCW           电流  3mA档            -----          -----",
	"016       DCW           电流 30mA档            -----          -----",
	"017       DCW           电流100mA档            -----          -----",
	"018       DCW            电弧侦测              -----          -----",
	"019        GR           3.00A->100mΩ          -----          -----",
	"020        GR          10.00A->100mΩ          -----          -----",
	"021        GR          30.00A->100mΩ          -----          -----",
	
#ifdef LC_MAX_VOL_250V
	"022        LC           电压 30.0V             -----          -----",
	"023        LC           电压150.0V             -----          -----",
	"024        LC           电压250.0V             -----          -----",
#else
	"022        LC           电压 30.0V             -----          -----",
	"023        LC           电压150.0V             -----          -----",
	"024        LC           电压300.0V             -----          -----",
#endif
	"024        LC           电流1.0A               -----          -----",
	"025        LC         辅助电压 30.0V           -----          -----",
	"026        LC         辅助电压150.0V           -----          -----",
	"027        LC         辅助电压250.0V           -----          -----",

	"028        LC          MD-E/AC/300uA           -----          -----",
	"029        LC          MD-E/AC/  3mA           -----          -----",
	"030        LC          MD-E/AC/ 30mA           -----          -----",
	"031        LC          MD-E/AC+DC/300uA        -----          -----",
	"032        LC          MD-E/AC+DC/  3mA        -----          -----",
	"033        LC          MD-E/AC+DC/ 30mA        -----          -----",
	"034        LC          MD-E/PEAK/300uA         -----          -----",
	"035        LC          MD-E/PEAK/  3mA         -----          -----",
	"036        LC          MD-E/PEAK/ 30mA         -----          -----",
	"037        LC          MD-E/DC/300uA           -----          -----",
	"038        LC          MD-E/DC/  3mA           -----          -----",
	"039        LC          MD-E/DC/ 30mA           -----          -----",
	"040        LC          MD-F/AC/300uA           -----          -----",
	"041        LC          MD-F/AC/  3mA           -----          -----",
	"042        LC          MD-F/AC/ 30mA           -----          -----",
	"043        LC          MD-F/AC+DC/300uA        -----          -----",
	"044        LC          MD-F/AC+DC/  3mA        -----          -----",
	"045        LC          MD-F/AC+DC/ 30mA        -----          -----",
	"046        LC          MD-F/PEAK/300uA         -----          -----",
	"047        LC          MD-F/PEAK/  3mA         -----          -----",
	"048        LC          MD-F/PEAK/ 30mA         -----          -----",
	"049        LC          MD-F/DC/300uA           -----          -----",
	"050        LC          MD-F/DC/  3mA           -----          -----",
	"051        LC          MD-F/DC/ 30mA           -----          -----",
	"052        LC          MD-G/AC/300uA           -----          -----",
	"053        LC          MD-G/AC/  3mA           -----          -----",
	"054        LC          MD-G/AC/ 30mA           -----          -----",
	"055        LC          MD-G/AC+DC/300uA        -----          -----",
	"056        LC          MD-G/AC+DC/  3mA        -----          -----",
	"057        LC          MD-G/AC+DC/ 30mA        -----          -----",
	"058        LC          MD-G/PEAK/300uA         -----          -----",
	"059        LC          MD-G/PEAK/  3mA         -----          -----",
	"060        LC          MD-G/PEAK/ 30mA         -----          -----",
	"061        LC          MD-G/DC/300uA           -----          -----",
	"062        LC          MD-G/DC/  3mA           -----          -----",
	"063        LC          MD-G/DC/ 30mA           -----          -----",
	"064        LC          SELF电压 10.0V          -----          -----",

	"065        IR          绝缘电阻  2MΩ          -----          -----",
	"066        IR          绝缘电阻  20MΩ         -----          -----",
	"067        IR          绝缘电阻 200MΩ         -----          -----",
	"068        IR          绝缘电阻   2GΩ         -----          -----",
	"069        IR          绝缘电阻   10Ω         -----          -----",
};
static u8 system_calibration(void *arg)
{
	rt_uint32_t msg;
	struct font_info_t font={&panel_home,CL_YELLOW,0x0,1,0,20};
__cal_display:
	clr_mem((u16 *)ExternSramHomeAddr,0x19f6,ExternSramHomeSize/2);
	/* 标题 */
	/* 语言判断 */
	if(language==1)
		font_draw(10,10,&font,"English");
	else
		font_draw(10,10,&font,"编号~~~~模式~~~~~~~~校准项目~~~~~~~~实际电压~~~~实际电流");
	listbox_draw(&cal_list_box);
	rt_mb_send(&screen_mb, UPDATE_HOME);
	ui_key_updata(0);
	
	
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			switch(msg)
			{
				/* 返回 */
				case KEY_F6   | KEY_UP:
				case KEY_EXIT | KEY_UP:
						
				  /*douyijun  此处添加校准系数保存函数*/
					Cal_Facter_Reserve();
					return 0;
				case CODE_RIGHT:
				case KEY_U | KEY_DOWN:
					if(cal_list_box.current_item>0)cal_list_box.current_item --;
					cal_list_box.start_item = (cal_list_box.current_item / cal_list_box.items_count)*cal_list_box.items_count;
// 					g_cur_step = position_step(cal_list_box.start_item+1);
					listbox_draw(&cal_list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
				
// 					if(cal_list_box.current_item >= cs99xx_list.size)
					break;
				case CODE_LEFT:
				case KEY_D | KEY_DOWN:
					if(cal_list_box.current_item<cal_list_box.total_items-1)cal_list_box.current_item ++;
					cal_list_box.start_item = (cal_list_box.current_item / cal_list_box.items_count)*cal_list_box.items_count;
					listbox_draw(&cal_list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
					break;
				case KEY_L | KEY_DOWN:
					if(cal_list_box.current_item >= cal_list_box.items_count)
						cal_list_box.current_item -= cal_list_box.items_count;
					cal_list_box.start_item = (cal_list_box.current_item / cal_list_box.items_count)*cal_list_box.items_count;
					listbox_draw(&cal_list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
					break;
				case KEY_R | KEY_DOWN:
					cal_list_box.current_item += cal_list_box.items_count;
					if(cal_list_box.current_item>cal_list_box.total_items-1)cal_list_box.current_item=cal_list_box.total_items-1;
					cal_list_box.start_item = (cal_list_box.current_item / cal_list_box.items_count)*cal_list_box.items_count;
					listbox_draw(&cal_list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
					break;
				default:
					break;
			}
		}
		else
		{
#define			DISPLAY_CAL_VOL_X			370
#define			DISPLAY_CAL_CUR_X			490
#define			DISPLAY_CAL_H				20
#define			DISPLAY_CAL_W				80			
#define			DISPLAY_CAL_Y				43
			
			if(GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_7) == 0)
			{
				rt_thread_delay( RT_TICK_PER_SECOND / 100);
				if(GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_7) == 0)
				{
					u32 temp;
					struct rect_type rects={190,140,200,300};
					struct font_info_t fonts={&panel_home,0Xffff,0x53fa,1,1,16};
					struct num_format nums;
					
// 					{
// 						u8 i;
// 						if(cal_list_box.current_item+1>=28){
// 							LC_Main_Output_Enable();
// 							LC_Main_Voltage_Set(100,60);
// 							Relay_ON(EXT_DRIVER_O8);
// 							for(i=0;i<20;i++)
// 							{
// 								D3_Mcp3202_Read(0);
// 								cal_Delay_ms(100);
// 							}
// 							LC_Main_Voltage_Set(0,60);
// 							Relay_OFF(EXT_DRIVER_O8);
// 							cal_Delay_ms(5000);
// 						}
// 					}
					
					switch(cal_list_box.current_item)
					{
						// ACW --------------------------------------------------------->
						case CAL_ACW_VOL_1:// ACW 100V
						{
							dyj_output(ACW, DA_ACW_100V);
							Relay_ON(ACW_DCW_IR);
							nums.num  = 100;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 50;
							nums.max  = 300;
							nums.unit = "kV";
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(ACW,CAL_VOL,1,temp);
							dyj_output(ACW,0);
							Relay_OFF(ACW_DCW_IR);
							if(temp == 0xffffffff)goto __cal_display;
							break;
						}
						case CAL_ACW_VOL_2:// ACW 2000V
						{
							dyj_output(ACW,DA_ACW_2000V);
							Relay_ON(ACW_DCW_IR);
							nums.num  = 2000;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 1500;
							nums.max  = 3000;
							nums.unit = "kV";
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(ACW,CAL_VOL,2,temp);
							dyj_output(ACW,0);
							Relay_OFF(ACW_DCW_IR);
							if(temp == 0xffffffff)goto __cal_display;
							break;
						}
						case CAL_ACW_VOL_3:// ACW 5000V
						{
							dyj_output(ACW,DA_ACW_5000V);
							Relay_ON(ACW_DCW_IR);
							nums.num  = 5000;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 4500;
							nums.max  = 6000;
							nums.unit = "kV";
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(ACW,CAL_VOL,3,temp);
							dyj_output(ACW,0);
							Relay_OFF(ACW_DCW_IR);
							if(temp == 0xffffffff)goto __cal_display;
							break;
						}
						case CAL_ACW_CUR_1:// 300uA
						{
							Sampling_Relay_State_CHange(DC_200uA);
							AC_Output_Enable();
							AC_SetVoltage(DA_ACW_CURCAL_VOL,60);
							Relay_ON(ACW_DCW_IR);
							nums.num  = 3000;
							nums._int = 3;
							nums._dec = 1;
							nums.min  = 0;
							nums.max  = 9999;
							nums.unit = "uA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(ACW,CAL_CUR,1,temp);
							AC_SetVoltage(0,60);
							AC_Output_Disable();
							Relay_OFF(ACW_DCW_IR);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_ACW_CUR_2:// 3mA
						{
							Sampling_Relay_State_CHange(DC_2mA);
							AC_Output_Enable();
							AC_SetVoltage(DA_ACW_CURCAL_VOL,60);
							Relay_ON(ACW_DCW_IR);
							nums.num  = 3000;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 0;
							nums.max  = 9999;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(ACW,CAL_CUR,2,temp);
							AC_SetVoltage(0,60);
							AC_Output_Disable();
							Relay_OFF(ACW_DCW_IR);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_ACW_CUR_3:// 30mA
						{
							Sampling_Relay_State_CHange(DC_20mA);
							AC_Output_Enable();
							AC_SetVoltage(DA_ACW_CURCAL_VOL,60);
							Relay_ON(ACW_DCW_IR);
							nums.num  = 3000;
							nums._int = 2;
							nums._dec = 2;
							nums.min  = 0;
							nums.max  = 9999;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(ACW,CAL_CUR,3,temp);
							AC_SetVoltage(0,60);
							AC_Output_Disable();
							Relay_OFF(ACW_DCW_IR);
							if(temp == 0xffffffff)goto __cal_display;

							break;
						}
						case CAL_ACW_CUR_4:// 100mA
						{
							Sampling_Relay_State_CHange(DC_100mA);
							AC_Output_Enable();
							AC_SetVoltage(DA_ACW_CURCAL_VOL,60);
							Relay_ON(ACW_DCW_IR);
							nums.num  = 1000;
							nums._int = 3;
							nums._dec = 1;
							nums.min  = 0;
							nums.max  = 9999;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(ACW,CAL_CUR,4,temp);
							AC_SetVoltage(0,60);
							AC_Output_Disable();
							Relay_OFF(ACW_DCW_IR);
							if(temp == 0xffffffff)goto __cal_display;

							break;
						}
						case CAL_ACW_ARC:// ARC
						{
							nums.num  = Global_Cal_Facter.ARC_Facter.ACW_ARC_Base;
							nums._int = 4;
							nums._dec = 0;
							nums.min  = 0;
							nums.max  = 4095;
							nums.unit = "";
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp == 0xffffffff)goto __cal_display;
							dyj_save_ad(ACW,CAL_ARC,4,temp);
							break;
						}
						
						
						
						
						
						
						
						
						
						
						
						
						// DCW --------------------------------------------------------->
						case CAL_DCW_VOL_1: // 100v
						{
							dyj_output(DCW,DA_DCW_100V);
							Relay_ON(ACW_DCW_IR);
							nums.num  = 100;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 0;
							nums.max  = 200;
							nums.unit = "kV";
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(DCW,CAL_VOL,1,temp);
							dyj_output(DCW,0);
							Relay_OFF(ACW_DCW_IR);
							if(temp == 0xffffffff)goto __cal_display;
							break;
						}
						case CAL_DCW_VOL_2: // 2000v
						{
							dyj_output(DCW,DA_DCW_2000V);
							Relay_ON(ACW_DCW_IR);
							nums.num  = 2000;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 1000;
							nums.max  = 3000;
							nums.unit = "kV";
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(DCW,CAL_VOL,2,temp);
							dyj_output(DCW,0);
							Relay_OFF(ACW_DCW_IR);
							if(temp == 0xffffffff)goto __cal_display;
							break;
						}
						case CAL_DCW_VOL_3: // 6000v
						{
							dyj_output(DCW,DA_DCW_6000V);
							Relay_ON(ACW_DCW_IR);
							nums.num  = 6000;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 5000;
							nums.max  = 7000;
							nums.unit = "kV";
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(DCW,CAL_VOL,3,temp);
							dyj_output(DCW,0);
							Relay_OFF(ACW_DCW_IR);
							if(temp == 0xffffffff)goto __cal_display;
							break;
						}
						case CAL_DCW_CUR_1:// 3uA
						{
							Sampling_Relay_State_CHange(DC_2uA);
							DC_Output_Enable();
							DC_SetVoltage(DA_DCW_CURCAL_VOL);
							Relay_ON(ACW_DCW_IR);
							nums.num  = 3000;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 0;
							nums.max  = 9999;
							nums.unit = "uA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(DCW,CAL_CUR,1,temp);
							DC_SetVoltage(0);
							DC_Output_Disable();
							Relay_OFF(ACW_DCW_IR);
							if(temp == 0xffffffff)goto __cal_display;
			
							break;
						}
						case CAL_DCW_CUR_2: // 30uA
						{
							Sampling_Relay_State_CHange(DC_20uA);
							DC_Output_Enable();
							DC_SetVoltage(DA_DCW_CURCAL_VOL);
							Relay_ON(ACW_DCW_IR);
							nums.num  = 3000;
							nums._int = 2;
							nums._dec = 2;
							nums.min  = 0;
							nums.max  = 9999;
							nums.unit = "uA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(DCW,CAL_CUR,2,temp);
							DC_SetVoltage(0);
							DC_Output_Disable();
							Relay_OFF(ACW_DCW_IR);
							if(temp == 0xffffffff)goto __cal_display;
			
							break;
						}
						case CAL_DCW_CUR_3:// 300uA
						{
							Sampling_Relay_State_CHange(DC_200uA);
							DC_Output_Enable();
							DC_SetVoltage(DA_DCW_CURCAL_VOL);
							Relay_ON(ACW_DCW_IR);
							nums.num  = 3000;
							nums._int = 3;
							nums._dec = 1;
							nums.min  = 0;
							nums.max  = 9999;
							nums.unit = "uA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(DCW,CAL_CUR,3,temp);
							DC_SetVoltage(0);
							DC_Output_Disable();
							Relay_OFF(ACW_DCW_IR);
							if(temp == 0xffffffff)goto __cal_display;
				
							break;
						}
						case CAL_DCW_CUR_4:// 3mA
						{
							Sampling_Relay_State_CHange(DC_2mA);
							DC_Output_Enable();
							DC_SetVoltage(DA_DCW_CURCAL_VOL);
							Relay_ON(ACW_DCW_IR);
							nums.num  = 3000;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 0;
							nums.max  = 9999;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(DCW,CAL_CUR,4,temp);
							DC_SetVoltage(0);
							DC_Output_Disable();
							Relay_OFF(ACW_DCW_IR);
							if(temp == 0xffffffff)goto __cal_display;
						
							break;
						}
						case CAL_DCW_CUR_5:// 30mA
						{
							Sampling_Relay_State_CHange(DC_20mA);
							DC_Output_Enable();
							DC_SetVoltage(DA_DCW_CURCAL_VOL);
							Relay_ON(ACW_DCW_IR);
							nums.num  = 3000;
							nums._int = 2;
							nums._dec = 2;
							nums.min  = 0;
							nums.max  = 9999;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(DCW,CAL_CUR,5,temp);
							DC_SetVoltage(0);
							DC_Output_Disable();
							Relay_OFF(ACW_DCW_IR);
							if(temp == 0xffffffff)goto __cal_display;
					
							break;
						}
						case CAL_DCW_CUR_6:// 100mA
						{
							Sampling_Relay_State_CHange(DC_100mA);
							DC_Output_Enable();
							DC_SetVoltage(DA_DCW_CURCAL_VOL);
							Relay_ON(ACW_DCW_IR);
							nums.num  = 1000;
							nums._int = 3;
							nums._dec = 1;
							nums.min  = 0;
							nums.max  = 9999;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(DCW,CAL_CUR,6,temp);
							DC_SetVoltage(0);
							DC_Output_Disable();
							Relay_OFF(ACW_DCW_IR);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_DCW_ARC://ARC
						{
							nums.num  = Global_Cal_Facter.ARC_Facter.DCW_ARC_Base;
							nums._int = 4;
							nums._dec = 0;
							nums.min  = 0;
							nums.max  = 4095;
							nums.unit = "";
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp == 0xffffffff)goto __cal_display;
							dyj_save_ad(DCW,CAL_ARC,1,temp);
							break;
						}
						
						
						
						
						
						
						
						
						
						
						
						
						// GR --------------------------------------------------------->
						case CAL_GR_CUR_1: // 3A
						{
							Relay_ON(EXT_DRIVER_O7);
							Relay_ON(AMP_RELAY_1);
							dyj_output(GR,DA_GR_3A);
							nums.num  = 300;
							nums._int = 3;
							nums._dec = 0;
							nums.min  = 200;
							nums.max  = 400;
							nums.unit = "mV";
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(GR,CAL_VOL,1,temp);
							dyj_output(GR,0);
							Relay_OFF(EXT_DRIVER_O7);
							Relay_OFF(AMP_RELAY_1);
							if(temp == 0xffffffff)goto __cal_display;
							break;
						}
						case CAL_GR_CUR_2:
						{
							Relay_ON(EXT_DRIVER_O7);
							Relay_ON(AMP_RELAY_1);
							dyj_output(GR,DA_GR_10A);
							nums.num  = 1000;
							nums._int = 4;
							nums._dec = 0;
							nums.min  = 900;
							nums.max  = 1100;
							nums.unit = "mV";
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(GR,CAL_VOL,2,temp);
							dyj_output(GR,0);
							Relay_OFF(EXT_DRIVER_O7);
							Relay_OFF(AMP_RELAY_1);
							if(temp == 0xffffffff)goto __cal_display;
							break;
						}
						case CAL_GR_CUR_3:
						{
							Relay_ON(EXT_DRIVER_O7);
							Relay_ON(AMP_RELAY_1);
							dyj_output(GR,DA_GR_30A);
							nums.num  = 3000;
							nums._int = 4;
							nums._dec = 0;
							nums.min  = 2500;
							nums.max  = 3500;
							nums.unit = "mV";
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(GR,CAL_VOL,3,temp);
							dyj_output(GR,0);
							Relay_OFF(EXT_DRIVER_O7);
							Relay_OFF(AMP_RELAY_1);
							if(temp == 0xffffffff)goto __cal_display;
							break;
						}
						
						
						
						
						
						
						
						
						
						
						
						
						// LC --------------------------------------------------------->
						case CAL_LC_MVOL_1:
						{
							if(LC_TEST_MODE==LC_YY)
							{
								dyj_output(LC,DA_LC_CAL_VOL_P1);
							}
							else
							{
								dyj_output(LC, 30);
							}
							
						#ifdef LC_MAX_VOL_250V
							nums.num  = 250;
						#else
							nums.num  = 300;
						#endif
							nums._int = 2;
							nums._dec = 1;
							nums.min  = 150;
							nums.max  = 350;
							nums.unit = "V";
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(LC,CAL_VOL,1,temp);
							
							
							dyj_output(LC,0);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_MVOL_2:
						{
							if(LC_TEST_MODE==LC_YY)
							{
								dyj_output(LC,DA_LC_CAL_VOL_P2);
							}
							else
							{
								dyj_output(LC, 150);
							}
							
						#ifdef LC_MAX_VOL_250V
							nums.num  = 1500;
						#else
							nums.num  = 1500;
						#endif
							nums._int = 3;
							nums._dec = 1;
							nums.min  = 1000;
							nums.max  = 2000;
							nums.unit = "V";
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(LC,CAL_VOL,2,temp);
							
							
							dyj_output(LC,0);
							if(temp == 0xffffffff)goto __cal_display;
							break;
						}
						case CAL_LC_MVOL_3:
						{
							if(LC_TEST_MODE==LC_YY)
							{
								dyj_output(LC,DA_LC_CAL_VOL_P3);
							}
							else
							{
								dyj_output(LC, 300);
							}
							
						#ifdef LC_MAX_VOL_250V
							nums.num  = 2500;
						#else
							nums.num  = 3000;
						#endif
							nums._int = 3;
							nums._dec = 1;
							nums.min  = 1500;
							nums.max  = 3500;
							nums.unit = "V";
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							
							if(temp != 0xffffffff)
							{
								dyj_save_ad(LC,CAL_VOL,3,temp);
							}
							
							dyj_output(LC,0);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_MCUR:
						{
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(220000,60);//220V
							nums.num  = 1000;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 0;
							nums.max  = 5000;
							nums.unit = "A";
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							
							if(temp != 0xffffffff)
							{
								dyj_save_ad(LC,CAL_CUR,1,temp);
							}
							
							dyj_output(LC,0);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_SVOL_1:
						{
							dyj_output(LC_ASSIST, DA_LC_30VS);/* 设置DA值 */
                            Relay_ON(EXT_DRIVER_O8);
                            nums.num  = 300;
							nums._int = 2;
							nums._dec = 1;
							nums.min  = 250;
							nums.max  = 350;
							nums.unit = "V";
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
						  if(temp != 0xffffffff)
						  {
							  dyj_save_ad(LC_ASSIST,CAL_VOLS,1,temp);
						  }
						  
						  dyj_output(LC_ASSIST,0);
						  
							if(temp == 0xffffffff)
							{
								goto __cal_display;
							}
							break;
						}
						case CAL_LC_SVOL_2:
						{
							dyj_output(LC_ASSIST, DA_LC_150VS);/* 设置DA值 DA_LC_150VS */
                            Relay_ON(EXT_DRIVER_O8);
                            nums.num  = 1500;
							nums._int = 3;
							nums._dec = 1;
							nums.min  = 1000;
							nums.max  = 2000;
							nums.unit = "V";
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(LC_ASSIST,CAL_VOLS,2,temp);
							dyj_output(LC_ASSIST,0);
							if(temp == 0xffffffff)goto __cal_display;
							break;
						}
						case CAL_LC_SVOL_3:
						{
							dyj_output(LC_ASSIST,DA_LC_300VS);
                            Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 3;
							nums._dec = 1;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "V";
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_ad(LC_ASSIST,CAL_VOLS,3,temp);
							dyj_output(LC_ASSIST,0);
							if(temp == 0xffffffff)goto __cal_display;
							break;
						}
						// MD_E
						case CAL_LC_EAC_CUR_1: // 300uA
						{
							dyj_lc_relay(MD_E,0,1);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
							Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 3;
							nums._dec = 1;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "uA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_E,0,1,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_EAC_CUR_2:
						{
							dyj_lc_relay(MD_E,0,2);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_E,0,2,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_EAC_CUR_3:
						{
							dyj_lc_relay(MD_E,0,3);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 2;
							nums._dec = 2;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_E,0,3,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_EAC_DC_CUR_1:
						{
							dyj_lc_relay(MD_E,1,1);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 3;
							nums._dec = 1;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "uA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_E,1,1,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_EAC_DC_CUR_2:
						{
							dyj_lc_relay(MD_E,1,2);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_E,1,2,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_EAC_DC_CUR_3:
						{
							dyj_lc_relay(MD_E,1,3);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 2;
							nums._dec = 2;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_E,1,3,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_EPEAK_CUR_1:
						{
							dyj_lc_relay(MD_E,2,1);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 3;
							nums._dec = 1;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "uA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_E,2,1,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_EPEAK_CUR_2:
						{
							dyj_lc_relay(MD_E,2,2);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_E,2,2,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_EPEAK_CUR_3:
						{
							dyj_lc_relay(MD_E,2,3);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 2;
							nums._dec = 2;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_E,2,3,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_EDC_CUR_1:
						{
							dyj_lc_relay(MD_E,3,1);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Relay_Control(LC_OUT6,1,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 3;
							nums._dec = 1;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "uA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_E,3,1,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_EDC_CUR_2:
						{
							dyj_lc_relay(MD_E,3,2);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Relay_Control(LC_OUT6,1,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_E,3,2,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_EDC_CUR_3:
						{
							dyj_lc_relay(MD_E,3,3);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Relay_Control(LC_OUT6,1,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 2;
							nums._dec = 2;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_E,3,3,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						// MD_F
						case CAL_LC_FAC_CUR_1: // 300uA
						{
							dyj_lc_relay(MD_F,0,1);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 3;
							nums._dec = 1;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "uA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_F,0,1,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_FAC_CUR_2:
						{
							dyj_lc_relay(MD_F,0,2);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_F,0,2,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_FAC_CUR_3:
						{
							dyj_lc_relay(MD_F,0,3);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 2;
							nums._dec = 2;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_F,0,3,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_FAC_DC_CUR_1:
						{
							dyj_lc_relay(MD_F,1,1);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 3;
							nums._dec = 1;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "uA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_F,1,1,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_FAC_DC_CUR_2:
						{
							dyj_lc_relay(MD_F,1,2);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_F,1,2,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_FAC_DC_CUR_3:
						{
							dyj_lc_relay(MD_F,1,3);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 2;
							nums._dec = 2;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_F,1,3,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_FPEAK_CUR_1:
						{
							dyj_lc_relay(MD_F,2,1);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 3;
							nums._dec = 1;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "uA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_F,2,1,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_FPEAK_CUR_2:
						{
							dyj_lc_relay(MD_F,2,2);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_F,2,2,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_FPEAK_CUR_3:
						{
							dyj_lc_relay(MD_F,2,3);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
                            Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 2;
							nums._dec = 2;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_F,2,3,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_FDC_CUR_1:
						{
							dyj_lc_relay(MD_F,3,1);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Relay_Control(LC_OUT6,1,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 3;
							nums._dec = 1;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "uA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_F,3,1,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_FDC_CUR_2:
						{
							dyj_lc_relay(MD_F,3,2);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Relay_Control(LC_OUT6,1,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_F,3,2,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_FDC_CUR_3:
						{
							dyj_lc_relay(MD_F,3,3);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Relay_Control(LC_OUT6,1,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 2;
							nums._dec = 2;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_F,3,3,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						// MD-G
						case CAL_LC_GAC_CUR_1: // 300uA
						{
							dyj_lc_relay(MD_G,0,1);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 3;
							nums._dec = 1;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "uA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_G,0,1,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_GAC_CUR_2:
						{
							dyj_lc_relay(MD_G,0,2);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_G,0,2,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_GAC_CUR_3:
						{
							dyj_lc_relay(MD_G,0,3);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 2;
							nums._dec = 2;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_G,0,3,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_GAC_DC_CUR_1:
						{
							dyj_lc_relay(MD_G,1,1);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 3;
							nums._dec = 1;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "uA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_G,1,1,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_GAC_DC_CUR_2:
						{
							dyj_lc_relay(MD_G,1,2);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_G,1,2,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_GAC_DC_CUR_3:
						{
							dyj_lc_relay(MD_G,1,3);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 2;
							nums._dec = 2;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_G,1,3,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_GPEAK_CUR_1:
						{
							dyj_lc_relay(MD_G,2,1);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 3;
							nums._dec = 1;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "uA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_G,2,1,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_GPEAK_CUR_2:
						{
							dyj_lc_relay(MD_G,2,2);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_G,2,2,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_GPEAK_CUR_3:
						{
							dyj_lc_relay(MD_G,2,3);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 2;
							nums._dec = 2;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_G,2,3,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_GDC_CUR_1:
						{
							dyj_lc_relay(MD_G,3,1);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Relay_Control(LC_OUT6,1,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 3;
							nums._dec = 1;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "uA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_G,3,1,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_GDC_CUR_2:
						{
							dyj_lc_relay(MD_G,3,2);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Relay_Control(LC_OUT6,1,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
						  Relay_ON(EXT_DRIVER_O8);
							nums.num  = 3000;
							nums._int = 1;
							nums._dec = 3;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_G,3,2,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);	
							LC_Main_Output_Disable();
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_GDC_CUR_3:
						{
							dyj_lc_relay(MD_G,3,3);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Relay_Control(LC_OUT6,1,1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
							Relay_ON(EXT_DRIVER_O8);	
							nums.num  = 3000;
							nums._int = 2;
							nums._dec = 2;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "mA";
							rects.x = DISPLAY_CAL_CUR_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_cur_ad(MD_G,3,3,temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
						case CAL_LC_SELF_VOL:
						{
							dyj_lc_relay(MD_G,3,3);
							CPLD_GPIO_Control(OUT_C,1);
							LC_Relay_Control(LC_OUT6,1,1);
							LC_4051_D15_SELECT(1);
							LC_Main_Output_Enable();
							CAL_LC_Main_Voltage_Set(DA_LC_CURCAL_VOL,60);
							Relay_ON(EXT_DRIVER_O6);
							Relay_ON(EXT_DRIVER_O8);	
						
							nums.num  = 500;
							nums._int = 2;
							nums._dec = 2;
							nums.min  = 0;
							nums.max  = 3500;
							nums.unit = "V";
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							temp = num_input(&fonts,&rects,&nums);
							if(temp != 0xffffffff)dyj_save_lc_selv_ad(temp);
							CAL_LC_Main_Voltage_Set(0,60);
							LC_Main_Output_Disable();
							CPLD_GPIO_Control(OUT_C,0);
							Relay_OFF(RET_GND_SELECT);
							Relay_OFF(EXT_DRIVER_O6);
							Relay_OFF(EXT_DRIVER_O8);
							if(temp == 0xffffffff)goto __cal_display;
							
							break;
						}
							
							
						//IR校准
						case CAL_IR_RES_1:// 2MΩ
						{
							Sampling_Relay_State_CHange(DC_2mA);
							DC_Output_Enable();
							DC_SetVoltage(0);
							Relay_ON(ACW_DCW_IR);
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							{
								uint16_t 	cal_point_index = 1;
								uint32_t  Voltage_value[10];
								uint16_t  AD_value[10];
								char buf[20] = "";
								uint32_t counter=0;
								
								DC_SetVoltage(100*cal_point_index);
								
								while(1)
								{
									
									if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
									{
										switch(msg)
										{
											/* 返回 */
											case KEY_F6   | KEY_UP:
											case KEY_EXIT | KEY_UP:
												DC_SetVoltage(0);
												DC_Output_Disable();
												Relay_OFF(ACW_DCW_IR);	
												goto __cal_display;
						
											
											
											default:
												break;
										}
									}else
									{
										counter++;
										if(counter % 500 == 0)
										{
											cal_point_index++;
											if(cal_point_index>10)
											{
												dyj_save_IR_ad(Voltage_value,AD_value,1);
												break;
											}
											DC_SetVoltage(100*cal_point_index);
										}
										else if(counter % 10 == 0)
										{
											Voltage_value[cal_point_index-1] = DC_GetVoltage();
											rt_sprintf(buf,"%d.%03dkV", Voltage_value[cal_point_index-1]/1000,Voltage_value[cal_point_index-1]%1000);
											rects.x = DISPLAY_CAL_VOL_X;
											ui_text_draw(&fonts,&rects,buf);
											AD_value[cal_point_index-1] = Read_AD_Value(W_I_AD_IN);
											rt_sprintf(buf,"%d", AD_value[cal_point_index-1]);
											rects.x = DISPLAY_CAL_CUR_X;
											ui_text_draw(&fonts,&rects,buf);
										}
										
										
									}
									
								}
							}
							DC_SetVoltage(0);
							DC_Output_Disable();
							Relay_OFF(ACW_DCW_IR);
			
							break;
						}
						case CAL_IR_RES_2:// 20MΩ
						{
							Sampling_Relay_State_CHange(DC_200uA);
							DC_Output_Enable();
							DC_SetVoltage(0);
							Relay_ON(ACW_DCW_IR);
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							{
								uint16_t 	cal_point_index = 1;
								uint32_t  Voltage_value[10];
								uint16_t  AD_value[10];
								char buf[20] = "";
								uint32_t counter=0;
								
								DC_SetVoltage(100*cal_point_index);
								
								while(1)
								{
									
									if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
									{
										switch(msg)
										{
											/* 返回 */
											case KEY_F6   | KEY_UP:
											case KEY_EXIT | KEY_UP:
												DC_SetVoltage(0);
												DC_Output_Disable();
												Relay_OFF(ACW_DCW_IR);	
												goto __cal_display;
						
											
											
											default:
												break;
										}
									}else
									{
										counter++;
										if(counter % 500 == 0)
										{
											cal_point_index++;
											if(cal_point_index>10)
											{
												dyj_save_IR_ad(Voltage_value,AD_value,2);
												break;
											}
											DC_SetVoltage(100*cal_point_index);
										}
										else if(counter % 10 == 0)
										{
											Voltage_value[cal_point_index-1] = DC_GetVoltage();
											rt_sprintf(buf,"%d.%03dkV", Voltage_value[cal_point_index-1]/1000,Voltage_value[cal_point_index-1]%1000);
											rects.x = DISPLAY_CAL_VOL_X;
											ui_text_draw(&fonts,&rects,buf);
											AD_value[cal_point_index-1] = Read_AD_Value(W_I_AD_IN);
											rt_sprintf(buf,"%d", AD_value[cal_point_index-1]);
											rects.x = DISPLAY_CAL_CUR_X;
											ui_text_draw(&fonts,&rects,buf);
										}
										
										
									}
									
								}
							}
							DC_SetVoltage(0);
							DC_Output_Disable();
							Relay_OFF(ACW_DCW_IR);
			
							break;
						}
						case CAL_IR_RES_3:// 200MΩ
						{
							Sampling_Relay_State_CHange(DC_20uA);
							DC_Output_Enable();
							DC_SetVoltage(0);
							Relay_ON(ACW_DCW_IR);
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							{
								uint16_t 	cal_point_index = 1;
								uint32_t  Voltage_value[10];
								uint16_t  AD_value[10];
								char buf[20] = "";
								uint32_t counter=0;
								
								DC_SetVoltage(100*cal_point_index);
								
								while(1)
								{
									
									if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
									{
										switch(msg)
										{
											/* 返回 */
											case KEY_F6   | KEY_UP:
											case KEY_EXIT | KEY_UP:
												DC_SetVoltage(0);
												DC_Output_Disable();
												Relay_OFF(ACW_DCW_IR);	
												goto __cal_display;
						
											
											
											default:
												break;
										}
									}else
									{
										counter++;
										if(counter % 500 == 0)
										{
											cal_point_index++;
											if(cal_point_index>10)
											{
												dyj_save_IR_ad(Voltage_value,AD_value,3);
												break;
											}
											DC_SetVoltage(100*cal_point_index);
										}
										else if(counter % 10 == 0)
										{
											Voltage_value[cal_point_index-1] = DC_GetVoltage();
											rt_sprintf(buf,"%d.%03dkV", Voltage_value[cal_point_index-1]/1000,Voltage_value[cal_point_index-1]%1000);
											rects.x = DISPLAY_CAL_VOL_X;
											ui_text_draw(&fonts,&rects,buf);
											AD_value[cal_point_index-1] = Read_AD_Value(W_I_AD_IN);
											rt_sprintf(buf,"%d", AD_value[cal_point_index-1]);
											rects.x = DISPLAY_CAL_CUR_X;
											ui_text_draw(&fonts,&rects,buf);
										}
										
										
									}
									
								}
							}
							DC_SetVoltage(0);
							DC_Output_Disable();
							Relay_OFF(ACW_DCW_IR);
			
							break;
						}
						case CAL_IR_RES_4:// 2GΩ
						{
							Sampling_Relay_State_CHange(DC_2uA);
							DC_Output_Enable();
							DC_SetVoltage(0);
							Relay_ON(ACW_DCW_IR);
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							{
								uint16_t 	cal_point_index = 1;
								uint32_t  Voltage_value[10];
								uint16_t  AD_value[10];
								char buf[20] = "";
								uint32_t counter=0;
								
								DC_SetVoltage(100*cal_point_index);
								
								while(1)
								{
									
									if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
									{
										switch(msg)
										{
											/* 返回 */
											case KEY_F6   | KEY_UP:
											case KEY_EXIT | KEY_UP:
												DC_SetVoltage(0);
												DC_Output_Disable();
												Relay_OFF(ACW_DCW_IR);	
												goto __cal_display;
						
											
											
											default:
												break;
										}
									}else
									{
										counter++;
										if(counter % 500 == 0)
										{
											cal_point_index++;
											if(cal_point_index>10)
											{
												dyj_save_IR_ad(Voltage_value,AD_value,4);
												break;
											}
											DC_SetVoltage(100*cal_point_index);
										}
										else if(counter % 10 == 0)
										{
											Voltage_value[cal_point_index-1] = DC_GetVoltage();
											rt_sprintf(buf,"%d.%03dkV", Voltage_value[cal_point_index-1]/1000,Voltage_value[cal_point_index-1]%1000);
											rects.x = DISPLAY_CAL_VOL_X;
											ui_text_draw(&fonts,&rects,buf);
											AD_value[cal_point_index-1] = Read_AD_Value(W_I_AD_IN);
											rt_sprintf(buf,"%d", AD_value[cal_point_index-1]);
											rects.x = DISPLAY_CAL_CUR_X;
											ui_text_draw(&fonts,&rects,buf);
										}
										
										
									}
									
								}
							}
							DC_SetVoltage(0);
							DC_Output_Disable();
							Relay_OFF(ACW_DCW_IR);
			
							break;
						}
						case CAL_IR_RES_5:// 10Ω
						{
							Sampling_Relay_State_CHange(DC_2uA);
							DC_Output_Enable();
							DC_SetVoltage(0);
							Relay_ON(ACW_DCW_IR);
							rects.x = DISPLAY_CAL_VOL_X;
							rects.y = DISPLAY_CAL_Y+26*(cal_list_box.current_item%13);
							rects.h = DISPLAY_CAL_H;
							rects.w = DISPLAY_CAL_W;
							{
								uint16_t 	cal_point_index = 1;
								uint32_t  Voltage_value[10];
								uint16_t  AD_value[10];
								char buf[20] = "";
								uint32_t counter=0;
								
								DC_SetVoltage(100*cal_point_index);
								
								while(1)
								{
									
									if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
									{
										switch(msg)
										{
											/* 返回 */
											case KEY_F6   | KEY_UP:
											case KEY_EXIT | KEY_UP:
												DC_SetVoltage(0);
												DC_Output_Disable();
												Relay_OFF(ACW_DCW_IR);	
												goto __cal_display;
						
											
											
											default:
												break;
										}
									}else
									{
										counter++;
										if(counter % 500 == 0)
										{
											cal_point_index++;
											if(cal_point_index>10)
											{
												dyj_save_IR_ad(Voltage_value,AD_value,5);
												break;
											}
											DC_SetVoltage(100*cal_point_index);
										}
										else if(counter % 10 == 0)
										{
											Voltage_value[cal_point_index-1] = DC_GetVoltage();
											rt_sprintf(buf,"%d.%03dkV", Voltage_value[cal_point_index-1]/1000,Voltage_value[cal_point_index-1]%1000);
											rects.x = DISPLAY_CAL_VOL_X;
											ui_text_draw(&fonts,&rects,buf);
											AD_value[cal_point_index-1] = Read_AD_Value(W_I_AD_IN);
											rt_sprintf(buf,"%d", AD_value[cal_point_index-1]);
											rects.x = DISPLAY_CAL_CUR_X;
											ui_text_draw(&fonts,&rects,buf);
										}
										
										
									}
									
								}
							}
							DC_SetVoltage(0);
							DC_Output_Disable();
							Relay_OFF(ACW_DCW_IR);
			
							break;
						}
						default:
							
						break;
					}
					
					if(cal_list_box.current_item < cal_list_box.total_items - 1)
					{
						cal_list_box.current_item ++;
					}
					
					cal_list_box.start_item = (cal_list_box.current_item / cal_list_box.items_count)*cal_list_box.items_count;
					listbox_draw(&cal_list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
				}
			}
		}
	}
}
static void cal_item_draw(struct font_info_t *font,rt_uint16_t index,rt_uint16_t x,rt_uint16_t y)
{
	font_draw(x+8,y,font,cal_item_name[index]);
}

#define   LC_K1         I_200uA_2mA_20mA
#define   LC_K2         GB9706_1
#define   LC_K3         LC_NY
#define   LC_K4         I_200uA_2mA_20mA
#define   LC_K5         ULN544NP
#define   LC_K6         ULN544P
#define   LC_K7         I_200uA_2mA_20mA
#define   LC_K11        GB4943_GBT12113
#define   LC_K12        FIG4_FIG5
#define   LC_K13        FIG3_FIG4
#define   LC_K14        I_200uA_2mA_20mA
#define   LC_K15        GBT12133
#define   LC_K16        I_200uA_2mA_20mA
#define   LC_K17        UL1563
#define   LC_K20        I_200uA_2mA_20mA
#define   LC_K25        I_FILES
#define   LC_K26        I_AC_DC_AC
#define   LC_K27        SELV_AC_DC_AC
#define   LC_K33        MD_HI_GND
#define   LC_K34        G_Change
#define   LC_K35        S3_CONTROL
//static void change_curdetection(uint8_t curdetection)
//{
//	switch(curdetection){
//		case 0:    // AC
//			LC_Relay_Control(LC_K26,0,1);
//			LC_4051_D1_SELECT(1);
//		
//			LC_Relay_Control(LC_K27,0,1);
//			LC_4051_D15_SELECT(1);	
//		break;
//		case 1:    // AC+DC
//			LC_Relay_Control(LC_K26,1,1);
//			LC_4051_D1_SELECT(1);
//		
//			LC_Relay_Control(LC_K27,1,1);
//			LC_4051_D15_SELECT(1);
//		break;
//		case 2:    // PEAK
//			LC_4051_D1_SELECT(0);
//			LC_4051_D15_SELECT(0);
////			LC_Relay_Control(PEAK_DISCHARGE,1,0);
//			LC_Relay_Control(SELV_PEAK_DISCHARGE,1,1);
//		break;
//		case 3:    // DC
//			LC_4051_D1_SELECT(2);
//			LC_4051_D15_SELECT(2);
//		break;

//		default:
//			
//		break;
//	}
//}








static void system_item_draw(struct font_info_t *font,rt_uint16_t index,rt_uint16_t x,rt_uint16_t y)
{
	font_draw(x+(300-rt_strlen(items[index].name[language])*8)/2,y,font,items[index].name[language]);
}

static u8 system_time_setting(void *arg)
{
	rt_uint32_t msg;
	struct panel_type *win;
	struct rect_type rect={190,140,200,300};
	struct font_info_t font={0,0,0XE73C,1,0,16};
	rt_device_t device;
	struct rtc_time_type	time;
	char buf[20];	
	rt_uint16_t postion[6][2];
	u8 pos = 0,num_input_n=0,error=0;
	struct rect_type rect_e={80,110,16,80};

	win = sui_window_create(T_STR("时间参数设置","Time parameter set"),&rect);
	font.panel = win;

	device = rt_device_find("rtc");
	if(device != RT_NULL)
	{
		rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);						
		rt_sprintf(buf,T_STR("日期：%04d-%02d-%02d","Date: %04d-%02d-%02d"),time.year+2000,time.month,time.date);
		font_draw(70,60,&font,buf);
		rt_sprintf(buf,T_STR("时间：%02d:%02d:%02d","Time: %02d:%02d:%02d"),time.hours,time.minutes,time.seconds);
		font_draw(70,85,&font,buf);
		font.fontcolor = 0XFC00;
					font_draw(20,130,&font,T_STR("提示：左右键移动光标；","Tips:The or so key move cursor;"));
		font_draw(20+32,130+20,&font,T_STR("按上下键或数字键输入；","Press keys or number key input;"));
		font_draw(20+32,130+40,&font,T_STR("\"ENTER\"键保存；\"EXIT\"键退出。","\"ENTER\"Key Save;\"EXIT\"Key Quit"));
		
	}
	else
	{
		font.fontcolor = 0xf800;
		font_draw(70,60,&font,T_STR("时间不可用！","Time not use!"));
		font_draw(70,85,&font,T_STR("按任意键退出。","Press any key to exit."));
		while(1)
		{
			if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
			{
				return 1;
			}
		}
	}
	
	postion[0][0] = 70 + 8*8;postion[0][1] = 60;
	postion[1][0] = 70 + 8*11;postion[1][1] = 60;
	postion[2][0] = 70 + 8*14;postion[2][1] = 60;
	postion[3][0] = 70 + 8*6;postion[3][1] = 85;
	postion[4][0] = 70 + 8*9;postion[4][1] = 85;
	postion[5][0] = 70 + 8*12;postion[5][1] = 85;
	rect.x = postion[pos][0];
	rect.y = postion[pos][1];
	rect.h = 16;
	rect.w = 16;
	sui_window_xor(win,&rect);
	sui_window_update(win);
	
	
	
// 	window_updata(win,&rect);
	
	font.fontcolor = 0;
	font.backcolor = 0XE73C;
	font.alpha	   = 0;
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			error = 0;
			switch(msg)
			{
				case KEY_L | KEY_DOWN:
					sui_window_xor(win,&rect);
					window_updata(win,&rect);
					if(pos>0)pos--;
					else pos = 5;
					rect.x = postion[pos][0];
					rect.y = postion[pos][1];
					sui_window_xor(win,&rect);
					window_updata(win,&rect);
					num_input_n=0;
					break;
				case KEY_R | KEY_DOWN:
					sui_window_xor(win,&rect);
					window_updata(win,&rect);
					if(pos<5)pos++;
					else pos = 0;
					rect.x = postion[pos][0];
					rect.y = postion[pos][1];
					sui_window_xor(win,&rect);
					window_updata(win,&rect);
					num_input_n=0;
					break;
				case CODE_RIGHT:
				case KEY_U | KEY_DOWN:
					switch(pos)
					{
						case 0:
							if(time.year<99)time.year++;
							else time.year = 0;
							if(time.month==2)
							{
								if(time.year%4==0 && time.date>29)
								{
									time.date=29;
									rt_sprintf(buf,"%02d",time.date);
									font_draw(postion[2][0],postion[2][1],&font,buf);
									rect.x = postion[2][0];
									rect.y = postion[2][1];
									window_updata(win,&rect);
									rect.x = postion[pos][0];
									rect.y = postion[pos][1];
								}
								else if(time.date>28)
								{
									time.date=28;
									rt_sprintf(buf,"%02d",time.date);
									font_draw(postion[2][0],postion[2][1],&font,buf);
									rect.x = postion[2][0];
									rect.y = postion[2][1];
									window_updata(win,&rect);
									rect.x = postion[pos][0];
									rect.y = postion[pos][1];
								}							
							}
							rt_sprintf(buf,"%02d",time.year);			
							break;
						case 1:
							if(time.month<12)time.month++;
							else time.month = 1;
							switch(time.month)
							{
								case 4:
								case 6:
								case 9:
								case 11:
									if(time.date>30)
									{
										time.date=30;
										rt_sprintf(buf,"%02d",time.date);
										font_draw(postion[pos+1][0],postion[pos+1][1],&font,buf);
										rect.x = postion[pos+1][0];
										rect.y = postion[pos+1][1];
										window_updata(win,&rect);
										rect.x = postion[pos][0];
										rect.y = postion[pos][1];
									}
									break;
								case 2:
									if(time.year%4==0)
									{
										if(time.date>29)time.date=29;
									}
									else
									{
										if(time.date>28)time.date=28;
									}
									rt_sprintf(buf,"%02d",time.date);
									font_draw(postion[pos+1][0],postion[pos+1][1],&font,buf);
									rect.x = postion[pos+1][0];
									rect.y = postion[pos+1][1];
									window_updata(win,&rect);
									rect.x = postion[pos][0];
									rect.y = postion[pos][1];
									break;
							}
							rt_sprintf(buf,"%02d",time.month);
							break;
						case 2:
							switch(time.month)
							{
								case 1:
								case 3:
								case 5:
								case 7:
								case 8:
								case 10:
								case 12:
									if(time.date<31)time.date++;
									else time.date = 1;
									break;
								case 2:
									if(time.year%4==0)
									{
										if(time.date<29)time.date++;
										else time.date = 1;
									}
									else
									{
										if(time.date<28)time.date++;
										else time.date = 1;
									}
									break;
								default:
									if(time.date<30)time.date++;
									else time.date = 1;
									break;
							}
							rt_sprintf(buf,"%02d",time.date);
							break;
						case 3:
							if(time.hours<23)time.hours++;
							else time.hours = 0;
							rt_sprintf(buf,"%02d",time.hours);
							break;
						case 4:
							if(time.minutes<59)time.minutes++;
							else time.minutes = 0;
							rt_sprintf(buf,"%02d",time.minutes);
							break;
						case 5:
							if(time.seconds<59)time.seconds++;
							else time.seconds = 0;
							rt_sprintf(buf,"%02d",time.seconds);
							break;
					}
					font_draw(postion[pos][0],postion[pos][1],&font,buf);
					sui_window_xor(win,&rect);
					window_updata(win,&rect);
					break;
				case CODE_LEFT:
				case KEY_D | KEY_DOWN:
					switch(pos)
					{
						case 0:
							if(time.year>0)time.year--;
							else time.year = 99;
							if(time.month==2)
							{
								if(time.year%4==0 && time.date>29)
								{
									time.date=29;
									rt_sprintf(buf,"%02d",time.date);
									font_draw(postion[2][0],postion[2][1],&font,buf);
									rect.x = postion[2][0];
									rect.y = postion[2][1];
									window_updata(win,&rect);
									rect.x = postion[pos][0];
									rect.y = postion[pos][1];
								}
								else if(time.date>28)
								{
									time.date=28;
									rt_sprintf(buf,"%02d",time.date);
									font_draw(postion[2][0],postion[2][1],&font,buf);
									rect.x = postion[2][0];
									rect.y = postion[2][1];
									window_updata(win,&rect);
									rect.x = postion[pos][0];
									rect.y = postion[pos][1];
								}							
							}
							rt_sprintf(buf,"%02d",time.year);
							break;
						case 1:
							if(time.month>1)time.month--;
							else time.month = 12;
							switch(time.month)
							{
								case 4:
								case 6:
								case 9:
								case 11:
									if(time.date>30)
									{
										time.date=30;
										rt_sprintf(buf,"%02d",time.date);
										font_draw(postion[pos+1][0],postion[pos+1][1],&font,buf);
										rect.x = postion[pos+1][0];
										rect.y = postion[pos+1][1];
										window_updata(win,&rect);
										rect.x = postion[pos][0];
										rect.y = postion[pos][1];
									}
									break;
								case 2:
									if(time.year%4==0)
									{
										if(time.date>29)time.date=29;
									}
									else
									{
										if(time.date>28)time.date=28;
									}
									rt_sprintf(buf,"%02d",time.date);
									font_draw(postion[pos+1][0],postion[pos+1][1],&font,buf);
									rect.x = postion[pos+1][0];
									rect.y = postion[pos+1][1];
									window_updata(win,&rect);
									rect.x = postion[pos][0];
									rect.y = postion[pos][1];
									break;
							}
							rt_sprintf(buf,"%02d",time.month);
							break;
						case 2:
							if(time.date>1)time.date--;
							else
							{
								switch(time.month)
								{
									case 1:
									case 3:
									case 5:
									case 7:
									case 8:
									case 10:
									case 12:
										time.date = 31;
										break;
									case 2:
										if(time.year%4==0)
										{
											time.date = 29;
										}
										else
										{
											time.date = 28;
										}
										break;
									default:
										time.date = 30;
										break;
								}
							}
							rt_sprintf(buf,"%02d",time.date);
							break;
						case 3:
							if(time.hours>0)time.hours--;
							else time.hours = 23;
							rt_sprintf(buf,"%02d",time.hours);
							break;
						case 4:
							if(time.minutes>0)time.minutes--;
							else time.minutes = 59;
							rt_sprintf(buf,"%02d",time.minutes);
							break;
						case 5:
							if(time.seconds>0)time.seconds--;
							else time.seconds = 59;
							rt_sprintf(buf,"%02d",time.seconds);
							break;
					}
					font_draw(postion[pos][0],postion[pos][1],&font,buf);
					sui_window_xor(win,&rect);
					window_updata(win,&rect);
					break;
				case KEY_ENTER | KEY_UP:
					rt_device_control(device, RT_DEVICE_CTRL_RTC_SET_TIME, &time);
					return 0;
				case KEY_F6 | KEY_UP:
				case KEY_EXIT | KEY_DOWN:
					return 1;	
				default:
					if(msg & KEY_DOWN)
					{
						u8 code = msg &0x1f;
						if((code >= KEY_NUM0) && (code <= KEY_NUM9))
						{
							code -=   KEY_NUM0;
							switch(pos)
							{
								case 0:
									if(num_input_n==0)
									{
										num_input_n++;
										time.year = code*10 + time.year%10;
										rt_sprintf(buf,"%02d",time.year);
										font_draw(postion[pos][0],postion[pos][1],&font,buf);
										sui_window_xor(win,&rect);
										window_updata(win,&rect);
									}
									else if(num_input_n==1)
									{
										num_input_n=0;
										time.year = time.year/10*10 + code;
										rt_sprintf(buf,"%02d",time.year);
										font_draw(postion[pos][0],postion[pos][1],&font,buf);
										window_updata(win,&rect);
										if(time.month==2)
										{
											if(time.year%4==0 && time.date>29)
											{
												time.date=29;
												rt_sprintf(buf,"%02d",time.date);
												font_draw(postion[2][0],postion[2][1],&font,buf);
												rect.x = postion[2][0];
												rect.y = postion[2][1];
												window_updata(win,&rect);
											}
											else if(time.date>28)
											{
												time.date=28;
												rt_sprintf(buf,"%02d",time.date);
												font_draw(postion[2][0],postion[2][1],&font,buf);
												rect.x = postion[2][0];
												rect.y = postion[2][1];
												window_updata(win,&rect);
											}							
										}
										pos++;
										rect.x = postion[pos][0];
										rect.y = postion[pos][1];
										sui_window_xor(win,&rect);
										window_updata(win,&rect);
									}
								break;
							case 1:
								if(num_input_n==0 && code<2)
								{
									num_input_n++;
									time.month = code*10 + time.month%10;
									if(time.month>12)time.month=12;
									rt_sprintf(buf,"%02d",time.month);
									font_draw(postion[pos][0],postion[pos][1],&font,buf);
									sui_window_xor(win,&rect);
									window_updata(win,&rect);
								}
								else if(num_input_n==1)
								{
									if((time.month /10 ==0) || (code <3))
									{
										num_input_n=0;
										time.month = time.month/10*10 + code;
										rt_sprintf(buf,"%02d",time.month);
										font_draw(postion[pos][0],postion[pos][1],&font,buf);
										window_updata(win,&rect);
										switch(time.month)
										{
											case 4:
											case 6:
											case 9:
											case 11:
												if(time.date>30)
												{
													time.date=30;
													rt_sprintf(buf,"%02d",time.date);
													font_draw(postion[2][0],postion[2][1],&font,buf);
													rect.x = postion[2][0];
													rect.y = postion[2][1];
													window_updata(win,&rect);
												}
												break;
											case 2:
												if(time.year%4==0)
												{
													if(time.date>29)time.date=29;
												}
												else
												{
													if(time.date>28)time.date=28;
												}
												rt_sprintf(buf,"%02d",time.date);
												font_draw(postion[2][0],postion[2][1],&font,buf);
												rect.x = postion[2][0];
												rect.y = postion[2][1];
												window_updata(win,&rect);
												break;
										}
										pos++;
										rect.x = postion[pos][0];
										rect.y = postion[pos][1];
										sui_window_xor(win,&rect);
										window_updata(win,&rect);
									}
									else error = 1;
								}
								else error = 1;
								break;
							case 2:
								if(num_input_n==0 && ((code<3)||(time.month!=2 && code<4)))
								{
									num_input_n++;
									time.date = code*10 + time.date%10;
									switch(time.month)
									{
										case 1:
										case 3:
										case 5:
										case 7:
										case 8:
										case 10:
										case 12:
											if(time.date>31)time.date=31;
											break;
										case 2:
											if(time.year%4==0)
											{
												if(time.date>29)time.date=29;
											}
											else
											{
												if(time.date>28)time.date=28;
											}
											break;
										default:
											if(time.date>30)time.date=30;
											break;
									}
									rt_sprintf(buf,"%02d",time.date);
									font_draw(postion[pos][0],postion[pos][1],&font,buf);
									sui_window_xor(win,&rect);
									window_updata(win,&rect);
								}
								else if(num_input_n==1)
								{
									if((time.date/10 < 2) || 
									   (code<1) || 
									   (time.date/10==2 && 
										((time.month !=2) || 
										 (time.month ==2  && (code<9 || time.year%4==0)))) ||
									   (time.date/10==3 && (time.month ==1 || time.month ==3 || time.month ==5 || time.month ==7 || time.month ==8 || time.month ==10 || time.month ==12) && code<2)
									  )
									{
										num_input_n=0;
										time.date = time.date/10*10 + code;
										rt_sprintf(buf,"%02d",time.date);
										font_draw(postion[pos][0],postion[pos][1],&font,buf);
										window_updata(win,&rect);
										pos++;
										rect.x = postion[pos][0];
										rect.y = postion[pos][1];
										sui_window_xor(win,&rect);
										window_updata(win,&rect);
									}
									else error = 1;
								}
								else error = 1;
								break;
							case 3:
								if(num_input_n==0 && code<6)
								{
									num_input_n++;
									time.hours = code*10 + time.hours%10;
									rt_sprintf(buf,"%02d",time.hours);
									font_draw(postion[pos][0],postion[pos][1],&font,buf);
									sui_window_xor(win,&rect);
									window_updata(win,&rect);
								}
								else if(num_input_n==1)
								{
									num_input_n=0;
									time.hours = time.hours/10*10 + code;
									rt_sprintf(buf,"%02d",time.hours);
									font_draw(postion[pos][0],postion[pos][1],&font,buf);
									window_updata(win,&rect);
									pos++;
									rect.x = postion[pos][0];
									rect.y = postion[pos][1];
									sui_window_xor(win,&rect);
									window_updata(win,&rect);
								}
								else error = 1;
								break;
							case 4:
								if(num_input_n==0 && code<6)
								{
									num_input_n++;
									time.minutes = code*10 + time.minutes%10;
									rt_sprintf(buf,"%02d",time.minutes);
									font_draw(postion[pos][0],postion[pos][1],&font,buf);
									sui_window_xor(win,&rect);
									window_updata(win,&rect);
								}
								else if(num_input_n==1)
								{
									num_input_n=0;
									time.minutes = time.minutes/10*10 + code;
									rt_sprintf(buf,"%02d",time.minutes);
									font_draw(postion[pos][0],postion[pos][1],&font,buf);
									window_updata(win,&rect);
									pos++;
									rect.x = postion[pos][0];
									rect.y = postion[pos][1];
									sui_window_xor(win,&rect);
									window_updata(win,&rect);
								}
								else error = 1;
								break;
							case 5:
								if(num_input_n==0 && code<6)
								{
									num_input_n++;
									time.seconds = code*10 + time.seconds%10;
									rt_sprintf(buf,"%02d",time.seconds);
									font_draw(postion[pos][0],postion[pos][1],&font,buf);
									sui_window_xor(win,&rect);
									window_updata(win,&rect);
								}
								else if(num_input_n==1)
								{
									num_input_n=0;
									time.seconds = time.seconds/10*10 + code;
									rt_sprintf(buf,"%02d",time.seconds);
									font_draw(postion[pos][0],postion[pos][1],&font,buf);
									window_updata(win,&rect);
									pos=0;
									rect.x = postion[pos][0];
									rect.y = postion[pos][1];
									sui_window_xor(win,&rect);
									window_updata(win,&rect);
								}
								else error = 1;
								break;
							}
						}
					}
					break;
			}
			
			if(error == 0)
			{
				font_draw(80,110,&font,"          ");
			}
			else
			{
				font.fontcolor = 0xf800;
				font_draw(80,110,&font,T_STR("输入非法！","Input illegality !"));
				font.fontcolor = 0;
			}
			window_updata(win,&rect_e);
		}
	}
}

#define 	_XOFFSET	 20
#define		_YOFFSET	 30
#define 	_X2OFFSET	 150
#define		_XSIZEOF	 100

static void _system1(struct font_info_t *font,u8 pos)
{
	clr_win(font->panel,0XE73C,_X2OFFSET,_YOFFSET,150,_XSIZEOF);
	clr_win(font->panel,0x53fa,_X2OFFSET,_YOFFSET+8+30*pos,20,_XSIZEOF);
	
	font_draw(_X2OFFSET + (_XSIZEOF-32)/2,_YOFFSET+10 ,font,(_system_password.systempasswordm_en)==0?boolean_name[language][0]:boolean_name[language][1]);
	font_draw(_X2OFFSET + (_XSIZEOF-48)/2,_YOFFSET+40 ,font,"******");
	font_draw(_X2OFFSET + (_XSIZEOF-32)/2,_YOFFSET+70 ,font,(_system_password.keylockmem_en)==0?boolean_name[language][0]:boolean_name[language][1]);
	font_draw(_X2OFFSET + (_XSIZEOF-32)/2,_YOFFSET+100,font,(_system_password.keylockpasswordm_en)==0?boolean_name[language][0]:boolean_name[language][1]);
	font_draw(_X2OFFSET + (_XSIZEOF-48)/2,_YOFFSET+130,font,"******");
}

static u8 system_password_setting(void *arg)
{
	rt_uint32_t msg;
	rt_uint8_t	pos=0;
	struct panel_type *win;
	struct rect_type rect={190,140,200,300};
	struct font_info_t font={0,0X4208,0XE73C,1,0,16};
	
	win = sui_window_create(T_STR("密码参数","Password parameter"),&rect);
	font.panel = win;
	
	/* 开始窗口界面绘制 */
__loop1:
	font_draw(_XOFFSET,_YOFFSET+10 ,&font,T_STR("  系统密码启用:","Sys pwd. use   :"));
	font_draw(_XOFFSET,_YOFFSET+40 ,&font,T_STR("  系统密码更改:","Sys pwd. change: "));
	font_draw(_XOFFSET,_YOFFSET+70 ,&font,T_STR("键盘锁关机存储:","Shutdown save  :"));
	font_draw(_XOFFSET,_YOFFSET+100,&font,T_STR("键盘锁密码启用:","KeyLock pwd.use:"));
	font_draw(_XOFFSET,_YOFFSET+130,&font,T_STR("键盘锁密码更改:","Change pwd.    :"));
	
	_system1(&font,pos);
	sui_window_update(win);
	
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			switch(msg)
			{
				case KEY_L | KEY_DOWN:
				case KEY_R | KEY_DOWN:
					switch(pos)
					{
						case 0:
							if(_system_password.systempasswordm_en!=0)_system_password.systempasswordm_en=0;
							else _system_password.systempasswordm_en = 1;
							break;
						case 1:
							break;
						case 2:
							if(_system_password.keylockmem_en!=0)_system_password.keylockmem_en=0;
							else _system_password.keylockmem_en = 1;
							break;
						case 3:
							if(_system_password.keylockpasswordm_en!=0)_system_password.keylockpasswordm_en=0;
							else _system_password.keylockpasswordm_en = 1;
							break;
						case 4:
							break;
					}
					_system1(&font,pos);
					sui_window_update(win);
					memory_systems_save();
					break;
				case KEY_ENTER | KEY_UP:
				{
					struct num_format num;
					rt_uint32_t		temp,temp1;
					
					struct rect_type rect1={_X2OFFSET,_YOFFSET+8,20,_XSIZEOF};
					
					font.center = 1;
					switch(pos)
					{
						case 0:
							break;
						case 1:
							clr_win(win,0XE73C,0,_YOFFSET,170,300);
							clr_win(win,0x53fa,_X2OFFSET,_YOFFSET+8  ,20,_XSIZEOF);
							clr_win(win,0x53fa,_X2OFFSET,_YOFFSET+38 ,20,_XSIZEOF);
							clr_win(win,0x53fa,_X2OFFSET,_YOFFSET+68 ,20,_XSIZEOF);
							font_draw(_XOFFSET,_YOFFSET+10 ,&font,T_STR("请输入原始密码:","Input old pwd:"));
							font_draw(_XOFFSET,_YOFFSET+40 ,&font,T_STR("请输入更改密码:","Input new pwd:"));
							font_draw(_XOFFSET,_YOFFSET+70 ,&font,T_STR("请确认更改密码:","Input again  :"));
							sui_window_update(win);
							
							font.fontcolor = 0Xf000;
							font.backcolor = 0x53fa;
						
							num.num  = 0;
							num._int = 6;
							num._dec = 0;
							num.min  = 0;
							num.max  = 99999999;
							num.unit = "";
							
							temp = num_input(&font,&rect1,&num);
							if(temp == 0xffffffff)
							{
								clr_win(win,0XE73C,0,_YOFFSET,170,300);
								font.fontcolor = 0X4208;
								goto __loop1;
							}
							else if(temp == _system_password.systempasswordm)
							{
								rect1.y = _YOFFSET+38;
								temp = num_input(&font,&rect1,&num);
								if(temp == 0xffffffff)
								{
									clr_win(win,0XE73C,0,_YOFFSET,170,300);
									font.fontcolor = 0X4208;
									goto __loop1;
								}
								rect1.y = _YOFFSET+68;
								temp1 = num_input(&font,&rect1,&num);
								if(temp1 == 0xffffffff)
								{
									clr_win(win,0XE73C,0,_YOFFSET,170,300);
									font.fontcolor = 0X4208;
									goto __loop1;
								}
								if(temp == temp1)// 保存更改后的密码
								{
									_system_password.systempasswordm = temp;
									memory_systems_save();
								}
							}
							else
							{
								font_draw(_XOFFSET,_YOFFSET+110 ,&font,T_STR("密码错误！","password Error "));
								sui_window_update(win);
								rt_thread_delay( RT_TICK_PER_SECOND );
							}
							clr_win(win,0XE73C,0,_YOFFSET,170,300);
							font.fontcolor = 0X4208;
							goto __loop1;
						case 2:
							
							break;
						case 3:
							
							break;
						case 4:
							clr_win(win,0XE73C,0,_YOFFSET,170,300);
							clr_win(win,0x53fa,_X2OFFSET,_YOFFSET+8  ,20,_XSIZEOF);
							clr_win(win,0x53fa,_X2OFFSET,_YOFFSET+38 ,20,_XSIZEOF);
							clr_win(win,0x53fa,_X2OFFSET,_YOFFSET+68 ,20,_XSIZEOF);
							font_draw(_XOFFSET,_YOFFSET+10 ,&font,T_STR("请输入原始密码:","Input old pwd:"));
							font_draw(_XOFFSET,_YOFFSET+40 ,&font,T_STR("请输入更改密码:","Input new pwd:"));
							font_draw(_XOFFSET,_YOFFSET+70 ,&font,T_STR("请确认更改密码:","Input again  :"));
							sui_window_update(win);
						
							font.fontcolor = 0Xf000;
							font.backcolor = 0x53fa;
						
							num.num  = 0;
							num._int = 6;
							num._dec = 0;
							num.min  = 0;
							num.max  = 99999999;
							num.unit = "";
							
							temp = num_input(&font,&rect1,&num);
							if(temp == 0xffffffff)
							{
								clr_win(win,0XE73C,0,_YOFFSET,170,300);
								font.fontcolor = 0X4208;
								goto __loop1;
							}
							else if(temp == _system_password.keylockpasswordm)
							{
								rect1.y = _YOFFSET+38;
								temp = num_input(&font,&rect1,&num);
								if(temp == 0xffffffff)
								{
									clr_win(win,0XE73C,0,_YOFFSET,170,300);
									font.fontcolor = 0X4208;
									goto __loop1;
								}
								rect1.y = _YOFFSET+68;
								temp1 = num_input(&font,&rect1,&num);
								if(temp1 == 0xffffffff)
								{
									clr_win(win,0XE73C,0,_YOFFSET,170,300);
									font.fontcolor = 0X4208;
									goto __loop1;
								}
								if(temp == temp1)// 保存更改后的密码
								{
									_system_password.keylockpasswordm = temp;
									memory_systems_save();
								}
							}
							else
							{
								font_draw(_XOFFSET,_YOFFSET+110 ,&font,T_STR("密码错误！","password Error "));
								sui_window_update(win);
								rt_thread_delay( RT_TICK_PER_SECOND );
							}
							clr_win(win,0XE73C,0,_YOFFSET,170,300);
							font.fontcolor = 0X4208;
							goto __loop1;						
					}
				}
					break;
				case CODE_RIGHT:
				case KEY_U | KEY_DOWN:
					if(pos>0)pos--;
					_system1(&font,pos);
					sui_window_update(win);
					break;
				case CODE_LEFT:
				case KEY_D | KEY_DOWN:
					if(pos<4)pos++;
					_system1(&font,pos);
					sui_window_update(win);
					break;
				case KEY_F6 | KEY_UP:
				case KEY_EXIT | KEY_DOWN:
					return 0;
				}
		}
	}
}
#undef		_XOFFSET
#undef		_YOFFSET
#undef		_X2OFFSET
#undef		_XSIZEOF

#define 	_XOFFSET	 20
#define		_YOFFSET	 30
#define 	_X2OFFSET	 150
#define 	_X3OFFSET	 97
#define 	_X4OFFSET	 227
#define		_XSIZEOF	 50
static void _system2(struct font_info_t *font,u8 pos)
{
	char buf[8];
	rt_uint32_t tmp;
	clr_win(font->panel,0XE73C,_X3OFFSET,_YOFFSET,150,_XSIZEOF);
	clr_win(font->panel,0XE73C,_X4OFFSET,_YOFFSET,150,_XSIZEOF);
	if(pos>6)
	{
		
		clr_win(font->panel,0x53fa,_X4OFFSET,_YOFFSET+8+20*(pos-7),20,_XSIZEOF);

	}
	else
	{
		
		clr_win(font->panel,0x53fa,_X3OFFSET,_YOFFSET+8+20*pos,20,_XSIZEOF);
	}
	
	tmp = _system_environment.lcdlight;
	if(tmp>10)tmp=10;
	rt_sprintf(buf,"%d",tmp);
	font_draw(_X3OFFSET + (_XSIZEOF-rt_strlen(buf)*8)/2,_YOFFSET+10 ,font,buf);
	
	tmp = _system_environment.beepvolume;
	if(tmp == 1)strcpy(buf,T_STR("中","M"));
	else if(tmp == 2)strcpy(buf,T_STR("低","L"));
	else strcpy(buf,T_STR("高","H"));
	font_draw(_X3OFFSET + (_XSIZEOF-16)/2,_YOFFSET+30 ,font,buf);
	
	tmp = _system_environment.GFI;
	font_draw(_X3OFFSET + (_XSIZEOF-32)/2,_YOFFSET+50 ,font,(tmp)==0?boolean_name[language][0]:boolean_name[language][1]);
	
	tmp = _system_environment.resultsave;
	font_draw(_X3OFFSET + (_XSIZEOF-32)/2,_YOFFSET+70 ,font,(tmp)==0?boolean_name[language][0]:boolean_name[language][1]);
	
	tmp = _system_environment.memorymargintips;
	if(tmp>100)tmp=100;
	rt_sprintf(buf,"%d%",tmp);
	font_draw(_X3OFFSET + (_XSIZEOF-rt_strlen(buf)*8)/2,_YOFFSET+90 ,font,buf);
	
	tmp = _system_environment.portmode;
// 	if(tmp == 1)
		strcpy(buf,T_STR("浮空","float"));
// 	else
// 		strcpy(buf,"接地");
	
	font_draw(_X3OFFSET + (_XSIZEOF-rt_strlen(buf)*8)/2,_YOFFSET+110 ,font,buf);
	
	tmp = _system_environment.systemlanguage;
	if(tmp == 1)
		strcpy(buf,"English");
	else
		strcpy(buf,"中文");
	font_draw(_X3OFFSET + (_XSIZEOF-rt_strlen(buf)*8)/2,_YOFFSET+130 ,font,buf);
	
	tmp = _system_environment.resetstop;
	font_draw(_X4OFFSET + (_XSIZEOF-32)/2,_YOFFSET+10 ,font,(tmp)==0?boolean_name[language][0]:boolean_name[language][1]);
	
	tmp = _system_environment.listdisplay;
	font_draw(_X4OFFSET + (_XSIZEOF-32)/2,_YOFFSET+30 ,font,(tmp)==0?boolean_name[language][0]:boolean_name[language][1]);
	
	tmp = _system_environment.numberingrules;
	if(tmp>10)tmp=10;
	rt_sprintf(buf,"%d",tmp);
	font_draw(_X4OFFSET + (_XSIZEOF-rt_strlen(buf)*8)/2,_YOFFSET+50 ,font,buf);
	
}
static u8 system_environment_setting(void *arg)
{
	const char *option_name[2][10] = {
		{"液晶亮度:","音    量:","GFI 保护:","结果保存:","余量提示:","端口模式:","系统语言:","复位停止:","列表显示:","编号规则:"},
		{ "Bright  : ",
		  "Volume  : ",
			"GFI     : ",
			"Result  : ",
			"Surplus : ",
			"PortMode: ",
			"Language: ",
			"R-Stop  : ",
			"ListView: ",
			"CodeRule: "}
	};
	rt_uint32_t msg;
	rt_uint8_t	pos=0;
	
	struct panel_type *win;
	struct rect_type rect={190,140,200,300};
	struct font_info_t font={0,0X4208,0XE73C,1,0,16};

	win = sui_window_create(T_STR("环境参数设置","Environment parameter set"),&rect);
	font.panel = win;
	
	font_draw(_XOFFSET ,_YOFFSET+10 ,&font,option_name[_system_environment.systemlanguage][0]);
	font_draw(_XOFFSET ,_YOFFSET+30 ,&font,option_name[_system_environment.systemlanguage][1]);
	font_draw(_XOFFSET ,_YOFFSET+50 ,&font,option_name[_system_environment.systemlanguage][2]);
	font_draw(_XOFFSET ,_YOFFSET+70 ,&font,option_name[_system_environment.systemlanguage][3]);
	font_draw(_XOFFSET ,_YOFFSET+90 ,&font,option_name[_system_environment.systemlanguage][4]);
	font_draw(_XOFFSET ,_YOFFSET+110,&font,option_name[_system_environment.systemlanguage][5]);
	font_draw(_XOFFSET ,_YOFFSET+130,&font,option_name[_system_environment.systemlanguage][6]);
	font_draw(_X2OFFSET,_YOFFSET+10 ,&font,option_name[_system_environment.systemlanguage][7]);
	font_draw(_X2OFFSET,_YOFFSET+30 ,&font,option_name[_system_environment.systemlanguage][8]);
	font_draw(_X2OFFSET,_YOFFSET+50 ,&font,option_name[_system_environment.systemlanguage][9]);
	
	_system2(&font,pos);
	sui_window_update(win);
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			
			switch(msg)
			{
				case KEY_L | KEY_DOWN:
					switch(pos)
					{
						case 0:
							if(_system_environment.lcdlight>0)_system_environment.lcdlight--;
							RA8875_SetBackLight(system_parameter_t.env.lcdlight*20+55);
							break;
						case 1:
							if(_system_environment.beepvolume>0)_system_environment.beepvolume--;
							break;
						case 2:
							if(_system_environment.GFI>0)_system_environment.GFI--;
							break;
						case 3:
							if(_system_environment.resultsave>0)_system_environment.resultsave--;
							break;
						case 4:
							if(_system_environment.memorymargintips>0)_system_environment.memorymargintips--;
							break;
						case 5:
							if(_system_environment.portmode>0)_system_environment.portmode--;
							break;
						case 6:
							if(_system_environment.systemlanguage>0)_system_environment.systemlanguage--;
							language = _system_environment.systemlanguage;
							break;
						case 7:
							if(_system_environment.resetstop>0)_system_environment.resetstop--;
							break;
						case 8:
							if(_system_environment.listdisplay>0)_system_environment.listdisplay--;
							break;
						case 9:
							if(_system_environment.numberingrules>0)_system_environment.numberingrules--;
							break;

					}
					
					_system2(&font,pos);
					sui_window_update(win);
					memory_systems_save();
					break;
				case KEY_R | KEY_DOWN:
					switch(pos)
					{
						case 0:
							if(_system_environment.lcdlight<10)_system_environment.lcdlight++;
							RA8875_SetBackLight(system_parameter_t.env.lcdlight*20+55);
							break;
						case 1:
							if(_system_environment.beepvolume<2)_system_environment.beepvolume++;
							break;
						case 2:
							if(_system_environment.GFI<1)_system_environment.GFI++;
							break;
						case 3:
							if(_system_environment.resultsave<1)_system_environment.resultsave++;
							break;
						case 4:
							if(_system_environment.memorymargintips<100)_system_environment.memorymargintips++;
							break;
						case 5:
// 							if(_system_environment.portmode<1)_system_environment.portmode++;
							_system_environment.portmode = 1;
							break;
						case 6:
							if(_system_environment.systemlanguage<1)_system_environment.systemlanguage++;
							language = _system_environment.systemlanguage;
// 							language = 0;
// 							_system_environment.systemlanguage = 0;
							break;
						case 7:
							if(_system_environment.resetstop<1)_system_environment.resetstop++;
							break;
						case 8:
							if(_system_environment.listdisplay<1)_system_environment.listdisplay++;
							break;
						case 9:
// 							if(_system_environment.numberingrules<0)_system_environment.numberingrules++;
							break;

					}
					
					_system2(&font,pos);
					sui_window_update(win);
					memory_systems_save();
					break;
				case CODE_RIGHT:
				case KEY_U | KEY_DOWN:
					if(pos>0)pos--;
					else pos=9;
					_system2(&font,pos);
					sui_window_update(win);
					break;
				case CODE_LEFT:
				case KEY_D | KEY_DOWN:
					if(pos<9)pos++;
					else pos=0;
					_system2(&font,pos);
					sui_window_update(win);
					break;
				case KEY_F6 | KEY_UP:
				case KEY_EXIT | KEY_DOWN:
					return 0;
				}
		}
	}
}
#undef		_XOFFSET
#undef		_YOFFSET
#undef		_X2OFFSET
#undef		_X3OFFSET
#undef		_X4OFFSET
#undef		_XSIZEOF

#define 	_XOFFSET	 20
#define		_YOFFSET	 30
#define 	_X2OFFSET	 150
#define		_XSIZEOF	 100
static void _system3(struct font_info_t *font,u8 pos)
{
	char buf[11];
	rt_uint32_t tmp;
	
	clr_win(font->panel,0XE73C,_X2OFFSET,_YOFFSET,150,_XSIZEOF);
	clr_win(font->panel,0x53fa,_X2OFFSET,_YOFFSET+8+20*pos,20,_XSIZEOF);
	
	tmp = _system_communication.interface;
	if(tmp==0)strcpy(buf,"RS232");
	else if(tmp==1)strcpy(buf,"RS485");
	else if(tmp==2)strcpy(buf," USB ");
	else strcpy(buf,T_STR(" 无  "," NO "));
	font_draw(_X2OFFSET + (_XSIZEOF-40)/2,_YOFFSET+10 ,font,buf);
	
	tmp = _system_communication.control;
	font_draw(_X2OFFSET + (_XSIZEOF-32)/2,_YOFFSET+30 ,font,(tmp)==0?boolean_name[language][0]:boolean_name[language][1]);
	
	tmp = _system_communication.address;
	rt_sprintf(buf,"%d",tmp);
	font_draw(_X2OFFSET + (_XSIZEOF-rt_strlen(buf)*8)/2,_YOFFSET+50 ,font,buf);
	
	tmp = _system_communication.baud = 1;  //douyijun
	if(tmp==0)strcpy(buf,     " 4800 bps ");
	else if(tmp==1)strcpy(buf," 9600 bps ");
	else if(tmp==2)strcpy(buf,"19200 bps ");
	else strcpy(buf,          T_STR(" 无  "," NO "));
	font_draw(_X2OFFSET + (_XSIZEOF-80)/2,_YOFFSET+70 ,font,buf);
	
	tmp = _system_communication.endcode = 1; //douyijun
	if(tmp==0)strcpy(buf,     "CR+LF");
	else if(tmp==1)strcpy(buf,"  #  ");
	else strcpy(buf, T_STR(" 无  "," NO "));
	font_draw(_X2OFFSET + (_XSIZEOF-40)/2,_YOFFSET+90 ,font,buf);
	
	tmp = _system_communication.networkinterface;
	font_draw(_X2OFFSET + (_XSIZEOF-32)/2,_YOFFSET+110 ,font,(tmp)==0?boolean_name[language][0]:boolean_name[language][1]);
	
	tmp = _system_communication.matchinterface;
	font_draw(_X2OFFSET + (_XSIZEOF-32)/2,_YOFFSET+130 ,font,(tmp)==0?boolean_name[language][0]:boolean_name[language][1]);
	
}
static u8 system_communication_setting(void *arg)
{
	rt_uint32_t msg;
	rt_uint8_t	pos=0;
	struct panel_type *win;
	struct rect_type rect={190,140,200,300};
	struct font_info_t font={0,0X4208,0XE73C,1,0,16};
	
	win = sui_window_create(T_STR("通讯配置","Communicate config"),&rect);
	font.panel = win;
	
	/* 开始窗口界面绘制 */
	font_draw(_XOFFSET,_YOFFSET+10 ,&font,T_STR("通讯接口:","Comm port  :"));
	font_draw(_XOFFSET,_YOFFSET+30 ,&font,T_STR("通讯控制:","Comm ctl   :"));
	font_draw(_XOFFSET,_YOFFSET+50 ,&font,T_STR("通讯地址:","Comm addr  :"));
	font_draw(_XOFFSET,_YOFFSET+70 ,&font,T_STR("波特率  :","Baud rate  :"));
	font_draw(_XOFFSET,_YOFFSET+90 ,&font,T_STR("结束码  :","epilog code:"));
	font_draw(_XOFFSET,_YOFFSET+110,&font,T_STR("网络接口:","NetworkPort:"));
	font_draw(_XOFFSET,_YOFFSET+130,&font,T_STR("选配接口:","Match  port:"));

	_system3(&font,pos);
	sui_window_update(win);
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			
			switch(msg)
			{
				case KEY_L | KEY_DOWN:
					switch(pos)
					{
						case 0:
							if(_system_communication.interface>0)_system_communication.interface--;
							rt_mb_send(&screen_mb, UPDATE_STATUS | STATUS_INTERFACE_EVENT | (_system_communication.interface));
							break;
						case 1:
							if(_system_communication.control>0)_system_communication.control--;
							break;
						case 2:
							if(_system_communication.address>0)_system_communication.address--;
							break;
						case 3:
//							if(_system_communication.baud>0)_system_communication.baud--;
							break;
						case 4:
//							if(_system_communication.endcode>0)_system_communication.endcode--;
							break;
						case 5:
							if(_system_communication.networkinterface>0)_system_communication.networkinterface--;
							break;
						case 6:
							if(_system_communication.matchinterface>0)_system_communication.matchinterface--;
							break;

					}
					_system3(&font,pos);
					sui_window_update(win);
					memory_systems_save();
					break;
				case KEY_R | KEY_DOWN:
					switch(pos)
					{
						case 0:
							if(_system_communication.interface<2)_system_communication.interface++;
							rt_mb_send(&screen_mb, UPDATE_STATUS | STATUS_INTERFACE_EVENT | (_system_communication.interface));
							break;
						case 1:
							if(_system_communication.control<1)_system_communication.control++;
							break;
						case 2:
							if(_system_communication.address<255)_system_communication.address++;
							break;
						case 3:
//							if(_system_communication.baud<2)_system_communication.baud++;
							break;
						case 4:
//							if(_system_communication.endcode<2)_system_communication.endcode++;
							break;
						case 5:
							if(_system_communication.networkinterface<1)_system_communication.networkinterface++;
							break;
						case 6:
							if(_system_communication.matchinterface<1)_system_communication.matchinterface++;
							break;

					}
					_system3(&font,pos);
					sui_window_update(win);
					memory_systems_save();
					break;
				case CODE_RIGHT:
				case KEY_U | KEY_DOWN:
					if(pos>0)pos--;
					_system3(&font,pos);
					sui_window_update(win);
					break;
				case CODE_LEFT:
				case KEY_D | KEY_DOWN:
					if(pos<6)pos++;
					_system3(&font,pos);
					sui_window_update(win);
					break;
				case KEY_F6 | KEY_UP:
				case KEY_EXIT | KEY_DOWN:
					{
						uint32_t  usartbaud;
						switch(system_parameter_t.com.baud){
							case 0:
								usartbaud = 4800;
							break;
							
							case 1:
								usartbaud = 9600;
							break;
							
							case 2:
								usartbaud = 19200;
							break;
							
							default:
								usartbaud = 9600;
							break;
		
						}
						
// 						eMBInit(MB_RTU, 0x01, 2, usartbaud,  MB_PAR_NONE);
						usart2_init(usartbaud);
						APP_CommProtocolLogicInit(system_parameter_t.com.address);
					}	
					return 0;
				}
		}
	}
}
#undef		_XOFFSET
#undef		_YOFFSET
#undef		_X2OFFSET
#undef		_XSIZEOF

#define 	_XOFFSET	 20
#define		_YOFFSET	 30
#define 	_X2OFFSET	 160
#define		_XSIZEOF	 100

typedef struct{
	rt_uint16_t   routine_test_year;  //出厂检验年份
	rt_uint16_t   routine_test_month; //出厂检验月份
	rt_uint16_t   routine_test_day;   //出厂检验日期
	rt_uint16_t   start_count;        //开机计数
	rt_uint32_t   test_count;         //测试计数
	rt_uint32_t   single_runtime;     //当次运行时间
	rt_uint32_t   total_runtime;      //累计运行时间
}system_run_info; 

#define  SYS_RUN_INF   (*((system_run_info *)0x40024000))

static u8 system_run_setting(void *arg)
{
	rt_uint32_t msg;
	char buf[50];
	struct panel_type *win;
	struct rect_type rect={170,140,250,360};
	struct font_info_t font={0,0X4208,0XE73C,1,1,16};
// 	SYS_RUN_INF.single_runtime = 0;
	win = sui_window_create(T_STR("运行日志","Run Date"),&rect);
	font.panel = win;
	memset(buf,0,sizeof(buf));
	/* 开始窗口界面绘制 */
	font_draw(_XOFFSET,_YOFFSET+10 ,&font,T_STR("仪器固化型号:","Device Solid      : "));
	font_draw(_XOFFSET,_YOFFSET+30 ,&font,T_STR("仪器测试模式:","Device Test Mode  : "));
	font_draw(_XOFFSET,_YOFFSET+50 ,&font,T_STR("软件固化版本:","Soft Solid Version: "));
	font_draw(_XOFFSET,_YOFFSET+70 ,&font,T_STR("硬件固化版本:","Hard Solid Version: "));
	font_draw(_XOFFSET,_YOFFSET+90 ,&font,T_STR("出厂检验日期:","PredeLiveryDate: "));
	font_draw(_XOFFSET,_YOFFSET+110,&font,T_STR("总计开机次数:","Total Up Time  : "));
	font_draw(_XOFFSET,_YOFFSET+130,&font,T_STR("总计测试次数:","Total Test Time: "));
	font_draw(_XOFFSET,_YOFFSET+150,&font,T_STR("开机运行时间:","Run Time       : "));
	font_draw(_XOFFSET,_YOFFSET+170,&font,T_STR("总计运行时间:","Total Run Time : "));

//	font_draw(_X2OFFSET,_YOFFSET+10 ,&font,"     CS9931WYS     ");
	rect.x = _X2OFFSET;rect.y = _YOFFSET+13;rect.h = 8;rect.w = 160;
	ui_text_draw_alpha(&font,&rect,CS9931_Config.Type_Name);
	rect.x = _X2OFFSET;rect.y = _YOFFSET+33;rect.h = 8;rect.w = 160;
	rt_sprintf(buf,"%s%s%s%s%s%s%s%s",CS9931_Config.ACW_Enable? "ACW ":"",
			                              CS9931_Config.DCW_Enable? "DCW ":"",
										  CS9931_Config.GR_Enable? "GR ":"",
										  CS9931_Config.LC_Enable? "LC ":"",
										  CS9931_Config.ACW_GR_Enable? "AG ":"",
										  CS9931_Config.DCW_GR_Enable? "DG ":"",
										  CS9931_Config.IR_Enable? "IR ":"",
										  CS9931_Config.PW_Enable? "PW":"");
	ui_text_draw_alpha(&font,&rect,buf);
//	font_draw(_X2OFFSET,_YOFFSET+30 ,&font,"   ACW DCW GR LC   ");

	font_draw(_X2OFFSET,_YOFFSET+50 ,&font,"      "TEST_SOFT_VERISON"     ");//测试程序版本
	font_draw(_X2OFFSET,_YOFFSET+70 ,&font,"       "HARD_VERISON"        ");//硬件版本
	
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			switch(msg)
			{
				case KEY_ENTER | KEY_LONG:
				{
					rt_device_t device;
					struct rtc_time_type	time;
					device = rt_device_find("rtc");
					if(device != RT_NULL)
					{
						rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);						
						SYS_RUN_INF.routine_test_year    = time.year;
						SYS_RUN_INF.routine_test_month   = time.month;
						SYS_RUN_INF.routine_test_day     = time.date;
					}
				}
				break;
				
				case KEY_D | KEY_LONG:
				{
					SYS_RUN_INF.single_runtime = 0;
					SYS_RUN_INF.total_runtime  = 0;
					SYS_RUN_INF.start_count    = 0;
					SYS_RUN_INF.test_count     = 0;
				}
				break;
				
				case KEY_F6 | KEY_UP:
				case KEY_EXIT | KEY_DOWN:
					return 0;
				}
		}else{
			clr_win(font.panel,0XE73C,_X2OFFSET,_YOFFSET+90,100,160);
			rt_sprintf(buf,T_STR("%d年%d月%d日","%dY %dM %dD"),SYS_RUN_INF.routine_test_year + 2000,
			                               SYS_RUN_INF.routine_test_month,
			                               SYS_RUN_INF.routine_test_day);
			if(strlen(buf) > 20)rt_sprintf(buf,"---");
			font_draw(_X2OFFSET + (20 - strlen(buf))*4 ,_YOFFSET+90 ,&font,buf);
			rt_sprintf(buf,T_STR("%d次","%dTime"),SYS_RUN_INF.start_count);                                       
			font_draw(_X2OFFSET + (20 - strlen(buf))*4,_YOFFSET+110,&font,buf);
			rt_sprintf(buf,T_STR("%d次","%dTime"),SYS_RUN_INF.test_count);
			font_draw(_X2OFFSET + (20 - strlen(buf))*4,_YOFFSET+130,&font,buf);
			rt_sprintf(buf,T_STR("%3d天%02d小时%02d分钟%02d秒","%3dD%02dH%02dM%02dS"),SYS_RUN_INF.single_runtime / 60 / 60 / 24,
			                                             SYS_RUN_INF.single_runtime / 60 / 60 % 24,
			                                             SYS_RUN_INF.single_runtime / 60 % 60,
								                                   SYS_RUN_INF.single_runtime % 60);
			if(strlen(buf) > 22){
				rt_sprintf(buf,T_STR("%3d天%02d小时%02d分钟","%3dD%02dH%02dM"),SYS_RUN_INF.single_runtime / 60 / 60 / 24,
			                                             SYS_RUN_INF.single_runtime / 60 / 60 % 24,
			                                             SYS_RUN_INF.single_runtime / 60 % 60);
			}
			font_draw(_X2OFFSET-16 + (22 - strlen(buf))*4,_YOFFSET+150,&font,buf);
			rt_sprintf(buf,T_STR("%3d天%02d小时%02d分钟%02d秒","%3dD%02dH%02dM%02dS"),SYS_RUN_INF.total_runtime / 60 / 60 / 24,
			                                             SYS_RUN_INF.total_runtime / 60 / 60 % 24,
			                                             SYS_RUN_INF.total_runtime / 60 % 60,
								                                   SYS_RUN_INF.total_runtime % 60);
			if(strlen(buf) > 22){
				rt_sprintf(buf,T_STR("%3d天%02d小时%02d分钟","%3dD%02dH%02dM"),SYS_RUN_INF.total_runtime / 60 / 60 / 24,
			                                             SYS_RUN_INF.total_runtime / 60 / 60 % 24,
			                                             SYS_RUN_INF.total_runtime / 60 % 60);
			}
			font_draw(_X2OFFSET-16 + (22 - strlen(buf))*4,_YOFFSET+170,&font,buf);

			sui_window_update(win);
		}
		
	}
}
#undef		_XOFFSET
#undef		_YOFFSET
#undef		_X2OFFSET
#undef		_XSIZEOF

static u8 system_default_setting(void *arg)
{
	rt_uint32_t msg;
	struct panel_type *win;
	struct rect_type rect={190,140,200,300};
	struct font_info_t font={0,0xf000,0XE73C,1,0,16};

	win = sui_window_create(T_STR("默认设置","Default setting"),&rect);
	font.panel = win;
	
	font_draw(62,54,&font,T_STR("确定要恢复默认设置吗？","Sure restore default set?"));
	font_draw(36,84,&font,T_STR("警告: 恢复默认参数操作将数据设","Warning: restore default setting "));
	font_draw(84,110,&font,T_STR("置为出厂默认值！","will be default data ! "));

	clr_win(win,CL_orange,40,145,30,80);
	clr_win(win,CL_orange,180,145,30,80);
	
	font.fontcolor = 0X4208;
	font_draw(64,152,&font,T_STR("确 定","OK"));
	font_draw(204,152,&font,T_STR("返回","Return"));
	
	sui_window_update(win);
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			switch(msg)
			{
				case KEY_ENTER | KEY_UP:
					memory_systems_defa();
					return 0;
				/* 返回 */
				case KEY_F6 | KEY_UP:
					case KEY_EXIT | KEY_DOWN:
					return 0;
			}
		}
	}
}


static u8 system_mode_setting(void *arg)
{
	rt_uint32_t msg;
	struct panel_type *win;
	struct rect_type rect={190,140,200,300};
	struct font_info_t font={0,0X4208,0x53fa,1,1,16};
	struct num_format num;

__loop5:
	rect.x = 190;
	rect.y = 140;
	rect.h = 200;
	rect.w = 300;
	
	//douyijun   检查校准开关是否打开
	{
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6) == 0)
		{
			win = sui_window_create(T_STR("校准开关确认","Align switch ensure"),&rect);
			font.panel = win;
			font_draw(80,80,&font,T_STR("请打开校准开关！！","Please open Align switch!!"));
			sui_window_update(win);
			while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6) == 0)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_EXIT | KEY_UP:
						case KEY_F6   | KEY_UP:
							return 1;
						default:
						break;
					}
				}
			}
		}
	}
	
	win = sui_window_create(T_STR("密码确认","Password ensure"),&rect);
	font.panel = win;
	
	font_draw(100,50,&font,T_STR("请输入密码：","Please Input Password: "));
	rect.x = 100;
	rect.y = 90;
	rect.h = 20;
	rect.w = 100;
	sui_window_update(win);
	ui_text_draw(&font,&rect,"******");
	num.num  = 0;
	num._int = 6;
	num._dec = 0;
	num.min  = 0;
	num.max  = 99999999;
	num.unit = "";
	
	msg = num_input(&font,&rect,&num);
	if(msg == 0xffffffff)
	{
		return 1;
	}
	else if(msg == _system_password.systempasswordm)
	{
		return 0;
	}
	else
	{
		font.fontcolor = 0xf000;
		font_draw(100,160,&font,T_STR("密码错误！","Password Error! "));
		font.fontcolor = 0X4208;
		sui_window_update(win);
		rt_thread_delay( RT_TICK_PER_SECOND);
		goto __loop5;
	}
}
static u8 system_arc_setting(void *arg)
{
	rt_uint32_t msg;
	struct panel_type *win;
	struct rect_type rect={190,140,200,300};
	
	win = sui_window_create(T_STR("电弧校准","Arc calibration "),&rect);
	sui_window_update(win);
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			return 0;
		}
	}
}





























static void Delay_ms(unsigned int dly_ms)
{
  unsigned int dly_i;
  while(dly_ms--)
    for(dly_i=0;dly_i<18714;dly_i++);
}

void ctrl_signal_dault_relay(uint8_t st)
{
	if(LC_TEST_MODE	== LC_YY)
	{
		if(st == RELAY_ON)
		{
			LC_Relay_Control(Signal_Dault,1,1);
		}
		else
		{
			LC_Relay_Control(Signal_Dault,0,1);
		}
	}
	else
	{
		if(st == RELAY_ON)
		{
			LC_Relay_Control(Signal_Dault,0,1);
		}
		else
		{
			LC_Relay_Control(Signal_Dault,1,1);
		}
	}
    
	Delay_ms(100);
}
/*
	mode:ACW,DCW,GR,LC
	value:输出的DA值
*/


void dyj_output(u8 mode, u16 value)
{
	switch(mode)
	{
		case ACW:
//			AC_Set_DAValue(value,60); //2016.9.2 wangxin
			AC_Set_Output_DA(value,60); //2016.9.2 wangxin
			AC_Output_Enable();
			break;
		case DCW:
//			DC_Set_DAValue(value);
			DC_Set_Output_DA(value);//2016.9.2 wangxin
			DC_Output_Enable();
			break;
		case GR:
//			GR_Set_DA_Value(value,60);
			GR_Set_ouput_da(value, 60);//2016.9.2 wangxin
			GR_Output_Enable();
			break;
		case LC:
				if(value)
				{
                    ctrl_signal_dault_relay(RELAY_OFF);
        
					Relay_ON(EXT_DRIVER_O8);
					LC_Main_ADValue_Set(value, 60);
					
					Relay_ON(AMP_RELAY_5);
					Relay_ON(AMP_RELAY_3);
					Relay_ON(EXT_DRIVER_O6);
					Relay_ON(EXT_DRIVER_O1);
                    ctrl_relay_EXT_DRIVE_O4_O5(RELAY_ON);///<2017.5.11 wangxin
				}
				else
				{
                    ctrl_signal_dault_relay(RELAY_ON);
					
					LC_Main_ADValue_Set(value, 60);
					
					Relay_OFF(EXT_DRIVER_O8);
					Relay_OFF(AMP_RELAY_5);
					Relay_OFF(AMP_RELAY_3);
					Relay_OFF(EXT_DRIVER_O6);
					Relay_OFF(EXT_DRIVER_O1);
                    ctrl_relay_EXT_DRIVE_O4_O5(RELAY_OFF);///<2017.5.11 wangxin
				}
				
			break;
		case LC_ASSIST:
				if(value){
					Relay_ON(EXT_DRIVER_O8);
                    ctrl_relay_EXT_DRIVE_O4_O5(RELAY_ON);///<2017.5.11 wangxin
					Relay_ON(AMP_RELAY_1);
					Relay_ON(AMP_RELAY_2);
					LC_Assit_ADValue_Set(value, 60);/* value 输出DA 频率60Hz */
				}else{
					LC_Assit_ADValue_Set(value,60);
					Relay_OFF(EXT_DRIVER_O8);
                    ctrl_relay_EXT_DRIVE_O4_O5(RELAY_OFF);///<2017.5.11 wangxin
					Relay_OFF(AMP_RELAY_1);
					Relay_OFF(AMP_RELAY_2);
				}
				
			break;
	}
}
/*
	保存对应的AD值
	mode:模式
	item:项目
		enum{
			CAL_VOL=0,
			CAL_VOLS,
			CAL_CUR,
			CAL_ARC,
		};
	cal:第几个校准点
	val:值
*/
void dyj_save_ad(u8 mode, u8 item, u8 cal, u32 val)
{
	switch(mode)
	{
		case ACW:
			switch(item)
			{
				case CAL_VOL:// 电压
					ACW_Cal_Facter_Refresh(item, cal, val);
					break;
				case CAL_CUR:// 电流
					ACW_Cal_Facter_Refresh(item, cal, val);
					break;
				case CAL_ARC:
					ACW_Cal_Facter_Refresh(item, cal, val);
					break;
			}
			break;
		case DCW:
			switch(item)
			{
				case CAL_VOL:// 电压
					DCW_Cal_Facter_Refresh(item, cal, val);
					break;
				case CAL_CUR:// 电流
					DCW_Cal_Facter_Refresh(item, cal, val);
					break;
				case CAL_ARC:
					DCW_Cal_Facter_Refresh(item, cal, val);
					break;
			}
			break;
		case GR:
			switch(item)
			{
				case CAL_VOL:// 电压
					GR_Cal_Facter_Refresh(item, cal, val);
				break;
			}
			break;
		case LC:
			switch(item)
			{
				case CAL_VOL:// 电压
						LC_Cal_M_V_Facter_Refresh(item, cal, val);
					break;
				case CAL_VOLS:// 辅助电压
						LC_Cal_A_V_Facter_Refresh(item, cal, val);
					break;
				case CAL_CUR:// 电流
						LC_Cal_A_V_Facter_Refresh(item, cal, val);
					break;
			}
			break;
		case LC_ASSIST:
			switch(item)
			{
				case CAL_VOL:// 电压
						LC_Cal_M_V_Facter_Refresh(item, cal, val);
					break;
				case CAL_VOLS:// 辅助电压
						LC_Cal_A_V_Facter_Refresh(item, cal, val);
					break;
				case CAL_CUR:// 电流
					break;
			}

			break;
	}
}

void dyj_save_lc_selv_ad(u32 val)
{
	uint8_t i = 0;
	uint32_t temp = 0;
	Global_Cal_Facter.SELV_Facter.LC_Cal_Point_Num  = 1;
	for(;i<10;i++){	
 		temp += D3_Mcp3202_Read(1);
		cal_Delay_ms(100);
	}
	Global_Cal_Facter.SELV_Facter.LC_Cal_Point[0].AD_value = temp / i;
	Global_Cal_Facter.SELV_Facter.LC_Cal_Point[0].Voltage  = val;
	
}


void dyj_save_lc_cur_ad(u8 network, u8 dem, u8 cal, u32 val)
{
	uint8_t i = 0;
	uint32_t temp = 0;
	for(;i<2;i++){
		
 		temp += D3_Mcp3202_Read(0);
		cal_Delay_ms(100);
		
	}
	Global_Cal_Facter.LC_C_Facter.LC_Cal_Point[network - MD_E][dem][cal-1].AD_value = temp / i;
	Global_Cal_Facter.LC_C_Facter.LC_Cal_Point[network - MD_E][dem][cal-1].Current  = val;
	    
	LC_Relay_Control(PEAK_DISCHARGE,0,1);
// 	switch(network)
// 	{
// 		case MD_E:
// 			switch(dem)
// 			{
// 				case 0:// AC
// 					
// 					break;
// 				case 1:// AC+DC
// 					
// 					break;
// 				case 2:// PEAK
// 					
// 					break;
// 				case 3:// DC
// 					
// 					break;
// 			}
// 			break;
// 		case MD_F:
// 			switch(dem)
// 			{
// 				case 0:// AC
// 					
// 					break;
// 				case 1:// AC+DC
// 					
// 					break;
// 				case 2:// PEAK
// 					
// 					break;
// 				case 3:// DC
// 					
// 					break;
// 			}
// 			break;
// 		case MD_G:
// 			switch(dem)
// 			{
// 				case 0:// AC
// 					
// 					break;
// 				case 1:// AC+DC
// 					
// 					break;
// 				case 2:// PEAK
// 					
// 					break;
// 				case 3:// DC
// 					
// 					break;
// 			}
// 			break;
// 	}
}

void dyj_save_IR_ad(uint32_t *p_Vol,uint16_t *p_ADValue,uint8_t gear)
{
	uint8_t i = 0;
	for(;i<10;i++){
		Global_Cal_Facter.IR_Facter.Cal_Point[gear].AD_value[i] = p_ADValue[i];
		Global_Cal_Facter.IR_Facter.Cal_Point[gear].Voltage[i]  = p_Vol[i];
	}
	
}

#define   LC_K1         I_200uA_2mA_20mA
#define   LC_K2         GB9706_1
#define   LC_K3         LC_NY
#define   LC_K4         I_200uA_2mA_20mA
#define   LC_K5         ULN544NP
#define   LC_K6         ULN544P
#define   LC_K7         I_200uA_2mA_20mA
#define   LC_K11        GB4943_GBT12113
#define   LC_K12        FIG4_FIG5
#define   LC_K13        FIG3_FIG4
#define   LC_K14        I_200uA_2mA_20mA
#define   LC_K15        GBT12133
#define   LC_K16        I_200uA_2mA_20mA
#define   LC_K17        UL1563
#define   LC_K20        I_200uA_2mA_20mA
#define   LC_K25        I_FILES
#define   LC_K26        I_AC_DC_AC
#define   LC_K27        SELV_AC_DC_AC
#define   LC_K33        MD_HI_GND
#define   LC_K34        G_Change
#define   LC_K35        S3_CONTROL

void dyj_lc_relay(u8 network, u8 dem, u8 cal)
{
	switch(network){
		case MD_A:
			LC_Relay_Control(LC_K3,1,0);
			LC_Relay_Control(LC_K2,1,0);
			LC_Relay_Control(LC_K5,1,0);
			LC_Relay_Control(LC_K6,1,0);
			LC_Relay_Control(LC_K15,1,0);
// 			LC_Relay_Control(LC_K17,1,0);
			LC_Relay_Control(LC_K11,0,0);
			LC_Relay_Control(LC_K12,0,0);
			LC_Relay_Control(LC_K13,0,1);
		break;
		case MD_B:
			LC_Relay_Control(LC_K3,1,0);
			LC_Relay_Control(LC_K2,1,0);
			LC_Relay_Control(LC_K5,1,0);
			LC_Relay_Control(LC_K6,1,0);
			LC_Relay_Control(LC_K15,1,0);
// 			LC_Relay_Control(LC_K17,1,0);
			LC_Relay_Control(LC_K11,0,0);
			LC_Relay_Control(LC_K12,1,0);
			LC_Relay_Control(LC_K13,0,0);
		break;
		case MD_C:
			LC_Relay_Control(LC_K3,1,0);
			LC_Relay_Control(LC_K2,1,0);
			LC_Relay_Control(LC_K5,1,0);
			LC_Relay_Control(LC_K6,1,0);
			LC_Relay_Control(LC_K15,1,0);
// 			LC_Relay_Control(LC_K17,1,0);
			LC_Relay_Control(LC_K11,0,0);
// 			LC_Relay_Control(LC_K12,1,0);
			LC_Relay_Control(LC_K13,1,1);
		break;
		case MD_D:
			LC_Relay_Control(LC_K3,1,0);
			LC_Relay_Control(LC_K2,1,0);
			LC_Relay_Control(LC_K5,1,0);
			LC_Relay_Control(LC_K6,1,0);
			LC_Relay_Control(LC_K15,1,0);
// 			LC_Relay_Control(LC_K17,1,0);
			LC_Relay_Control(LC_K11,1,0);
// 			LC_Relay_Control(LC_K12,1,0);
			LC_Relay_Control(LC_K13,1,1);
		break;
		case MD_E:
			LC_Relay_Control(LC_K3,1,1);
			LC_Relay_Control(LC_K2,0,1);
 			LC_Relay_Control(LC_K5,1,1);
// 			LC_Relay_Control(LC_K6,1,0);
// 			LC_Relay_Control(LC_K15,1,0);
// 			LC_Relay_Control(LC_K17,1,0);
// 			LC_Relay_Control(LC_K11,1,0);
// 			LC_Relay_Control(LC_K12,1,0);
// 			LC_Relay_Control(LC_K13,1,0);
		break;
		case MD_F:
			LC_Relay_Control(LC_K3,1,0);
			LC_Relay_Control(LC_K2,0,0);
			LC_Relay_Control(LC_K5,0,0);
 			LC_Relay_Control(LC_K6,1,1);
// 			LC_Relay_Control(LC_K15,1,0);
// 			LC_Relay_Control(LC_K17,1,0);
// 			LC_Relay_Control(LC_K11,1,0);
// 			LC_Relay_Control(LC_K12,1,0);
// 			LC_Relay_Control(LC_K13,1,0);
		break;
		case MD_G:
			LC_Relay_Control(LC_K3,1,0);
			LC_Relay_Control(LC_K2,1,0);
			LC_Relay_Control(LC_K5,0,0);
			LC_Relay_Control(LC_K6,0,0);
			LC_Relay_Control(LC_K16,0,0);
		  LC_Relay_Control(LC_K17,0,0);
		  LC_Relay_Control(GB7000_1,0,1);
// 			LC_Relay_Control(LC_K5,0,0);
// 			LC_Relay_Control(LC_K6,1,1);
// 			LC_Relay_Control(LC_K15,1,0);
// 			LC_Relay_Control(LC_K17,1,0);
// 			LC_Relay_Control(LC_K11,1,0);
// 			LC_Relay_Control(LC_K12,1,0);
// 			LC_Relay_Control(LC_K13,1,0);
		break;
		case MD_H:
			LC_Relay_Control(LC_K3,1,0);
			LC_Relay_Control(LC_K2,1,0);
			LC_Relay_Control(LC_K5,1,0);
			LC_Relay_Control(LC_K6,1,0);
			LC_Relay_Control(LC_K15,1,0);
			LC_Relay_Control(LC_K17,1,1);
// 			LC_Relay_Control(LC_K11,0,0);
// 			LC_Relay_Control(LC_K12,0,0);
// 			LC_Relay_Control(LC_K13,0,1);
		break;

		default:
			
		break;
	}
	
	switch(dem){
		case 0:    // AC
			LC_Relay_Control(LC_K26,0,1);
			LC_4051_D1_SELECT(1);
		
			LC_Relay_Control(LC_K27,0,1);
			LC_4051_D15_SELECT(1);	
		break;
		case 1:    // AC+DC
			LC_Relay_Control(LC_K26,1,1);
			LC_4051_D1_SELECT(1);
		
			LC_Relay_Control(LC_K27,1,1);
			LC_4051_D15_SELECT(1);
		break;
		case 2:    // PEAK
			LC_4051_D1_SELECT(0);
			LC_4051_D15_SELECT(0);
//			LC_Relay_Control(PEAK_DISCHARGE,1,0);
			LC_Relay_Control(SELV_PEAK_DISCHARGE,1,1);
		break;
		case 3:    // DC
			LC_4051_D1_SELECT(2);
			LC_4051_D15_SELECT(2);
		break;

		default:
			
		break;
	}
	
	switch(cal+1){
		case I3uA:
			
		break;
		case I30uA:
			
		break;
		case I300uA:
			LC_Relay_Control(I_200uA_2mA_20mA,1,1);
			LC_Relay_Control(LC_K25,0,1);
		break;
		case I3mA:
			LC_Relay_Control(I_200uA_2mA_20mA,1,0);
			LC_Relay_Control(LC_K25,1,1);
		break;
		case I30mA:
			LC_Relay_Control(I_200uA_2mA_20mA,0,0);
			LC_Relay_Control(LC_K25,1,1);
		break;
		case I100mA:
			
		break;
		default:
			
		break;
	}
	
	LC_Relay_Control(LC_K33,1,1);
//	Relay_ON(RET_GND_SELECT);
	LC_Relay_Control(PEAK_DISCHARGE,1,1);
//	LC_Relay_Control(SELV_PEAK_DISCHARGE,1,1);
}

