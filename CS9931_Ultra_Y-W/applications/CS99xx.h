/*
 * Copyright(c) 2014,
 * All rights reserved
 * 文件名称：CS99xx.H
 * 摘  要  ：头文件
 * 当前版本：V1.0，编写
 * 修改记录：
 *
 */
#ifndef __CS99xx_H
#define __CS99xx_H

#if defined(__cplusplus)
    extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#include <rtthread.h>

#ifndef	_WIN32
	#include "stm32f4xx.h"
	#include "rtc.h"

	#include "buzzer.h"
	#include "bsp_button.h"
	#include "RA8875.h"
	#include "bsp_sdio_sd.h"

#ifdef RT_USING_DFS
	/* 包含DFS 的头文件*/
	#include <dfs_file.h>
	#include <dfs_posix.h>	
	
	#include "ui_config.h"	
	#include "bmp.h"
	#include "bsp_font.h"
	
#endif
#endif
		
/******************************* 功能选择开关 ***********************************/
#define		FILE_NUM				(50+1) /* 记忆组个数，最多为56 */
#define		TOTALSTEP_NUM			40 /* 总步数，最大为8 */
#define		MEMORY_NUM				100 /* 存储步数 */

#define		DIS_FILE_NUMB			10

#define		MODE_ACW_EN				1 /* 交流耐压模式开关。0：关闭 */
#define		MODE_DCW_EN				1 /* 直流耐压模式开关。0：关闭 */
#define		MODE_IR_EN				1 /* 绝缘电阻模式开关。0：关闭 */
#define		MODE_GR_EN				1 /* 接地电阻模式开关。0：关闭 */

#define		SCPI_ENABLE				0
#define		USBMEM_ENABLE			1
// #define		__DEBUG		
// #define		__USART2

/***************************************************/

/* wangxin 2016.11.28 */
#define GFI_DELAY_COUNT_MAX      10  /* GFI 累积报警 */
#define GFI_CLEAR_COUNT_CYCLE    100 /* GFI 清除计数值周期 单位ms */
typedef struct{
	uint32_t gfi_delay_count;//gfi报警计数
	uint8_t gfi_en;//GFI报警使能
    uint8_t bar_code_sw;//扫描条码开关
}TEST_FLAG;
extern TEST_FLAG test_flag;
extern void gfi_cycle_clear_count(void);
/***************************************************/


enum test_mode_type
{
	ACW,
	ACW_GR,
	DCW,
	DCW_GR,
	IR,
	IR_GR,
	GR,
	LC,
	LC_PW,
	PW,
	LR,
	OSC,
	DLA,
};
enum work_mode_type
{
	N_WORK,
	G_WORK,
};

enum current_gear_type
{
	I3uA   = 0,
	I30uA  = 1,
	I300uA = 2,
	I3mA   = 3,
	I30mA  = 4,
	I100mA = 5,
};
enum error_type
{
	PASS,
	HIGH,
	LOW,
	OPEN,
	SHORT,
	ARC,
	GFI,
	AMP,
	VOL_ABNORMAL,
};
enum dla_type
{
	MD_A = 0,
	MD_B = 1,
	MD_C = 2,
	MD_D = 3,
	MD_E = 4,
	MD_F = 5,
	MD_G = 6,
	MD_H = 7
};
#define offsetof(type,f) ((unsigned int)((char*)&((type*)0)->f))


struct file_info_t
{
	rt_uint8_t	en;
// 	rt_list_t	parent;
	char name[20];              // 文件名
	enum work_mode_type mode;   // 工作模式 N模式 or G模式
	rt_uint8_t totalstep;       // 总步数
	struct rtc_time_type time;  // 建立时间 如:10000s(2014-06-06 12:00:00)
	rt_uint8_t arc;             // 电弧侦测模式 等级 or 电流
	rt_uint16_t outdelay;       // 输出延时时间 范围 0 - 999.9s
	rt_uint16_t passtime;       // PLC PASS保持时间 范围 0 - 999.9s
	rt_uint16_t buzzertime;     // 蜂鸣器报警时间（PASS时蜂鸣时间）
	rt_uint8_t  offset_en;      // offset使能
};

