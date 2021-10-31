
#ifndef TIMER_H
#define TIMER_H

#include "common/math/fixed16.h"

//-----------------------------------------------------------------------------
typedef enum timer_mode_e
{
	TIMER_MODE_ONCE,
	TIMER_MODE_FLIP_FLOP,
	TIMER_MODE_LOOP,

} timer_mode_t;

//-----------------------------------------------------------------------------
typedef struct perf_timer_s
{
	uint32_t start;
	uint32_t duration;
	timer_mode_t mode;

} game_timer_t;


//-----------------------------------------------------------------------------
game_timer_t timer_start(uint16_t duration, timer_mode_t mode);

//-----------------------------------------------------------------------------
fixed16_t timer_progress(game_timer_t timer);

//-----------------------------------------------------------------------------
uint32_t timer_get_frame_count(void);

//-----------------------------------------------------------------------------
int8_t timer_expired(game_timer_t timer);


#endif
