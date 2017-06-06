#include "memory.h"
#include "CS9931_Config.h"

static struct rt_list_node *g_free_list = NULL;/* 指向空链表 */
struct rt_list_node *g_cur_step = NULL;/* 指向当前步 */

STEP_NODE step_pool[TOTALSTEP_NUM];
// struct file_info_t file_info[FILE_NUM];

CS99XX_LIST cs99xx_list;/* 链表定义 */

void init_acw_step(UN_STR *un)
{
	struct step_acw_t _acw;
	_acw.step			=	un->acw.step;
	_acw.mode			=	ACW;
	_acw.outvol			=	50; // output voltage.
	_acw.curgear		=	I3mA; // current gear.
	_acw.curhigh		=	500; // current high limit.
	_acw.curlow			=	0; // current low limit.
	_acw.rmscur			=	0; // RMS current.
	_acw.startvol		=	0; // start voltage.
	_acw.waittime		=	0; // wait time.
	_acw.ramptime		=	0; // ramp time.
	_acw.testtime		=	30; // test time.
	_acw.downtime		=	0; // down time.
	_acw.pausetime		=	0; // pause time.
	_acw.arc			=	0; // msb D15-D12 is grade and D11-D0 is current  modele.
	_acw.outfreq		=	500; // out frequency.
	_acw.steppass		=	1; // step pass.
	_acw.stepcontinuation	=	0; // step continuation.
	_acw.failstop		=	0; // test fail stop.
	_acw.scanport		=	0; // scanning ports.
	
	un->acw = _acw;
}
void init_dcw_step(UN_STR *un)
{
	struct step_dcw_t _dcw;
	_dcw.step			=	un->dcw.step;
	_dcw.mode			=	DCW;
	_dcw.outvol			=	50; // output voltage.
	_dcw.curgear		=	I3mA; // current gear.
	_dcw.curhigh		=	500; // current high limit.
	_dcw.curlow			=	0; // current low limit.
	_dcw.chargecur	=	0; // charging current.
	_dcw.startvol		=	0; // start voltage.
	_dcw.waittime		=	0; // wait time.
	_dcw.delaytime		=	0; // alarm delay times.
	_dcw.ramptime		=	0; // ramp time.
	_dcw.testtime		=	30; // test time.
	_dcw.downtime		=	0; // down time.
	_dcw.pausetime		=	0; // pause time.
	_dcw.arc			=	0; // msb D15-D12 is grade and D11-D0 is current  modele.
	_dcw.steppass		=	0; // step pass.
	_dcw.stepcontinuation	=	0; // step continuation.
	_dcw.failstop		=	0; // test fail stop.
	_dcw.scanport		=	0; // scanning ports.
	
	un->dcw = _dcw;
}


void init_gr_step(UN_STR *un)
{
	struct step_gr_t _gr;
	_gr.step			  =	un->gr.step;
	_gr.mode			  =	GR;
	_gr.outcur			=	100; // output current.
	_gr.reshigh			=	100; // resistance high limit.
	_gr.reslow			=	0; // resistance low limit.
	_gr.alarmvol		=	0; // alarm voltage.
	_gr.waittime		=	0; // wait time.
	_gr.ramptime		=	0; // ramp time.
	_gr.testtime		=	30; // test time.
	_gr.pausetime		=	0; // pause time.
	_gr.outfreq			=	500; // out frequency.
	_gr.steppass		=	0; // step pass.
	_gr.stepcontinuation=	0; // step continuation.
	_gr.failstop		=	0; // test fail stop.
	
	un->gr = _gr;
}
void init_lc_step(UN_STR *un)
{
	struct step_lc_t _lc;
	_lc.step			=	un->lc.step;
	_lc.mode			=	LC;
	_lc.outvol			=	300;  // output voltage.
	_lc.curgear			=	I3mA; // current gear.
	_lc.curhigh			=	500; // current high limit.
	_lc.curlow			=	0; // current low limit.

	_lc.ramptime		=	0; // ramp time.
	_lc.testtime		=	30; // test time.
	_lc.pausetime		=	0; // pause time.
	_lc.outfreq			=	500; // out frequency.
	
	_lc.NorLphase		=	0; // N or L phase.
	_lc.curdetection	=	0; // current detection.
	
	_lc.steppass		=	0; // step pass.
	_lc.stepcontinuation=	0; // step continuation.
	_lc.failstop		=	0; // test fail stop.
	_lc.scanport		=	0; // scanning ports.
	
	_lc.MDvol			=	0; // MD voltage.
	_lc.MDnetwork		=	MD_G; // MD network.
	
	/* 医用专用 */
	_lc.assistvol		=	2200;
	_lc.singlefault		=	0;
	_lc.MDpostion		=	1;
	_lc.SL				=	0;
	_lc.SH				= 	0;
	_lc.S7				= 	0;
	_lc.S8				= 	0;
	_lc.S9				= 	0;
	_lc.S10				= 	0;
	_lc.S11				= 	0;
	_lc.S12				= 	0;
	_lc.S13				= 	0;
	_lc.S14				= 	0;
	
	un->lc = _lc;
}