struct step_acw_t
{
	rt_uint8_t step;
	rt_uint8_t mode;
	rt_uint16_t outvol; // output voltage.
	enum current_gear_type curgear; // current gear.
	rt_uint16_t curhigh; // current high limit.
	rt_uint16_t curlow; // current low limit.
	rt_uint16_t rmscur; // RMS current.
	rt_uint16_t startvol; // start voltage.
	rt_uint16_t waittime; // wait time.
	rt_uint16_t ramptime; // ramp time.
	rt_uint16_t testtime; // test time.
	rt_uint16_t downtime; // down time.
	rt_uint16_t pausetime; // pause time.
	rt_uint16_t arc; // msb D15-D12 is grade and D11-D0 is current  modele.
	rt_uint16_t outfreq; // out frequency.
	rt_uint8_t steppass; // step pass.
	rt_uint8_t stepcontinuation; // step continuation.
	rt_uint8_t failstop; // test fail stop.
	rt_uint32_t scanport; // scanning ports.
	rt_uint32_t offsetvalue; //offset value.
};

struct step_dcw_t
{
	rt_uint8_t step;
	rt_uint8_t mode;
	rt_uint16_t outvol; // output voltage.
	enum current_gear_type curgear; // current gear.
	rt_uint16_t curhigh; // current high limit.
	rt_uint16_t curlow; // current low limit.
	rt_uint16_t chargecur; // charging current.
	rt_uint16_t startvol; // start voltage.
	rt_uint16_t waittime; // wait time.
	rt_uint16_t delaytime; // alarm delay times.
	rt_uint16_t ramptime; // ramp time.
	rt_uint16_t testtime; // test time.
	rt_uint16_t downtime; // down time.
	rt_uint16_t pausetime; // pause time.
	rt_uint16_t arc; // msb D15-D12 is grade and D11-D0 is current  modele.
	rt_uint8_t steppass; // step pass.
	rt_uint8_t stepcontinuation; // step continuation.
	rt_uint8_t failstop; // test fail stop.
	rt_uint32_t scanport; // scanning ports.
	rt_uint32_t offsetvalue; //offset value.
};

struct step_ir_t
{
	rt_uint8_t step;
	rt_uint8_t mode;
	rt_uint16_t outvol; // output voltage.
	rt_uint8_t autogear; // auto switch gear.
	rt_uint32_t reshigh; // resistance high limit.
	rt_uint32_t reslow; // resistance low limit.
	rt_uint16_t delaytime; // alarm delay times.
	rt_uint16_t waittime; // wait time.
	rt_uint16_t ramptime; // ramp time.
	rt_uint16_t testtime; // test time.
	rt_uint16_t pausetime; // pause time.
	rt_uint8_t steppass; // step pass.
	rt_uint8_t stepcontinuation; // step continuation.
	rt_uint8_t failstop; // test fail stop.
	rt_uint8_t scanport; // scanning ports.
	rt_uint32_t offsetvalue; //offset value.
};

struct step_gr_t
{
	rt_uint8_t step;
	rt_uint8_t mode;
	rt_uint16_t outcur; // output current.
	rt_uint16_t reshigh; // resistance high limit.
	rt_uint16_t reslow; // resistance low limit.
	rt_uint16_t alarmvol; // alarm voltage.
	rt_uint16_t waittime; // wait time.
	rt_uint16_t ramptime; // ramp time.
	rt_uint16_t testtime; // test time.
	rt_uint16_t pausetime; // pause time.
	rt_uint16_t outfreq; // out frequency.
	rt_uint8_t steppass; // step pass.
	rt_uint8_t stepcontinuation; // step continuation.
	rt_uint8_t failstop; // test fail stop.
	rt_uint32_t offsetvalue; //offset value.
};

struct step_acw_gr_t
{
	rt_uint8_t step;
	rt_uint8_t mode;
	rt_uint16_t outvol; // output voltage.
	enum current_gear_type curgear; // current gear.
	rt_uint16_t curhigh; // current high limit.
	rt_uint16_t curlow; // current low limit.
	rt_uint16_t rmscur; // RMS current.
	rt_uint16_t startvol; // start voltage.
	rt_uint16_t waittime; // wait time.
	rt_uint16_t ramptime; // ramp time.
	rt_uint16_t testtime; // test time.
	rt_uint16_t downtime; // down time.
	rt_uint16_t pausetime; // pause time.
	rt_uint16_t arc; // msb D15-D12 is grade and D11-D0 is current  modele.
	rt_uint16_t outfreq; // out frequency.
	rt_uint8_t steppass; // step pass.
	rt_uint8_t stepcontinuation; // step continuation.
	rt_uint8_t failstop; // test fail stop.
	rt_uint8_t scanport; // scanning ports.
	rt_uint32_t offsetvalue_cw; //offset value.

