#ifndef __PLC__H__
#define __PLC__H__


#include "stdint.h"


#define   PLC_TEST_TESTING     (1)
#define   PLC_TEST_STOP        (0)












void PLC_Interface_Init(void);
void PLC_Testing_Out(uint8_t state);
void PLC_Pass_Out(uint8_t state);
void PLC_Fail_Out(uint8_t state);


#endif