void init_ir_step(UN_STR *un)
{
	struct step_ir_t _ir;
	_ir.step			=	un->ir.step;
	_ir.mode			=	IR;	
	_ir.outvol		=	50; // output voltage.
	_ir.autogear	=	0; // auto switch gear.
	_ir.reshigh		=	0; // resistance high limit.
	_ir.reslow		=	100; // resistance low limit.
	_ir.delaytime	=	0; // alarm delay times.
	_ir.waittime	=	0; // wait time.
	_ir.ramptime	=	0; // ramp time.
	_ir.testtime	=	30; // test time.
	_ir.pausetime	=	0; // pause time.
	_ir.steppass	=	0; // step pass.
	_ir.stepcontinuation		=	0; // step continuation.
	_ir.failstop	=	0; // test fail stop.
	_ir.scanport	=	0; // scanning ports.

	un->ir = _ir;
}

void init_pw_step(UN_STR *un)
{
	struct step_pw_t _pw;
	_pw.step			 =	un->pw.step;
	_pw.mode			 =	PW;
	_pw.outvol     =	800;    // output voltage.
	_pw.curhigh    =	500;    // current high limit.
	_pw.curlow     =	0;      // current low limit.
	_pw.pwhigh     =	3000;   // pw high limit.
	_pw.pwlow      =	0;      //  pw low limit.
	_pw.factorhigh =	1000;   // factor high limit.
	_pw.factorlow  =	0;      //  factor low limit.
	_pw.testtime   =	30;     // test time.
	_pw.pausetime  =	0;      // pause time.
	_pw.outfreq    =	600;    // out frequency.
	_pw.steppass   =	0;      // step pass.
	_pw.stepcontinuation   =	0; // step continuation.
	_pw.failstop   =	0;      // test fail stop.
	_pw.scanport   =	0;      // scanning ports.
	
	un->pw = _pw;
}

void init_acw_gr_step(UN_STR *un)
{
	struct step_acw_gr_t _acw_gr;
	_acw_gr.step			  =	un->acw_gr.step;
	_acw_gr.mode			  =	ACW_GR;
	_acw_gr.outvol			=	50; // output voltage.
	_acw_gr.curgear		  =	I3mA; // current gear.
	_acw_gr.curhigh		  =	500; // current high limit.
	_acw_gr.curlow			=	0; // current low limit.
	_acw_gr.rmscur			=	0; // RMS current.
	_acw_gr.startvol		=	0; // start voltage.
	_acw_gr.waittime		=	0; // wait time.
	_acw_gr.ramptime		=	0; // ramp time.
	_acw_gr.testtime		=	30; // test time.
	_acw_gr.downtime		=	0; // down time.
	_acw_gr.pausetime		=	0; // pause time.
	_acw_gr.arc			    =	0; // msb D15-D12 is grade and D11-D0 is current  modele.
	_acw_gr.outfreq		  =	500; // out frequency.
	_acw_gr.steppass		=	1; // step pass.
	_acw_gr.stepcontinuation	=	0; // step continuation.
	_acw_gr.failstop		=	0; // test fail stop.
	_acw_gr.scanport		=	0; // scanning ports.
	
	_acw_gr.groutcur    = 100; // output current.
	_acw_gr.grreshigh   = 100; // resistance high limit.
	_acw_gr.grreslow    = 0; // resistance low limit.
	_acw_gr.gralarmvol  = 0; // alarm voltage.
	_acw_gr.groutfreq   = 500; // out frequency.

	
	un->acw_gr = _acw_gr;
}