	rt_uint16_t groutcur; // output current.
	rt_uint16_t grreshigh; // resistance high limit.
	rt_uint16_t grreslow; // resistance low limit.
	rt_uint16_t gralarmvol; // alarm voltage.
	rt_uint16_t groutfreq; // out frequency.
	rt_uint32_t offsetvalue; //offset value.
};

struct step_dcw_gr_t
{
	rt_uint8_t step;
	rt_uint8_t mode;
	rt_uint16_t outvol; // output voltage.
	enum current_gear_type curgear; // current gear.
	rt_uint16_t curhigh; // current high limit.
	rt_uint16_t curlow; // current low limit.
	rt_uint16_t chargecur; // charging current.
	rt_uint16_t startvol; // start voltage.
	rt_uint16_t waittime; // wait time.
	rt_uint16_t delaytime; // alarm delay times.
	rt_uint16_t ramptime; // ramp time.
	rt_uint16_t testtime; // test time.
	rt_uint16_t downtime; // down time.
	rt_uint16_t pausetime; // pause time.
	rt_uint16_t arc; // msb D15-D12 is grade and D11-D0 is current  modele.
	rt_uint8_t steppass; // step pass.
	rt_uint8_t stepcontinuation; // step continuation.
	rt_uint8_t failstop; // test fail stop.
	rt_uint32_t scanport; // scanning ports.
	rt_uint32_t offsetvalue_cw; //offset value.

	rt_uint16_t groutcur; // output current.
	rt_uint16_t grreshigh; // resistance high limit.
	rt_uint16_t grreslow; // resistance low limit.
	rt_uint16_t gralarmvol; // alarm voltage.
	rt_uint16_t groutfreq; // out frequency.
	rt_uint32_t offsetvalue; //offset value.
};

struct step_ir_gr_t
{
	struct step_ir_t ir;

	rt_uint16_t groutcur; // output current.
	rt_uint16_t grreshigh; // resistance high limit.
	rt_uint16_t grreslow; // resistance low limit.
	rt_uint16_t gralarmvol; // alarm voltage.
	rt_uint16_t groutfreq; // out frequency.
	rt_uint32_t offsetvalue; //offset value.
};
struct step_lc_t
{
	rt_uint8_t step;
	rt_uint8_t mode;
	rt_uint16_t outvol; // output voltage.
	enum current_gear_type curgear; // current gear.
	rt_uint16_t curhigh; // current high limit.
	rt_uint16_t curlow; // current low limit.

	rt_uint16_t ramptime; // ramp time.
	rt_uint16_t testtime; // test time.
	rt_uint16_t pausetime; // pause time.
	rt_uint16_t outfreq; // out frequency.
	
	rt_uint8_t NorLphase; // N or L phase.
	rt_uint8_t curdetection; // current detection.
	
	rt_uint8_t steppass; // step pass.
	rt_uint8_t stepcontinuation; // step continuation.
	rt_uint8_t failstop; // test fail stop.
	rt_uint8_t scanport; // scanning ports.
	
	rt_uint16_t MDvol; // MD voltage.
	rt_uint8_t	MDnetwork; // MD network.
	
	/* 医用才有 */
	rt_uint16_t	assistvol;//辅助电压：关、30--300V
	rt_uint8_t	singlefault;//单一故障：开、关
	rt_uint8_t	MDpostion;//MD位置：MD1、MD2、MD3、MD4
	rt_uint8_t	SL;//MD接地、浮地
	rt_uint8_t	SH;//开、关
	rt_uint8_t	S7;
	rt_uint8_t	S8;
	rt_uint8_t	S9;
	rt_uint8_t	S10;
	rt_uint8_t	S11;
	rt_uint8_t	S12;
	rt_uint8_t	S13;
	rt_uint8_t	S14;
	
