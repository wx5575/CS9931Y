#include "zwd414b_tools.h"

float CharToFloat(uint8_t * char_data)
{
    union {
        char ch_data[4];
        float float_data;
    }change_data;
    uint8_t i;

    for(i = 0; i< 4; i++) {
        change_data.ch_data[3-i] = char_data[i];  
    }

    return change_data.float_data;
}

void FloatTochar(uint8_t * char_data, float Float_data)
{
    union {
        char ch_data[4];
        float float_data;
    }change_data;
    uint8_t i;
   
    change_data.float_data = Float_data;
       
    for(i = 0; i < 4; i++) {
         char_data[i] = change_data.ch_data[i];  
    }
}


uint16_t CharToUint16(uint8_t * char_data)
{
    uint16_t data;

    data = (char_data[0] << 8) | char_data[1];

    return data;
}

void Uint16ToChar(uint8_t * char_data, uint16_t uint16_data)
{
    char_data[0] = (uint16_data >> 8) & 0xff;
    char_data[1] = uint16_data &0xff;
}

static void Swap(uint16_t * data1, uint16_t *data2)
{
    uint16_t temp;

    temp = *data1;
    *data1 = *data2;
    *data2 = temp;
}

void Sort(uint16_t data[], uint8_t DataNum)
{
    uint8_t data_min ,i , j;
    for(i = 0; i < DataNum - 1; i++) {
        data_min = i;
        for(j = i + 1; j < DataNum ; j++) {
            if(data[data_min] > data[j])  {
                data_min = j;
            }
        }
        if(data_min != i) {
            Swap(data+data_min , data+i); 
        }
    }
}