void init_dcw_gr_step(UN_STR *un)
{
	struct step_dcw_gr_t _dcw_gr;
	_dcw_gr.step			  =	un->dcw_gr.step;
	_dcw_gr.mode			  =	DCW_GR;
	_dcw_gr.outvol			=	50; // output voltage.
	_dcw_gr.curgear		  =	I3mA; // current gear.
	_dcw_gr.curhigh		  =	500; // current high limit.
	_dcw_gr.curlow			=	0; // current low limit.
	_dcw_gr.chargecur 	=	0; // charging current.
	_dcw_gr.startvol		=	0; // start voltage.
	_dcw_gr.waittime		=	0; // wait time.
	_dcw_gr.delaytime		=	0; // alarm delay times.
	_dcw_gr.ramptime		=	0; // ramp time.
	_dcw_gr.testtime		=	30; // test time.
	_dcw_gr.downtime		=	0; // down time.
	_dcw_gr.pausetime		=	0; // pause time.
	_dcw_gr.arc			    =	0; // msb D15-D12 is grade and D11-D0 is current  modele.
	_dcw_gr.steppass		=	0; // step pass.
	_dcw_gr.stepcontinuation	=	0; // step continuation.
	_dcw_gr.failstop		=	0; // test fail stop.
	_dcw_gr.scanport		=	0; // scanning ports.
	
	_dcw_gr.groutcur    = 100; // output current.
	_dcw_gr.grreshigh   = 100; // resistance high limit.
	_dcw_gr.grreslow    = 0; // resistance low limit.
	_dcw_gr.gralarmvol  = 0; // alarm voltage.
	_dcw_gr.groutfreq   = 500; // out frequency.

	
	un->dcw_gr = _dcw_gr;
}

static void add_node(void)
{
	UN_STR un;
	
	struct rt_list_node *cur = NULL;
	
	un.com.step = 1;
	if(CS9931_Config.ACW_Enable){
		init_acw_step(&un);
	}else if(CS9931_Config.DCW_Enable){
		init_dcw_step(&un);
	}else if(CS9931_Config.GR_Enable){
		init_gr_step(&un);
	}else if(CS9931_Config.LC_Enable){
		init_lc_step(&un);
	}else if(CS9931_Config.IR_Enable){
		init_ir_step(&un);
	}else if(CS9931_Config.PW_Enable){
		init_pw_step(&un);
	}else if(CS9931_Config.ACW_GR_Enable){
		init_acw_gr_step(&un);
	}else if(CS9931_Config.DCW_GR_Enable){
		init_dcw_gr_step(&un);
	}
	
	
	cur = g_free_list;
	
	if(cur != NULL)
	{
		g_free_list = cur->next;
		cur->next = NULL;
		rt_list_entry(cur, STEP_NODE, list)->un = un;
	}
	else
	{
		return;//错误处理,可以使用邮箱或消息队列
	}
	
	if(cs99xx_list.tail)
	{
		cs99xx_list.tail->next = cur;
		cur->prev = cs99xx_list.tail;
	}
	else
	{
		cs99xx_list.head = cur;
	}
	
	cs99xx_list.tail = cur;
	cs99xx_list.size++;
}

void init_list(void)
{
	int8_t i = 0;
	struct rt_list_node *a = NULL;
	struct rt_list_node *b = NULL;
	
	/* 初始化为空链表 */
	memset(step_pool, 0, sizeof step_pool);
	
	for(i = 0; i < TOTALSTEP_NUM-1; i++)
	{
		a = &step_pool[i].list;
		b = &step_pool[i+1].list;
		
		a->next = b;
	}
	
// 	a = &step_pool[i].list;
// 	a->next = NULL;
	
	cs99xx_list.head = NULL;
	cs99xx_list.tail = NULL;
	cs99xx_list.size = 0;
	
	g_free_list = &step_pool[0].list;/* 初始化空链表头 */
	
	add_node();/* 链表默认初始化为一步 acw */
}


