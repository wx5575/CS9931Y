#ifndef __ZWD414B_TOOLS_H
#define __ZWD414B_TOOLS_H

#include "stm32f4xx_conf.h"




extern void Sort(uint16_t data[], uint8_t DataNum);
extern void FloatTochar(uint8_t * char_data, float Float_data);
extern float CharToFloat(uint8_t *char_data);

extern uint16_t CharToUint16(uint8_t *char_data);
extern void Uint16ToChar(uint8_t * char_data, uint16_t uint16_data);


#endif
