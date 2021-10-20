
#include "sequence.h"

#include "common/debug/debug.h"

//	initialize that grabs sequence store from resources !! 

typedef enum sequence_builtin_event_type_e
{
	event_type_nop		= 0x1D,
	event_type_store	= 0x1E,
	event_type_branch	= 0x1F,

} sequence_builtin_event_type_t;


void sequence_reset(sequence_player_ptr player)
{ }

void sequence_schedule(sequence_player_ptr player, int16_t sequence_id)	//	what happens if we schedule multiple ?? do they queue ??? 
{
	sequence_player_state_persistent_ptr player_state = player->persistent;

	//	do something if we get a sequence_id of < 0

	if (player_state->scheduled_event >= 0)
	{
		debug_printf(DEBUG_LOG_WARN, 
			"sequence::schedule %i, replacing previously scheduled sequence event %i", 
			sequence_id, player_state->scheduled_event);
	}

	player_state->scheduled_event = sequence_id;

	debug_printf(DEBUG_LOG_DEBUG, "sequence::schedule %i", sequence_id);
}

//	schedule custom event func ( )

void sequence_update(sequence_player_ptr player)	
{
	sequence_player_state_persistent_ptr player_state = player->persistent;

	if (player->state == PLAYER_STATE_IDLE)
	{
		if (player_state->scheduled_event >= 0)
		{
			player_state->current_event = player_state->scheduled_event;
			player_state->scheduled_event = -1;
			player->state = PLAYER_STATE_PLAYING;
		}
	}

	if (player->state == PLAYER_STATE_WAITING &&
		*player->completion_flag)
	{
		player->completion_flag = NULL;
		if (player_state->current_event >= 0)
		{
			player->state = PLAYER_STATE_PLAYING;
		}
		else
		{
			player->state = PLAYER_STATE_IDLE;
		}
	}

	if (player->state == PLAYER_STATE_PLAYING)
	{
		while (player_state->current_event >= 0)
		{
			sequence_event_t event = player->store->events[player_state->current_event];
			uint8_t event_type = SEQUENCE_EVENT_TYPE(event);
			sequence_completion_mode_t completion = 0;

			//	builtin event_types, for persistence and branching 
			switch (event_type)
			{
				case event_type_branch:
				{
					uint32_t extra_data = SEQUENCE_EVENT_EXTRA(event);
					uint32_t bit_idx = extra_data & 0x7F;
					int32_t next_event = 0;

					if (player->persistent->state[(bit_idx >> 5)] & 1 << (bit_idx & 0x1F))
					{
						next_event = SEQUENCE_EVENT_NEXT(event);
					}
					else
					{
						next_event = extra_data >> 8;
					}

					player_state->current_event += next_event;
					if (next_event == 0)
						player_state->current_event = -1;

					completion = SEQUENCE_COMPLETION_IMMEDIATE;
					break;
				}
				case event_type_store:
				{
					uint32_t bit_idx = SEQUENCE_EVENT_EXTRA(event) & 0x7F;
					player->persistent->state[(bit_idx >> 5)] |= 1 << (bit_idx & 0x1F);

					player_state->current_event += SEQUENCE_EVENT_NEXT(event);
					if (SEQUENCE_EVENT_NEXT(event) == 0)
						player_state->current_event = -1;

					completion = SEQUENCE_COMPLETION_IMMEDIATE;
					break;
				}
				case event_type_nop:
				{
					player_state->current_event += SEQUENCE_EVENT_NEXT(event);
					if (SEQUENCE_EVENT_NEXT(event) == 0)
						player_state->current_event = -1;

					completion = SEQUENCE_COMPLETION_NEXT_TICK;
					break;
				}
				default: // user defined event_types 
				{
					//	need to take the signed bit for negative next event ... 
					player_state->current_event += SEQUENCE_EVENT_NEXT(event);
					if (SEQUENCE_EVENT_NEXT(event) == 0)
						player_state->current_event = -1;

					completion = player->event_handlers[event_type](player, event);
					break;
				}
			}
			
			if (completion == SEQUENCE_COMPLETION_IMMEDIATE)			//	immediately move on to next event
			{
				if (player_state->current_event < 0)
				{
					player->state = PLAYER_STATE_IDLE;
					break;
				}
			}
			else if (completion == SEQUENCE_COMPLETION_NEXT_TICK)		//	wait until next update to move on to next event
			{
				if (player_state->current_event < 0)
				{
					player->state = PLAYER_STATE_IDLE;
				}
				else
				{
					player->state = PLAYER_STATE_PLAYING;	//	for events that push states
				}
				break;
			}
			else if (completion == SEQUENCE_COMPLETION_WAIT_ON_FLAG)		//	wait on internal timer, before moving onto next event
			{
				player->state = PLAYER_STATE_WAITING;
				break;
			}
		}
	}
}


/*
	Supported Use Cases:
		+ Pre-Rendered Cutscene
		+ Realtime Rendered Cutscene
		+ Dialogue

	Unsupported Use Cases:
		+ NPC Logic <<-- coded in C
			+ sequence could be extended to support NPC state machine transitions


	Stuff like view change wants to happen immediately ... or does it not matter if this takes a frame ?

*/