STEP_NODE *remove_step(rt_uint8_t step)
{
	struct rt_list_node *node = NULL;
	struct rt_list_node *temp = NULL;
	
	STEP_NODE	*res = NULL;
	rt_uint8_t	sign = 0;
	
	/* 如果链表中只有一个节点 就返回 */
	if(cs99xx_list.size == 1)
	{
		return rt_list_entry(cs99xx_list.head, STEP_NODE, list);
	}
	
	for(node = cs99xx_list.head; NULL != node; node = node->next)
	{
		
		if(rt_list_entry(node, STEP_NODE, list)->un.com.step == step)
		{
			sign = 1;
			break;
		}
	}
	
	/* 没找到 */
	if(!sign)
	{
		return NULL;//ERR
	}
	
	/* 找到了 */
	if(node->prev)
	{
		node->prev->next = node->next;
	}
	else
	{
		cs99xx_list.head = node->next;
		res = rt_list_entry(cs99xx_list.head, STEP_NODE, list);
	}
	
	if(node->next)
	{
		node->next->prev = node->prev;
	}
	else
	{
		cs99xx_list.tail = node->prev;
		res = rt_list_entry(cs99xx_list.tail, STEP_NODE, list);
	}
	
	/* 如果res的结果 为空 就赋值为该节点的下一个节点 */
	if(NULL == res)
	{
		res = rt_list_entry(node->next, STEP_NODE, list);
	}
	
	cs99xx_list.size--;
	
	temp = node;
	for(temp=temp->next;temp;temp = temp->next)
	{
		rt_list_entry(temp, STEP_NODE, list)->un.com.step--;//将新节点后的节点的步数全部减1
	}
	
	memset (&rt_list_entry(temp, STEP_NODE, list)->un , 0, sizeof(UN_STR));
	
	node->next = g_free_list;
	g_free_list = node;
	
	return res;
}

rt_int8_t insert_after(rt_uint8_t step)
{
	struct rt_list_node *node = NULL;
	struct rt_list_node *cur = NULL;
	UN_STR un;
	
	/* 检查step */
	if(step > cs99xx_list.size)
	{
		return -1;
	}
	
	cur = g_free_list;	/* 获取空步 */
	un.com.step = step+1;//插入到当前步的后面
	
//	init_acw_step(&un);	/* 默认插入的是acw */
	if(CS9931_Config.ACW_Enable){
		init_acw_step(&un);
	}else if(CS9931_Config.DCW_Enable){
		init_dcw_step(&un);
	}else if(CS9931_Config.GR_Enable){
		init_gr_step(&un);
	}else if(CS9931_Config.LC_Enable){
		init_lc_step(&un);
	}else if(CS9931_Config.IR_Enable){
		init_ir_step(&un);
	}else if(CS9931_Config.PW_Enable){
		init_pw_step(&un);
	}else if(CS9931_Config.ACW_GR_Enable){
		init_acw_gr_step(&un);
	}else if(CS9931_Config.DCW_GR_Enable){
		init_dcw_gr_step(&un);
	}
	
	if(cur)
	{
		g_free_list = cur->next;
		rt_list_entry(cur, STEP_NODE, list)->un = un;
	}
	else
	{
		return -2;//错误处理
	}
	
	/* 定位 */
	for(node = cs99xx_list.head;NULL != node;node = node->next)
	{
		if(rt_list_entry(node, STEP_NODE, list)->un.com.step == step)
		{
			break;
		}
	}
	
	cur->prev = node;
	cur->next = node->next;
	
	if(node->next)
	{
		node->next->prev = cur;
	}
	else
	{
		cs99xx_list.tail = cur;
	}
	
	node->next = cur;
	
	for(node=cur->next;node;node = node->next)
	{
		rt_list_entry(node, STEP_NODE, list)->un.com.step++;//将新节点后的节点的步数全部加1
	}
	
	cs99xx_list.size++;//总步数加1
	return 0;
}

