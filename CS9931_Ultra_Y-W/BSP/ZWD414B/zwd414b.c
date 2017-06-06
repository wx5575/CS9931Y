#include "zwd414b.h"
#include "string.h"
// uint8_t neednot_to_wait;

#define RECEIVE_DATA_BUFFER_SIZE        (100)


/*************************************************
接收数据处理
**************************************************/
static uint8_t GetReceiveData(uint8_t *receive_data);
static void Zwd414B_Receive_data_Dispsoe( 
              ModBus_Receive_Inf * ModBus_Receive_data , Zwd414b_Receive_Inf * Zwd414b_Receive_data);


extern void ModBus_Write_TestCmd(MEMORY_ADDRESS test_cmd , uint8_t addr_len);
extern void ModBus_Write_SetCmd(MEMORY_ADDRESS test_cmd , uint8_t addr_len , uint8_t param_len , uint8_t *param);
uint8_t this_target_address ;
uint8_t receive_timeout_counter;


Zwd414b_Receive_Inf Zwd414b_Receive_data;

void Zwd414b_Init(void)
{
    this_target_address = 1; 
    
    ModBus_Init(); 
}


extern void Test_Dispsoe(void);

void Receive_data_Dispose(void)
{
    uint8_t data_num = 0;
    uint8_t  receive_data[RECEIVE_DATA_BUFFER_SIZE] ;
    ModBus_Receive_Inf ModBus_Receive_data;
	
	memset (receive_data, 0, sizeof (receive_data));
	memset (&ModBus_Receive_data, 0, sizeof (ModBus_Receive_data));
    
    /*判断ModBus接收数据定时器是否溢出*/
    if(get_ModBus_timer_status() == TIMER_OVER ) {
		receive_timeout_counter = 0;	//关闭计数器
        /*读取数据，并且返回数据的个数*/
        data_num = GetReceiveData(receive_data); 
// 		if (data_num)
// 			neednot_to_wait = 1;
        if(ModBus_Receive_data_Dispose(data_num ,receive_data, &ModBus_Receive_data) != ERROR) {
            Zwd414B_Receive_data_Dispsoe(&ModBus_Receive_data , &Zwd414b_Receive_data);
                
            /*数据读取正常 , 允许发送下一个队列的数据*/
                  
                
            Test_Dispsoe();
            send_msg.queue_front ++;
        }
        /* 将modbus至于空闲状态, 允许发送数据
        如果数据错误则queue_front 不移动，
        如果数据正确queue_front后移一位*/
        send_msg.ModBus_Status = MODBUS_EMPTY;
    }
	else if (send_msg.ModBus_Status == MODBUS_BUSY)
	{
		if (receive_timeout_counter == 1) //计数器超时
		{
			receive_timeout_counter = 0;//关闭计数器
			send_msg.ModBus_Status = MODBUS_EMPTY;
		}
	}
}

/*将Uart1Receive_Data中的数据写入到receive_data*/
extern char Usart1Receive_Data[] ;
extern uint8_t ReceiveDataNum;
static uint8_t GetReceiveData(uint8_t *receive_data)
{
    uint8_t i;
    
    for(i = 0; i < ReceiveDataNum; i++ ) {
        receive_data[i] = Usart1Receive_Data[i];
    }   

    ReceiveDataNum = 0;

    return i;
}

