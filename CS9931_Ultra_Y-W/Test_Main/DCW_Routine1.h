#ifndef  __ACW_ROUTINE_H__
#define  __ACW_ROUTINE_H__

/*包含的头文件*/
#include "driver.h"
#include <stdint.h>
#include "stm32f4xx.h"



enum current_gear_type
{
	I3uA,
	I30uA,
	I300uA,
	I3mA,
	I30mA,
	I100mA,
};


struct step_dcw_t
{
	uint8_t step;
	uint8_t mode;
	uint16_t outvol; // output voltage.
	enum current_gear_type curgear; // current gear.
	uint16_t curhigh; // current high limit.
	uint16_t curlow; // current low limit.
	uint16_t rmscur; // RMS current.
	uint16_t startvol; // start voltage.
	uint16_t waittime; // wait time.
	uint16_t ramptime; // ramp time.
	uint16_t testtime; // test time.
	uint16_t downtime; // down time.
	uint16_t pausetime; // pause time.
	uint16_t arc; // msb D15-D12 is grade and D11-D0 is current  modele.
	uint16_t outfreq; // out frequency.
	uint8_t steppass; // step pass.
	uint8_t stepcontinuation; // step continuation.
	uint8_t failstop; // test fail stop.
	uint8_t scanport; // scanning ports.
};



void DCWModeTestEnvironmentEnter(struct step_dcw_t *acw_test_para);
void DCWModeTestEnvironmentExit(void);

#endif
