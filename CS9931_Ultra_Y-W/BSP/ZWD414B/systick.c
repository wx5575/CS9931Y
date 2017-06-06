#include "systick.h"

extern uint8_t receive_timeout_counter;

/*
    ModBus 通信协议中接受一组数据的间隔时间是3.5个计数时间，
        采用9600的波特率所以间隔时间为4mS左右
        这里采用5mS为标志，每间隔5mS表示一组数据接收完成

        每次接收到一个字符的数据需要将ModBus定时器进行REFRESH调用
        在1mS的定时器中对其进行N_REFRESH的调用

*/
__IO static TIMER_STATUS ModBusTimerStatus;

void  Timer_ModBus(REFRESH_STATUS Refrash)
{
    static int8_t time_counter;    

    if (receive_timeout_counter > 1)   //未接收到数据时，减减
		receive_timeout_counter--;

    if(Refrash == N_REFRESH )  { 
    
        if(ModBusTimerStatus == TIMER_EMPTY) {
            return ;   
        }
           
        if(time_counter < 5){
            time_counter ++;
            ModBusTimerStatus = TIMER_CONTINUE;
        }
        else {
            ModBusTimerStatus = TIMER_OVER ;   //
// 			OSMutexPost (USART3Mutex);
// 			USART3Mutex_need_to_post = 1;
        }
    }
    else {
        time_counter = 0;
        ModBusTimerStatus = TIMER_CONTINUE;
    }
	
	
}

void set_ModBus_timer_status(TIMER_STATUS state)
{
    ModBusTimerStatus = state; 
}

TIMER_STATUS get_ModBus_timer_status(void)
{
    if(ModBusTimerStatus == TIMER_OVER) {
        ModBusTimerStatus = TIMER_EMPTY;

        return TIMER_OVER;
    }

    return ModBusTimerStatus;
}




void SysTick_Dispose(void)
{
    Timer_ModBus(N_REFRESH);  //ZWD414B用

}