static void Write_Data_Inf(Zwd414b_Receive_Inf * Zwd414b_Receive_data , float * param_float , 
            uint16_t * param_int16 , uint8_t param_num)
{
    uint8_t left_byte;
    uint16_t memory_addr ;
    
    left_byte = param_num;
    memory_addr = (send_msg.Send_Queue[send_msg.queue_front][MEM_ADDR_BYTE_HIGH] <<8 )
                        |(send_msg.Send_Queue[send_msg.queue_front][MEM_ADDR_BYTE_LOW]);
   
    while(left_byte) {
        Zwd414b_Receive_data->read = F;
        switch (memory_addr) {
            case VOLTAGE:
                Zwd414b_Receive_data->Voltage.data = * param_float;
                Zwd414b_Receive_data->Voltage.read_flag =  F; 
                memory_addr +=2;
                param_float ++;  
            break;

            case CURRENT:
                Zwd414b_Receive_data->Current.data = * param_float;
                Zwd414b_Receive_data->Current.read_flag =  F; 
                memory_addr +=2;
                param_float ++; 
            break;

            case POWER:
                Zwd414b_Receive_data->Power.data = * param_float;
                Zwd414b_Receive_data->Power.read_flag =  F; 
                memory_addr +=2;
                param_float ++; 
            break;

            case PF:
                Zwd414b_Receive_data->PF.data = * param_float;
                Zwd414b_Receive_data->PF.read_flag =  F;
                memory_addr +=2;
                param_float ++;  
            break;

            case FREQUENCY:
                Zwd414b_Receive_data->Frequency.data = * param_float;
                Zwd414b_Receive_data->Frequency.read_flag =  F; 
                memory_addr +=2;
                param_float ++; 
            break;

            case POWER_NOT_WORK:
                Zwd414b_Receive_data->Power_not_work.data = * param_float;
                Zwd414b_Receive_data->Power_not_work.read_flag =  F;
                memory_addr +=2;
                param_float ++;  
            break;

            case POWER_WORK:
                Zwd414b_Receive_data->Power_work.data = * param_float;
                Zwd414b_Receive_data->Power_work.read_flag =  F; 
                memory_addr +=2;
                param_float ++; 
            break;

            case GRAND_TIME:
                Zwd414b_Receive_data->Grand_time.data = * param_float;
                Zwd414b_Receive_data->Grand_time.read_flag =  F; 
                memory_addr +=2;
                param_float ++; 
            break;

            case VOLTAGE_MULTIPLE:
                Zwd414b_Receive_data->Voltage_multiple.data = * param_float;
                Zwd414b_Receive_data->Voltage_multiple.read_flag =  F; 
                memory_addr +=2;
                param_float ++; 
            break;

            case CURRENT_MULTIPLE:
                Zwd414b_Receive_data->Current_multiple.data = * param_float;
                Zwd414b_Receive_data->Current_multiple.read_flag =  F; 
                memory_addr +=2;
                param_float ++; 
            break;

            case COM_ADDRESS:
                Zwd414b_Receive_data->Com_address.data = * param_int16;
                Zwd414b_Receive_data->Com_address.read_flag =  F; 
                memory_addr +=1;
                param_int16 ++; 
            break;

            case BAUD_RATE:
                Zwd414b_Receive_data->Buad_rate.data = * param_int16;
                Zwd414b_Receive_data->Buad_rate.read_flag =  F; 
                memory_addr +=1;
                param_int16 ++; 
            break;

            case ENERGY:
                Zwd414b_Receive_data->Energy.data = * param_int16;
                Zwd414b_Receive_data->Energy.read_flag =  F; 
                memory_addr +=1;
                param_int16 ++; 
            break;     
        
        } /*end of switch*/

        left_byte --;      

    } /*end of while*/
}


static void Zwd414B_Receive_data_Dispsoe( \
            ModBus_Receive_Inf * ModBus_Receive_data , Zwd414b_Receive_Inf * Zwd414b_Receive_data)
{
    uint8_t left_byte , param_num;
    uint16_t param_int16[8];
    float param_float[8]; 
    uint16_t memory_addr ;

    param_num = 0;
    
    if(ModBus_Receive_data->cmd == CMD_TEST) {
        /*接收到的指令为测试指令*/
        left_byte = ModBus_Receive_data->param_num;
        memory_addr = (send_msg.Send_Queue[send_msg.queue_front][MEM_ADDR_BYTE_HIGH] <<8 )
                            |(send_msg.Send_Queue[send_msg.queue_front][MEM_ADDR_BYTE_LOW]);
        if(memory_addr < COM_ADDRESS) {
            /*float型参数，每个参数占四个字节*/
			if (left_byte % 4 == 0)	//防止死循环
			{
				while(left_byte) {
					param_float[param_num] = CharToFloat(ModBus_Receive_data->param_ch + param_num*4); 
					left_byte -= 4;
					param_num ++;
				}
			}
        }
        else {
            /*uint16_t 类型参数，每个参数两个字节*/
			if (left_byte % 2 == 0)	//防止死循环
			{
				while(left_byte) {
					param_int16[param_num] = CharToUint16(ModBus_Receive_data->param_ch + param_num*2); 
					left_byte -= 2;
					param_num ++;
				}
			}
        }
        Write_Data_Inf(Zwd414b_Receive_data ,param_float , param_int16 , param_num);
    } 
}




