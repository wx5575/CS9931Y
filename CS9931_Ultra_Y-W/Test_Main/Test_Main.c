/*包含的头文件*/
#include "driver.h"
#include <stdint.h>
#include "stm32f4xx.h"


enum current_gear_type
{
	I3uA = 0,
	I30uA,
	I300uA,
	I3mA,
	I30mA,
	I100mA,
};

struct step_lc_t
{
	uint8_t step;
	uint8_t mode;
	uint16_t outvol; // output voltage.
	enum current_gear_type curgear; // current gear.
	uint16_t curhigh; // current high limit.
	uint16_t curlow; // current low limit.

	uint16_t ramptime; // ramp time.
	uint16_t testtime; // test time.
	uint16_t pausetime; // pause time.
	uint16_t outfreq; // out frequency.
	
	uint8_t NorLphase; // N or L phase.
	uint8_t curdetection; // current detection.
	
	uint8_t steppass; // step pass.
	uint8_t stepcontinuation; // step continuation.
	uint8_t failstop; // test fail stop.
	uint8_t scanport; // scanning ports.
	
	uint16_t MDvol; // MD voltage.
	uint8_t	MDnetwork; // MD network.
	
	/* 医用才有 */
	uint16_t	assistvol;//辅助电压：关、30--300V
	uint8_t	singlefault;//单一故障：开、关
	uint8_t	MDpostion;//MD位置：MD1、MD2、MD3、MD4
	uint8_t	MDlow;//MD接地、浮地
	uint8_t	SW3;//开、关
	uint8_t	SW7;
	uint8_t	SW8;
	uint8_t	SW10;
	uint8_t	SW11;
	uint8_t	SW12;
};



uint8_t LC_Test(struct step_lc_t *lc_test_para)
{
	lc_test_para->step = 0;
	return 0;
}





