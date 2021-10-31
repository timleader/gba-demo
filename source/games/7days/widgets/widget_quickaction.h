
#ifndef WIDGET_QUICKACTION_H
#define WIDGET_QUICKACTION_H

#include "common/utils/timer.h"
#include "common/graphics/overlay.h"

#include "games/7days/world.h"

/*
	STATE:
		- IDLE
		- TRANSITION_IN
		- READY					//	How do we switch ? 
		- TRANSITION_OUT

	DATA:
		interaction_point	?? 
		action_callback		?? 

*/

typedef struct widget_quickaction_state_s
{
	uint8_t panel_id;

	uint8_t mode;	//	enum this 
	/*
		0: Idle
		1: Transition In
		2: Ready
		3: Transition Out
	*/

	uint8_t reserved[2];

	game_timer_t timer;

	interaction_point_ptr current_interaction;
	interaction_point_ptr next_interaction;

} widget_quickaction_state_t;

typedef widget_quickaction_state_t* widget_quickaction_state_ptr;


void widget_quickaction_schedule_interaction(widget_quickaction_state_ptr state, interaction_point_t* interaction);

void widget_quickaction_snap_to_ready(widget_quickaction_state_ptr state);

void widget_quickaction_clear(widget_quickaction_state_ptr state);


void widget_quickaction_initialize(widget_quickaction_state_ptr state);

void widget_quickaction_shutdown(widget_quickaction_state_ptr state);

void widget_quickaction_update(widget_quickaction_state_ptr state);


#endif