rt_int8_t swap_step(rt_uint8_t one, rt_uint8_t two)
{//交换
	struct rt_list_node *node1 = NULL;
	struct rt_list_node *node2 = NULL;
	UN_STR temp_un;
	
	if((one>cs99xx_list.size) || (two>cs99xx_list.size))
	{
		return -1;
	}
	
	/* 定位 */
	for(node1 = cs99xx_list.head; NULL != node1; node1 = node1->next)
	{
		if(rt_list_entry(node1, STEP_NODE, list)->un.com.step == one)
		{
			break;
		}
	}
	for(node2 = cs99xx_list.head; NULL != node2; node2 = node2->next)
	{
		if(rt_list_entry(node2, STEP_NODE, list)->un.com.step == two)
		{
			break;
		}
	}
	
	/* 交换 */
	rt_list_entry(node1, STEP_NODE, list)->un.com.step = two;
	rt_list_entry(node2, STEP_NODE, list)->un.com.step = one;
	
	temp_un = rt_list_entry(node1, STEP_NODE, list)->un;
	rt_list_entry(node1, STEP_NODE, list)->un = rt_list_entry(node2, STEP_NODE, list)->un;
	rt_list_entry(node2, STEP_NODE, list)->un = temp_un;
	
	return 0;
}

struct rt_list_node *position_step(rt_uint8_t pos)
{
	struct rt_list_node *node = NULL;
	
	/* 查找 */
	for(node = cs99xx_list.head; NULL != node; node = node->next)
	{
		if(rt_list_entry(node, STEP_NODE, list)->un.com.step == pos)
		{
			break;
		}
	}
	
	return node;
}


static rt_int16_t count_offset(const rt_uint16_t step)
{
	rt_int16_t i;
	
	/* 第一页 */
	if(step <= PER_P_STEPS)
	{
		i = 1;
	}
	/* 第二页 */
	else if(step <= 2*PER_P_STEPS)
	{
		i = 2;
	}
	/* 第三页 */
	else if(step <= 3*PER_P_STEPS)
	{
		i = 3;
	}
	/* 第四页 */
	else if(step <= 4*PER_P_STEPS)
	{
		i = 4;
	}
	else
	{
		return -1;/* 数据非法 */
	}
	
	return i;
}
rt_int8_t save_steps_to_flash(const rt_uint8_t n)
{/* n文件编号 */
	struct rt_list_node *node = NULL;
	
	rt_int16_t i = 0;/* 页内偏移 */
	rt_int16_t j = 0;/* 页偏移 */
	rt_int16_t p = 0;/* 页数 */
	rt_int16_t res = 0;/* 结果 */
	rt_int16_t size = file_info[n].totalstep;
	
	p = count_offset(size);
	
	if(p == -1)
	{
		return -1;
	}
	FLASH_CS_SET(1);
	for(node = cs99xx_list.head, i=0; NULL != node; node = node->next, i++)
	{
		if(i == PER_P_STEPS)
		{
			i = 0;
			j++;
		}
		
		res = sf_WriteBuffer((uint8_t*)&rt_list_entry(node, STEP_NODE, list)->un,
				GROUP_BASE + n*GROUP_OFFSET + j*F_PAGE_SIZE + i*STEP_OFFSET, STEP_OFFSET);
		if(res == 0)
		{
			return -1;/* 失败 */
		}
	}
	
	return 0;
}

rt_int8_t read_flash_to_list(const rt_uint8_t n)
{/* n文件编号 */
	struct rt_list_node *node = NULL;
	UN_STR temp;
	rt_int16_t i = 0;/* 页内偏移 */
	rt_int16_t j = 0;/* 页偏移 */
	rt_int16_t p = 0;/* 页数 */
	rt_int16_t size = file_info[n].totalstep;
	
	if(size > TOTALSTEP_NUM)
	{
		return -1;/* 数据非法 */
	}
	
	p = count_offset(size);
	
	if(p == -1)
	{
		return -1;
	}
	
	for(i = 0; i < size-1; i++)
	{
		insert_after(i+1);
	}
	FLASH_CS_SET(1);
	for(node = cs99xx_list.head, i=0; (NULL != node) && (i < size); node = node->next, i++)
	{
		if(i == PER_P_STEPS)
		{
			i = 0;
			if(j < p)
			{
				j++;
			}
		}
		
		sf_ReadBuffer((rt_uint8_t*)&temp,
				GROUP_BASE + n*GROUP_OFFSET + j*F_PAGE_SIZE + STEP_OFFSET*i, STEP_OFFSET);
		rt_list_entry(node, STEP_NODE, list)->un = temp;
	}
	
	/* 检查数据 */
	for(node = cs99xx_list.head, i=0; (NULL != node) && (i < cs99xx_list.size); node = node->next, i++)
	{
		if(rt_list_entry(node, STEP_NODE, list)->un.com.step != (i+1))
		{
			return -1;
		}
		
		if(rt_list_entry(node, STEP_NODE, list)->un.com.mode>DLA)
		{
			return -1;
		}
	}
	
	return 0; 
}

