#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "stm32f4xx.h"
#include "CS99xx.h"
#include "rtservice.h"
#include "rtdef.h"
#include "stdio.h"	/* 引入 NULL 定义 */
#include "stdlib.h"
#include "spi_flash.h"

// #define FILE_MAX	120	/* 文件的最大数目 */
// #define TOTALSTEP_NUM	70	/* 文件包含的最大步骤数 */

/********************** Flash分配 **************************/
#define F_PAGE_SIZE (1024*4)	/* Flash 一页的大小 */

#define	FILE_BASE		(1024*64)	/* 文件信息存储基址 空间64k 大概存1400个文件 */
#define FILE_OFFSET		(sizeof(struct file_info_t))	/* 大概44个字节 */
#define PER_P_FILES		(F_PAGE_SIZE/FILE_OFFSET)	/* 每4k能放的文件数目 大概 93个文件 */

#define GROUP_BASE		(1024*128)	/* 记忆组基址 128k */
#define GROUP_OFFSET 	(1024*16)	/* 16K */

#define STEP_OFFSET		sizeof(UN_STR)	/* 大概48个字节 */
#define PER_P_STEPS		(F_PAGE_SIZE/STEP_OFFSET)	/* 每4k能存放的步骤数目 大概85步 */

/***********************************************************/

struct step_com{
	rt_uint8_t step;
	rt_uint8_t mode;
};

typedef union{
	struct step_com com;
	struct step_acw_t acw;
	struct step_dcw_t dcw;
	struct step_ir_t ir;
	struct step_gr_t gr;
	struct step_lc_t lc;
	struct step_acw_gr_t acw_gr;
	struct step_dcw_gr_t dcw_gr;
	struct step_ir_gr_t ir_gr;
	struct step_pw_t pw;
}UN_STR;

/* 节点定义 */
typedef struct{
	UN_STR un;
	struct rt_list_node list;	/* 已经设置的链表 */
}STEP_NODE;

/* 链表定义 */
typedef struct{
	rt_uint8_t size;/* 链表长度 */
	struct rt_list_node *head;/* 链表头 */
	struct rt_list_node *tail;/* 链表尾 */
}CS99XX_LIST;

extern struct rt_list_node *head;/* 链表头 */
extern struct rt_list_node *tail;/* 链表尾 */
// struct rt_list_node *g_free_list;/* 指向空链表 */

extern STEP_NODE step_pool[];/* 记忆组中所有的步 */
extern struct file_info_t file_info[];
extern CS99XX_LIST cs99xx_list;/* 链表定义 */

extern struct rt_list_node *g_cur_step;/* 指向当前步 */

void init_list(void);
void init_acw_step(UN_STR *un);
void init_dcw_step(UN_STR *un);
void init_ir_step(UN_STR *un);
void init_gr_step(UN_STR *un);
void init_lc_step(UN_STR *un);
void init_ir_step(UN_STR *un);
void init_pw_step(UN_STR *un);
void init_acw_gr_step(UN_STR *un);
void init_dcw_gr_step(UN_STR *un);

rt_int32_t read_flash_to_files(void);
rt_int32_t save_files_to_flash(void);
rt_int32_t save_file_to_flash(rt_uint8_t n);
rt_int8_t save_steps_to_flash(const rt_uint8_t n);
rt_int8_t read_flash_to_list(const rt_uint8_t n);

rt_int8_t insert_after(rt_uint8_t step);
struct rt_list_node *position_step(rt_uint8_t pos);
STEP_NODE *remove_step(rt_uint8_t step);
rt_int8_t swap_step(rt_uint8_t one, rt_uint8_t two);

#endif//__MEMORY_H__
