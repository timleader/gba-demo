
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

void sequence_schedule(sequence_player_ptr player, int8_t channel_id, int16_t sequence_id)	//	what happens if we schedule multiple ?? do they queue ??? 
{
	uint32_t* state = player->persistent_state;
	sequence_channel_persistent_t* channel = &player->persistent_channels[channel_id];

	//	do something if we get a sequence_id of < 0

	if (channel->scheduled_event >= 0)
	{
		debug_printf(DEBUG_LOG_WARN, 
			"sequence::schedule %i, replacing previously scheduled sequence event %i", 
			sequence_id, channel->scheduled_event);
	}

	channel->scheduled_event = sequence_id;

	debug_printf(DEBUG_LOG_DEBUG, "sequence::schedule %i", sequence_id);
}

//	schedule custom event func ( )

void sequence_update(sequence_player_ptr player)	
{
	uint32_t* state = player->persistent_state;
	for (int8_t channel_idx = 0; channel_idx < player->channel_count; ++channel_idx)
	{
		sequence_channel_ephermeral_t* channel_e = &player->ephermeral_channels[channel_idx];
		sequence_channel_persistent_t* channel_p = &player->persistent_channels[channel_idx];

		if (channel_e->state == PLAYER_STATE_IDLE)
		{
			if (channel_p->scheduled_event >= 0)
			{
				channel_p->current_event = channel_p->scheduled_event;
				channel_p->scheduled_event = -1;
				channel_e->state = PLAYER_STATE_PLAYING;
			}
		}

		if (channel_e->state == PLAYER_STATE_WAITING &&
			*channel_e->completion_flag)
		{
			channel_e->completion_flag = NULL;
			if (channel_p->current_event >= 0)
			{
				channel_e->state = PLAYER_STATE_PLAYING;
			}
			else
			{
				channel_e->state = PLAYER_STATE_IDLE;
			}
		}

		if (channel_e->state == PLAYER_STATE_PLAYING)
		{
			while (channel_p->current_event >= 0)
			{
				sequence_event_t event = player->store->events[channel_p->current_event];
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

					if (state[(bit_idx >> 5)] & 1 << (bit_idx & 0x1F))
					{
						next_event = SEQUENCE_EVENT_NEXT(event);
					}
					else
					{
						next_event = extra_data >> 8;
					}

					channel_p->current_event += next_event;
					if (next_event == 0)
						channel_p->current_event = -1;

					completion = SEQUENCE_COMPLETION_IMMEDIATE;
					break;
				}
				case event_type_store:
				{
					uint32_t bit_idx = SEQUENCE_EVENT_EXTRA(event) & 0x7F;
					state[(bit_idx >> 5)] |= 1 << (bit_idx & 0x1F);

					channel_p->current_event += SEQUENCE_EVENT_NEXT(event);
					if (SEQUENCE_EVENT_NEXT(event) == 0)
						channel_p->current_event = -1;

					completion = SEQUENCE_COMPLETION_IMMEDIATE;
					break;
				}
				case event_type_nop:
				{
					channel_p->current_event += SEQUENCE_EVENT_NEXT(event);
					if (SEQUENCE_EVENT_NEXT(event) == 0)
						channel_p->current_event = -1;

					completion = SEQUENCE_COMPLETION_NEXT_TICK;
					break;
				}
				default: // user defined event_types 
				{
					//	need to take the signed bit for negative next event ... 
					channel_p->current_event += SEQUENCE_EVENT_NEXT(event);
					if (SEQUENCE_EVENT_NEXT(event) == 0)
						channel_p->current_event = -1;

					completion = player->event_handlers[event_type](player, event);
					break;
				}
				}

				if (completion == SEQUENCE_COMPLETION_IMMEDIATE)			//	immediately move on to next event
				{
					if (channel_p->current_event < 0)
					{
						channel_e->state = PLAYER_STATE_IDLE;
						break;
					}
				}
				else if (completion == SEQUENCE_COMPLETION_NEXT_TICK)		//	wait until next update to move on to next event
				{
					if (channel_p->current_event < 0)
					{
						channel_e->state = PLAYER_STATE_IDLE;
					}
					else
					{
						channel_e->state = PLAYER_STATE_PLAYING;	//	for events that push states
					}
					break;
				}
				else if (completion == SEQUENCE_COMPLETION_WAIT_ON_FLAG)		//	wait on internal timer, before moving onto next event
				{
					channel_e->state = PLAYER_STATE_WAITING;
					break;
				}
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

