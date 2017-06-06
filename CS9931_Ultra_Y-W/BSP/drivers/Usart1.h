#ifndef    __USART1_H__
#define    __USART1_H__


#define     BUFFER_SIZE     (100)


void Usart1_init(void);
void Usart_SendData(uint8_t send_data[] , uint8_t num);
void usart2_send_data(uint8_t send_data[] , uint8_t num);
void usart2_init(uint32_t ulBaudRate);

#endif
