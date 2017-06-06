#ifndef __SELF_TEST_H
#define __SELF_TEST_H

#if defined(__cplusplus)
    extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*包含的头文件*/
#include <stdint.h>
#include "stm32f4xx.h"
			
			

		
#define  LOOP_SELFTEST_VREF_1V        (0)
#define  LOOP_SELFTEST_SINE_OPEN      (1)			
#define  LOOP_SELFTEST_SINE_CLOSE     (2)			
			
			

			
			
extern  uint8_t VREF_SelfTest(uint8_t ch);
extern  uint8_t Test_Loop_Selftest(uint8_t testway);




/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/
#endif
