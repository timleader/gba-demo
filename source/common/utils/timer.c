
#include "timer.h"

#include "common/graphics/graphics.h"


game_timer_t timer_start(uint16_t duration, timer_mode_t mode)		//	rename to game_timer_t
{
	game_timer_t timer = { g_graphics_context.vblank_total_count, duration, mode };
	return timer;
}

fixed16_t timer_progress(game_timer_t timer)
{
	uint32_t progress_frame = g_graphics_context.vblank_total_count - timer.start;
	fixed16_t progress = fixed16_zero;

	switch (timer.mode)
	{
		case TIMER_MODE_ONCE:
		{
			if (progress_frame >= timer.duration)
			{
				progress = fixed16_one;
			}
			else
			{
				progress = fixed16_div(fixed16_from_int(progress_frame), fixed16_from_int(timer.duration));
			}

			break;
		}
		case TIMER_MODE_FLIP_FLOP:
		{
			uint8_t flip_flop = 0;

			while (progress_frame >= timer.duration)
			{
				progress_frame -= timer.duration;
				//timer.start += timer.duration;
				flip_flop ^= 1;
			}

			progress = fixed16_div(fixed16_from_int(progress_frame), fixed16_from_int(timer.duration));

			if (flip_flop)
				progress = fixed16_one - progress;

			break;
		}
		case TIMER_MODE_LOOP:
		{
			while (progress_frame >= timer.duration)	//	this is going to get more costly as time goes on, maybe tweak start 
			{
				progress_frame -= timer.duration;
			}

			progress = fixed16_div(fixed16_from_int(progress_frame), fixed16_from_int(timer.duration));

			break;
		}
	}

	return progress;
}

uint32_t timer_get_frame_count(void)
{
	return g_graphics_context.vblank_total_count;
}

int8_t timer_expired(game_timer_t timer)
{
	return g_graphics_context.vblank_total_count > (timer.start + timer.duration);
}

