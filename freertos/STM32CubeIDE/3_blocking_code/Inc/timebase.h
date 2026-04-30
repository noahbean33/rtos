/*
 * timebase.h
 *
 * 	Target Development Board: STM32 Nucleo F429ZI
 */

#ifndef TIMEBASE_H_
#define TIMEBASE_H_

#include <stdint.h>
#include "stm32f4xx.h"

void delay(uint32_t seconds);
void timebase_init(void);

#endif /* TIMEBASE_H_ */