rt_int16_t save_step_to_flash(rt_uint8_t n, rt_uint8_t step, UN_STR *pun)
{/* n文件编号  step 步骤号 p保存步数据 */
	rt_uint8_t p = 0;/* 在第几页 */
	rt_int16_t res = 0;
	rt_int16_t i = 0;/* 页内偏移 */
	rt_int16_t j = 0;/* 页偏移 */
	
	if(step < 1)
	{
		return -1;
	}
	
	p = count_offset(step);
	j = p - 1;
	
	i = step%PER_P_STEPS;
	if(i == 0)
	{
		i = PER_P_STEPS;
	}
	FLASH_CS_SET(1);
	res = sf_WriteBuffer((uint8_t*)pun,
			GROUP_BASE + n*GROUP_OFFSET + F_PAGE_SIZE*j + (i-1)*STEP_OFFSET, STEP_OFFSET);
	
	if(res)
	{
		return 0;/* 成功 */
	}
	return -1;/* 失败 */
}

rt_int32_t read_flash_to_step(rt_uint8_t n, rt_uint8_t step, UN_STR *pun)
{
	rt_uint8_t p = 0;/* 在第几页 */
	rt_int16_t i = 0;/* 页内偏移 */
	rt_int16_t j = 0;/* 页偏移 */
	
	if(step < 1)
	{
		return -1;
	}
	
	p = count_offset(step);
	j = p - 1;/* 计算页偏移 */
	
	i = step%PER_P_STEPS;/* 计算页内偏移 */
	if(i == 0)
	{
		i = PER_P_STEPS;
	}
	FLASH_CS_SET(1);
	sf_ReadBuffer((uint8_t*)pun,
			GROUP_BASE + n*GROUP_OFFSET + F_PAGE_SIZE*j + (i-1)*STEP_OFFSET, STEP_OFFSET);
	
	return 0;
}

rt_int32_t save_files_to_flash(void)
{
// 	rt_int32_t p = FILE_NUM/PER_P_FILES + (FILE_NUM%PER_P_FILES==0?0:1);/* 页数 */
// 	rt_int32_t i = 0;/* 页偏移 */
// 	rt_int32_t res = 0;
// 	rt_int32_t cur_p_num = 0;/* 当前页的文件数 */
// 	
// 	FLASH_CS_SET(1);
// 	for(i = 0; i < p; i++)
// 	{
// 		if((i+1) < p)
// 		{
// 			cur_p_num = PER_P_FILES;
// 		}
// 		else if((i+1) == p)
// 		{
// 			cur_p_num = FILE_NUM%PER_P_FILES;
// 		}
// 		
// 		res = sf_WriteBuffer((uint8_t*)&file_info[i*PER_P_FILES], FILE_BASE + F_PAGE_SIZE*i, cur_p_num*FILE_OFFSET);
// 		if(res == 0)
// 		{
// 			return -1;
// 		}
// 	}
	rt_int8_t NumOfPage = FILE_NUM/PER_P_FILES;
	rt_int8_t NumOfSingle = FILE_NUM%PER_P_FILES;
	rt_int8_t i;
	FLASH_CS_SET(1);
	for(i=0;i<NumOfPage;i++)
		sf_WriteBuffer((uint8_t*)&file_info[i*PER_P_FILES], FILE_BASE + F_PAGE_SIZE*i, FILE_OFFSET*PER_P_FILES);
	if(NumOfSingle != 0)
		sf_WriteBuffer((uint8_t*)&file_info[i*PER_P_FILES], FILE_BASE + F_PAGE_SIZE*i, FILE_OFFSET*NumOfSingle);
	return 0;
}
rt_int32_t save_file_to_flash(rt_uint8_t n)
{
// 	rt_int32_t p = n/PER_P_FILES + (n%PER_P_FILES==0?0:1);/* 页数 */
// 	rt_int32_t i = 0;/* 页内偏移 */
// 	rt_int32_t j = 0;/* 页偏移 */
// 	rt_int32_t res = 0;
// 	
// 	if(n == 0)
// 	{
// 		j = p;
// 	}
// 	else
// 	{
// 		j = p - 1;
// 	}
// 	
// 	i = n%PER_P_FILES;
// 	if(i == 0)
// 	{
// 		i = PER_P_FILES;
// 	}
// 	FLASH_CS_SET(1);
// 	res = sf_WriteBuffer((uint8_t*)&file_info[n], FILE_BASE + F_PAGE_SIZE*j + FILE_OFFSET*i, FILE_OFFSET);
// 	
// 	if(res == 0)
// 	{
// 		return -1;
// 	}
	FLASH_CS_SET(1);
	sf_WriteBuffer((uint8_t*)&file_info[n], 
			FILE_BASE + (n/PER_P_FILES)*4096 + FILE_OFFSET*(n%PER_P_FILES), 
			FILE_OFFSET);
	return 0;
}