/*************************************************
发送数据处理
**************************************************/

static uint8_t Get_Addr_len(MEMORY_ADDRESS test)
{
    uint8_t addr_len;
    if (test <= CURRENT_MULTIPLE) {  //两个字节偏移   
        addr_len = 2; 
    }  
    else {
        addr_len = 1;
    }  
    
    return addr_len;  
}

/*
    写入测试指令
*/
void Zwd414b_Test(TEST_CMD test_cmd[], uint8_t test_num)
{
    uint8_t addr_len = 0;
    uint8_t front_test = 0 ;   
    uint8_t next_test = 1;

    Sort((uint16_t *)test_cmd, test_num);


    addr_len += Get_Addr_len(test_cmd[front_test]);

    while(next_test < test_num) {
        if(test_cmd[front_test] + addr_len == test_cmd[next_test]) {
            /*地址连续 , 地址偏移增加 , next_test后移一位*/
            addr_len += Get_Addr_len(test_cmd[next_test]);
            next_test ++;
        }
        else {
            /*地址不连续 ， 写入发送队列，
            新的偏移地址next_test的偏移，首地址为next_test的地址*/
            ModBus_Write_TestCmd(test_cmd[front_test] , addr_len);

            front_test = next_test;
            addr_len = Get_Addr_len(test_cmd[next_test]);

            next_test ++;

        }
    } 
    
    ModBus_Write_TestCmd(test_cmd[front_test] , addr_len);
}
//void ModBus_Write_SetCmd_WithParam(MEMORY_ADDRESS test_cmd , uint8_t addr_len , uint8_t param_len , uint8_t *param)
void Zwd414b_Set_Voltage_multiple(float multiple)
{
    uint8_t char_data[4];

    FloatTochar(char_data, multiple);

    ModBus_Write_SetCmd(VOLTAGE_MULTIPLE ,2, 4, char_data);
}

void Zwd414b_Set_Current_multiple(float multiple)
{
    uint8_t char_data[4];

    FloatTochar(char_data, multiple);

    ModBus_Write_SetCmd(CURRENT_MULTIPLE ,2, 4, char_data);
}

void Zwd414b_Set_Energy(ENERGY_CMD energy_cmd)
{
    uint8_t char_data[2];

    Uint16ToChar(char_data, energy_cmd);

    ModBus_Write_SetCmd(ENERGY ,1, 2, char_data);
}



/*发送数据控制*/
void Zwd414b_Send_data_Dispose(void)
{
    ModBus_send_data();
}




void Test_Dispsoe(void)
{
    if(Zwd414b_Receive_data.read == F) {
        Zwd414b_Receive_data.read = T;
    }
}



uint16_t Get_ZWD414B (uint16_t ZWD414Item)
{
	uint16_t temp_value = 0;
	float f_temp = 0.0;
	//float f_ddd = 1.036;
	//f_temp = f_ddd;
	switch (ZWD414Item)
	{
		case VOLTAGE:
			f_temp = Zwd414b_Receive_data.Voltage.data * 10;
			break;

		case CURRENT:
			f_temp = Zwd414b_Receive_data.Current.data * 1000;
			break;

		case POWER:
			f_temp = Zwd414b_Receive_data.Power.data * 10; 
			break;

		case PF:
			f_temp = Zwd414b_Receive_data.PF.data * 1000; 
			break;

		case FREQUENCY:
			f_temp = Zwd414b_Receive_data.Frequency.data;
			break;

		case POWER_NOT_WORK:
			f_temp = Zwd414b_Receive_data.Power_not_work.data; 
			break;

		case POWER_WORK:
			f_temp = Zwd414b_Receive_data.Power_work.data; 
			break;

		case GRAND_TIME:
			f_temp = Zwd414b_Receive_data.Grand_time.data;
			break;

		case VOLTAGE_MULTIPLE:
			f_temp = Zwd414b_Receive_data.Voltage_multiple.data;
			break;

		case CURRENT_MULTIPLE:
			f_temp = Zwd414b_Receive_data.Current_multiple.data;   
			break;
	}
	temp_value = (uint16_t)f_temp;
	return temp_value;
}