	rt_uint32_t offsetvalue; //offset value.
};
struct step_pw_t
{
	rt_uint8_t step;
	rt_uint8_t mode;
	rt_uint16_t outvol;      // output voltage.
	rt_uint16_t curhigh;     // current high limit.
	rt_uint16_t curlow;      // current low limit.
	rt_uint16_t pwhigh;      // pw high limit.
	rt_uint16_t pwlow;       //  pw low limit.
	rt_uint16_t factorhigh;  // factor high limit.
	rt_uint16_t factorlow;   //  factor low limit.
	rt_uint16_t waittime; // wait time.
	rt_uint16_t testtime; // test time.
	rt_uint16_t pausetime; // pause time.
	rt_uint16_t outfreq;     // out frequency.
	rt_uint8_t steppass; // step pass.
	rt_uint8_t stepcontinuation; // step continuation.
	rt_uint8_t failstop; // test fail stop.
	rt_uint32_t scanport; // scanning ports.
	rt_uint32_t offsetvalue; //offset value.
};
//struct step_lr_t
//{
//
//};
//struct step_osc_t
//{
//
//};
//struct step_dla_t
//{
//
//};


struct system_password
{
	rt_uint8_t	systempasswordm_en;
// 	char systempasswordm[10];
	rt_uint32_t systempasswordm;
	
	rt_uint8_t	keylockmem_en;
	
	rt_uint8_t	keylockpasswordm_en;
// 	char keylockpasswordm[10];
	rt_uint32_t keylockpasswordm;
	
};
typedef struct system_password *system_password_t;

struct system_environment
{
	rt_uint8_t	lcdlight;
	rt_uint8_t	beepvolume;
	rt_uint8_t	GFI;
	rt_uint8_t	resultsave;
	rt_uint8_t	memorymargintips;
	rt_uint8_t	portmode;
	rt_uint8_t	systemlanguage;
	rt_uint8_t	resetstop;
	rt_uint8_t	listdisplay;
	rt_uint8_t	numberingrules;
	
};
typedef struct system_environment *system_environment_t;

struct system_communication
{
	rt_uint8_t	interface;
	rt_uint8_t	control;
	rt_uint8_t	address;
	rt_uint8_t	baud;
	rt_uint8_t	endcode;
	rt_uint8_t	networkinterface;
	rt_uint8_t	matchinterface;
	
};
typedef struct system_communication *system_communication_t;

struct system_operationlog
{
	rt_uint32_t	model;
	rt_uint32_t	testmodes;
	rt_uint32_t	softwareversion;
	rt_uint32_t	hardwareversion;
	rt_uint32_t	factoryinspectiondate;
	rt_uint32_t	boottotalnumber;
	rt_uint32_t	testtotalnumber;
	rt_uint32_t	startuptime;
	rt_uint32_t	totalruntime;
	
};
typedef struct system_operationlog *system_operationlog_t;


struct system_parameter
{
	rt_uint32_t						bkp;
	
	rt_uint8_t						key_lock;
	rt_uint8_t						Com_lock;
	
	struct system_password 			  psd;
	struct system_environment		  env;
	struct system_communication		com;
	struct system_operationlog		ope;
};

/******************* 函数声明 *********************/
void cs99xx_init(void);
struct file_info_t *read_file_info(rt_uint8_t num);
float strtofloat(char *p,char len);

void CS99xx_GPIO_Config(void);
void CS99xx_Peripheral_Config(void);

/********************** 外部用到的变量 **************************/
extern rt_thread_t com_tid;

extern rt_uint8_t current_file_num;
extern struct file_info_t file_info[FILE_NUM];

extern struct system_parameter system_parameter_t;
#define	 language					(system_parameter_t.env.systemlanguage)


extern rt_uint8_t panel_flag;
extern rt_uint8_t startup_flag;
extern rt_uint8_t current_step_num; 
extern char version_name[2][40];
extern const char *mode_name[];
extern const char *curgear_name[];
extern const char* boolean_name[2][2];
extern const char* single_boolean_name[2][2];
extern const char* sw_status_name[2][2];
extern const char* sw_status_name_static[2][2];
extern const char *workmode_name[];
extern const char *error_name[2][10];

extern const char* lc_phase_name[2][2];
extern const char* lc_detection_name[];
extern const char* lc_MDnetwork_name[];


/******************************************/
#define T_STR(S1, S2) 	language? S2:S1    									//************eng_ch***********
#define T_ARRY(CHI_A, ENG_A,n)  language? ENG_A[n]:CHI_A[n]

/******************************************/


extern unsigned short elitech_logo_bm[];

/******************************************/
void update_gif_protect_function(void);
void draw_elitech_custom_logo(uint16_t x, uint16_t y);
/******************************************/



/**************************************************/
#if defined(__cplusplus)
    }
#endif 
/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/
#endif