rt_int32_t read_flash_to_files(void)
{
// 	rt_int32_t p = (FILE_NUM/PER_P_FILES + (FILE_NUM%PER_P_FILES==0?0:1));/* 页数 */
// 	rt_int32_t i = 0;/* 页偏移 */
// 	rt_int32_t cur_p_num = 0;/* 当前页的文件数 */
// 	
// 	FLASH_CS_SET(1);
// 	for(i = 0; i < p; i++)
// 	{
// 		if((i+1) < p)
// 		{
// 			cur_p_num = PER_P_FILES;
// 		}
// 		else if((i+1) == p)
// 		{
// 			cur_p_num = FILE_NUM%PER_P_FILES;
// 		}
// 		
// 		sf_ReadBuffer((uint8_t*)&file_info[i*PER_P_FILES], FILE_BASE + F_PAGE_SIZE*i, cur_p_num*FILE_OFFSET);
// 	}
// 	return 0;
	rt_int8_t NumOfPage = FILE_NUM/PER_P_FILES;
	rt_int8_t NumOfSingle = FILE_NUM%PER_P_FILES;
	rt_int8_t i;
	for(i=0;i<NumOfPage;i++)
		sf_ReadBuffer((uint8_t*)&file_info[i*PER_P_FILES], FILE_BASE + F_PAGE_SIZE*i, FILE_OFFSET*PER_P_FILES);
	if(NumOfSingle != 0)
		sf_ReadBuffer((uint8_t*)&file_info[i*PER_P_FILES], FILE_BASE + F_PAGE_SIZE*i, FILE_OFFSET*NumOfSingle);
	return 0;
}

rt_int32_t read_file_to_flash(rt_uint8_t n)
{
// 	rt_int32_t p = (n+1)/PER_P_FILES + ((n+1)%PER_P_FILES==0?0:1);/* 页数 */
// 	rt_int32_t i = 0;/* 页内偏移 */
// 	rt_int32_t j = 0;/* 页偏移 */
// 	rt_int32_t res = 0;
// 	
// 	j = p - 1;
// 	
// 	i = n%PER_P_FILES;
// 	if(i == 0)
// 	{
// 		i = PER_P_FILES;
// 	}
// 	
// 	FLASH_CS_SET(1);
// 	sf_ReadBuffer((uint8_t*)&file_info[n], FILE_BASE + F_PAGE_SIZE*j + FILE_OFFSET*i, FILE_OFFSET);
// 	
// 	if(res == 0)
// 	{
// 		return -1;
// 	}
	
	FLASH_CS_SET(1);
	sf_ReadBuffer((uint8_t*)&file_info[n], 
			FILE_BASE + (n/PER_P_FILES)*4096 + FILE_OFFSET*(n%PER_P_FILES), 
			FILE_OFFSET);
	return 0;
}
