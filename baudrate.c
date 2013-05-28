#include "baudrate.h"

static ticks_t _default_ticks(){return 0; }
static u08 _default_poll(){return 0; }

static volatile u32		_bd_min_ticks = 0xffffffff;
static volatile u32 	_bd_prev_ticks; 
static volatile u08		_bd_prev_poll = 0; 

static bd_ticks_proc bd_ticks = &_default_ticks;
static bd_poll_proc bd_poll = &_default_poll; 

extern u32 BD_TICKS_PER_SEC; 

void bd_init(bd_ticks_proc ticks, bd_poll_proc poll) {
	if(ticks) bd_ticks = ticks;
	if(poll) bd_poll = poll;
}

void bd_state_changed(){
	ticks_t t = bd_ticks(); 
	if(!_bd_prev_ticks) {
		_bd_prev_ticks = t; 
		return; 
	}
	_bd_min_ticks = ((t - _bd_prev_ticks) < _bd_min_ticks)?(t - _bd_prev_ticks):_bd_min_ticks;
	_bd_prev_ticks = t; 
}

u32 bd_measure(){
	u32 time = BD_TICKS_PER_SEC / 200;
	u32 c = 0;
	do {
		u08 b = bd_poll();
		if((!b && _bd_prev_poll) || (b && !_bd_prev_poll)){
			bd_state_changed();
		}
		_bd_prev_poll = b;
		c++;
	} while(c < time);
	u32 baud = bd_baudrate(); 
	bd_reset();
	return baud;
}

u32 bd_baudrate(){
	if(!_bd_min_ticks) return 0;
	return BD_TICKS_PER_SEC / _bd_min_ticks;
}

void bd_reset(){
	_bd_min_ticks = 0xffffffff;
	_bd_prev_ticks = 0; 
}
