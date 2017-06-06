#ifndef __TEST_SCHED_H__
#define __TEST_SCHED_H__


/*包含的头文件*/
#include <stdint.h>
#include "stm32f4xx.h"
#include "CS99xx.h"
#include "memory.h"


/*宏定义*/

#define    TEST_SCHED_PERIODIC             (10)     //测试函数调用周期
#define    TEST_SAMPLE_PERIODIC            (25)     //测试函数调用周期

#define    TEST_SCHED_STATE_INIT            (0)     //测试调度器在初始化状态
#define    TEST_SCHED_STATE_RUNNING         (1)     //测试调度器在运行状态
#define    TEST_SCHED_STATE_STOP            (2)     //测试调度器在停止状态

#define    TEST_STEP_OUT                    (0)     //测试步——不在测试状态
#define    TEST_STEP_TEST_WAIT              (1)     //测试步——等待测试状态
#define    TEST_STEP_VOL_RISE               (2)     //测试步——电压上升状态
#define    TEST_STEP_TESTING                (3)     //测试步——测试状态          
#define    TEST_STEP_VOL_DOWN               (4)     //测试步——电压下降状态
#define    TEST_STEP_PAUSE                  (5)     //测试步——步间等待
#define    TEST_STEP_STOP                   (6)     //测试步——测试停止

#define    TEST_STATE_REFRESH_EVENT         (0)     //状态刷新事件
#define    TEST_VOLTAGE_REFRESH_EVENT       (1)     //电压刷新事件
#define    TEST_CURRENT_REFRESH_EVENT       (2)     //电流刷新事件
#define    TEST_RESISTER_REFRESH_EVENT      (3)     //电阻刷新
#define    TEST_TIME_COUNT_REFRESH_EVENT    (4)     //时间计数刷新事件
#define    TEST_WARNING_REFRESH_EVENT       (5)     //告警刷新事件
#define    TEST_MAIN_CURRENT_REFRESH_EVENT  (6)     //LC主电流
#define    TEST_MAIN_POWER_REFRESH_EVENT    (7)     //LC主功率


#define    TEST_STATE_VOL_RISE              (0)     //电压上升
#define    TEST_STATE_TEST                  (1)     //正在测试
#define    TEST_STATE_VOL_DOWN              (2)     //电压下降
#define    TEST_STATE_PAUSE                 (3)     //间隔等待
#define    TEST_STATE_WAIT                  (4)     //等待测试
#define    TEST_STATE_PASS                  (5)     //测试合格
#define    TEST_STATE_STOP                  (6)     //停止测试
#define    TEST_STATE_HIGH                  (7)     //上限报警
#define    TEST_STATE_LOW                   (8)     //下限报警
#define    TEST_STATE_SHORT                 (9)     //短路报警
#define    TEST_STATE_VOL_ABNORMAL          (10)    //电压异常
#define    TEST_STATE_ARC_ALARM             (11)    //电弧报警
#define    TEST_STATE_GFI_ALARM             (12)    //GFI报警
#define    TEST_STATE_FAIL                  (13)    //测试失败
#define    TEST_STATE_REAL_ALARM            (14)    //电流刷新事件
#define    TEST_STATE_CHARGE_ALARM          (15)    //充电报警
#define    TEST_STATE_RANGE_ALARM           (16)    //量程报警
#define    TEST_STATE_PA_ALARM              (17)    //功放报警
#define    TEST_STATE_OUTPUT_DELAY          (18)    //输出延时
#define    TEST_STATE_OVERLOAD              (19)    //过载报警

/*数据类型定义*/
typedef  UN_STR *(CALLBACK_GET_TESTPARAM)(void);    //获取测试参数的回调函数

typedef struct{
	
	uint8_t                  Stop_Flag        : 1;    //Stop按下状态
	uint8_t                  Pass_Flag        : 1;    //测试通过标志
	uint8_t                  Test_Sched_State : 2;    //测试调度器的当前状态
	uint8_t                  Test_Step_State  : 4;    //测试步状态
	CALLBACK_GET_TESTPARAM   *p_Get_Test_Param_Fun;   //获取测试参数的回调函数
	UN_STR                   *p_Test_Param;           //指向参数的指针
	uint8_t                  Short_Flag       : 1;    //短路中断标志
	uint8_t                  Warning_Flag     : 1;    //短路中断标志
	uint8_t                  gfi_Flag         : 1;    //GFI中断标志
	uint8_t                  arc_Flag         : 1;    //arc中断标志
	uint8_t                  arc_Isable       : 1;    //arc中断使能标志
	uint8_t                  Test_Status;             //测试状态  
	uint8_t                  Offset_Get_Flag  : 1;    //获取offset值的标志
	uint8_t                  Offset_Is_Flag   : 1;    //测试过程中是否用加入offset
	uint8_t                  Pause_Flag       : 1;    //暂停标志
	uint8_t                  Continue_Flag    : 1;    //继续标志
	
}TEST_SCHED_PARAM;                                  //测试过程参数的结构体

//全局变量
extern	TEST_SCHED_PARAM    Test_Sched_Param;           //调度测试参数
extern volatile int sampling_mutex_flag;//采样使用的互斥量
extern uint16_t cur_ave;
extern uint16_t vol_ave;
extern rt_timer_t Test_Sampling_Timer;
#define LC_SAMPLING_DEBUG   0

/*文件功能函数接口*/
void Test_Sched_Start(CALLBACK_GET_TESTPARAM   *p_Get_Test_Param);
void Test_Sched_Environment_Init(void);
void Test_Sched_Close(void);
UN_STR *function1(void);
void Test_Sched_Main(void *p_Param);
void dis_test_pass(void);






#define DEEP_SLOW		10
#define DEEP_FAST		5
#define DEEP_MID		80
#define SLITHER_DEEP    DEEP_SLOW
#define SLITHER_DEEP_	DEEP_SLOW
typedef struct{
    uint32_t index;
    uint64_t sum;
    uint32_t c;
    uint16_t buf[SLITHER_DEEP_];/* 滤波缓冲 */
}SLITHER_AVE_INFO;

extern SLITHER_AVE_INFO lc_fs[2];
extern SLITHER_AVE_INFO acw_fs[2];

extern uint16_t slither_f(SLITHER_AVE_INFO *info, uint16_t value);
extern void clear_slither_data(void);

#define     LC_FS_CUR  0
#define     LC_FS_VOL  1

#define     ACW_FS_CUR  0
#define     ACW_FS_VOL  1




#endif

