
#include "jj98.h"
// #include "Usart1.h"
#include "Communication.h"

jj98_FRAME send_jj98_frame;//发送帧
jj98_FRAME rece_jj98_frame;//接收帧

typedef struct{
    uint16_t data;//数据
    uint8_t available;//数据可以使用标记
}DATA_INF;

DATA_INF vol;//电压

/* 设置变频电源的输出电压 */
void jj98_set_vol(uint16_t vol, uint8_t decs)
{
	send_jj98_frame.direction = DIR_MASTER_W;
	send_jj98_frame.val.value = vol;
	send_jj98_frame.ctrl.bits.rw = CTRL_MASTER_WRITE;
	send_jj98_frame.ctrl.bits.vol  = 1;
	send_jj98_frame.ctrl.bits.freq  = 0;
	send_jj98_frame.ctrl.bits.cur  = 0;
	send_jj98_frame.ctrl.bits.pw  = 0;
	send_jj98_frame.ctrl.bits.decs_num  = decs;
	jj98_stop_test();//使用stop来设置参数
}
/* 获取变频电源的电压值 */
void jj98_get_vol(uint8_t decs)
{
	send_jj98_frame.direction = DIR_MASTER_R;
	send_jj98_frame.val.value = 0;
	send_jj98_frame.ctrl.bits.rw = CTRL_MASTER_READ;
	send_jj98_frame.ctrl.bits.vol  = 1;
	send_jj98_frame.ctrl.bits.freq  = 0;
	send_jj98_frame.ctrl.bits.cur  = 0;
	send_jj98_frame.ctrl.bits.pw  = 0;
	send_jj98_frame.ctrl.bits.decs_num  = decs;
	jj98_start_test();//使用start来设置参数
}
/* 设置频率 */
void jj98_set_freq(uint16_t freq, uint8_t decs)
{
	send_jj98_frame.direction = DIR_MASTER_W;
	send_jj98_frame.val.value = freq;
	send_jj98_frame.ctrl.bits.rw = CTRL_MASTER_WRITE;
	send_jj98_frame.ctrl.bits.vol  = 0;
	send_jj98_frame.ctrl.bits.freq  = 1;
	send_jj98_frame.ctrl.bits.decs_num  = decs;
	jj98_stop_test();//使用stop来设置参数
}
/* 启动测试 */
void jj98_start_test(void)
{
	send_jj98_frame.ctrl.bits.ctrl = CTRL_SLAVE_START;
	jj98_send_frame_to_slove();
}
/* 停止测试 */
void jj98_stop_test(void)
{
	send_jj98_frame.ctrl.bits.ctrl = CTRL_SLAVE_STOP;
	jj98_send_frame_to_slove();
}
static void jj98_Delay_ms(unsigned int dly_ms)
{
  unsigned int dly_i;
  while(dly_ms--)
    for(dly_i=0;dly_i<18714;dly_i++);
}
/* 向变频电源发送数据 */
void jj98_send_frame_to_slove(void)
{
	uint32_t time_out = 0xfffff;
	
	if(get_usart2_busy_st() == 1)
	{
		return;
	}
	
	set_usart2_busy_st();
	
	rt_uart_write(&send_jj98_frame, sizeof(send_jj98_frame));
	
	/* 等待发送完成 */
	while(1)
	{		
		if(get_usart2_send_st() || --time_out == 0)
		{
			break;
		}
	}
	
	reset_usart2_busy_st();
	
	jj98_Delay_ms(500);
}


void jj98_comm_analysis(uint8_t data)
{
    static uint32_t data_count;
    static uint8_t f_buf[10];
    jj98_FRAME *frame = (void*)f_buf;
    
    if(data == 0x18)
    {
        data_count = 0;
        memset(f_buf, 0, sizeof(f_buf));
    }
    
    f_buf[data_count++] = data;
    
    if(data_count == 4)
    {
        vol.data = frame->val.value;
        vol.available = 1;
        data_count = 0;
    }
}



