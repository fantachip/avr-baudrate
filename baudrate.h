#ifndef _BAUDRATE_H_
#define _BAUDRATE_H_

/*
BAUD RATE MEASUREMENT

Interface for measuring baud rate: 
- define function that returns current clock count as u32
- (optional) define a function that will poll a pin and return 1 or 0

Usage: 
- call bd_init(&ticks_proc, &poll_proc);
- if not using poll, call bd_state_changed() as soon as bit change is detected
- after a set interval call bd_baudrate() which will return baudrate calculated 
	based on the timing of the shortest bit. 
- call bd_reset() to reset the baudrate counter. 
*/ 

#include <stdlib.h>
#include <stdio.h>

#ifndef u32
	#define u32 uint32_t 
#endif
#ifndef u08
	#define u08 uint8_t
#endif

typedef u32 ticks_t; 
typedef ticks_t (*bd_ticks_proc)();
typedef u08 (*bd_poll_proc)();

void bd_init(bd_ticks_proc ticks, bd_poll_proc poll);
void bd_state_changed();
u32 bd_baudrate();
void bd_reset();
u32 bd_measure();

#endif
