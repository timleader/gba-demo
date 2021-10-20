
#include "states.h"

#include "common/math/point.h"
#include "common/graphics/graphics.h"
#include "common/graphics/overlay.h"
#include "common/resources/resources.h"
#include "common/utils/profiler.h"
#include "common/input/input.h"

#include <stdio.h>	//	we should remove this 

//-----------------------------------------------------------------------------
#define ST_ANALYSIS_NUM_ON_SCREEN 18

//-----------------------------------------------------------------------------
typedef struct st_analysis_context_s
{
	int16_t	selected_idx;

	uint8_t panel_analysis_log;
	uint8_t r_key_held_counter;

	profiler_entry_t* profiler_entries_snapshot;

} st_analysis_context_t;

typedef st_analysis_context_t* st_analysis_context_ptr;

//-----------------------------------------------------------------------------
void st_analysis_draw_list(st_analysis_context_ptr context)
{
	overlay_clear(context->panel_analysis_log, 0);

	int16_t start_idx = context->selected_idx - (ST_ANALYSIS_NUM_ON_SCREEN - 1);
	if (start_idx < 0)
		start_idx = 0;

	int16_t last_idx = start_idx + ST_ANALYSIS_NUM_ON_SCREEN;
	if (last_idx > PROFILER_MAX_ENTRIES_COUNT)
		last_idx = PROFILER_MAX_ENTRIES_COUNT;

	profiler_entry_t* entries = context->profiler_entries_snapshot;
	if (entries)
	{
		point2_t text_pos = { 0, 0 };
		char text_buffer[128];
		for (uint16_t idx = start_idx; idx < last_idx; ++idx)	
		{
			uint8_t color_idx = 2;
			if (context->selected_idx == idx)
				color_idx = 3;

			char* text_ptr = &text_buffer[0];
			for (uint16_t j = 0; j < entries[idx].nested_depth; ++j)
				*text_ptr++ = ' ';

			sprintf(text_ptr, "%s - %d", entries[idx].label, (int)entries[idx].value);
			overlay_draw_string(context->panel_analysis_log, text_buffer, color_idx, text_pos);
			text_pos.y += 1;
		}
	}
}

//-----------------------------------------------------------------------------
void st_analysis_enter(st_analysis_context_ptr context, uint32_t parameter)
{
	uint32_t profiler_data_size_in_bytes = sizeof(profiler_entry_t) * PROFILER_MAX_ENTRIES_COUNT;

	context->profiler_entries_snapshot = memory_allocate(profiler_data_size_in_bytes, MEMORY_EWRAM);
	memory_copy(context->profiler_entries_snapshot, profiler_entries(), profiler_data_size_in_bytes);

	context->panel_analysis_log = overlay_create_panel(6, 18);

	point2_t pos = { 2, 8 };
	overlay_set_position(context->panel_analysis_log, pos);

	palette_ptr palette = resources_find_palette_from_name("ui/pal");
	overlay_write_palette(palette);

	st_analysis_draw_list(context);
}

//-----------------------------------------------------------------------------
void st_analysis_exit(st_analysis_context_ptr context)
{
	overlay_destroy_panel(context->panel_analysis_log);

	memory_free(context->profiler_entries_snapshot);
}

//-----------------------------------------------------------------------------
void st_analysis_update(st_analysis_context_ptr context, fixed16_t dt)
{
	if (key_hit(KI_B))
	{
		state_pop(0);
	}
	else
	{
		if (key_held(KI_UP) ||		//	 should this be built into the input system
			key_held(KI_DOWN))
		{
			if (context->r_key_held_counter < 5)
				context->r_key_held_counter++;
		}
		else
		{
			context->r_key_held_counter = 0;
		}

		if ((key_held(KI_UP) && context->r_key_held_counter >= 5) ||
			key_hit(KI_UP))
		{
			if (--context->selected_idx < 0)
				context->selected_idx = 0;

			st_analysis_draw_list(context);
		}

		if ((key_held(KI_DOWN) && context->r_key_held_counter >= 5) ||
			key_hit(KI_DOWN))
		{
			if (++context->selected_idx >= PROFILER_MAX_ENTRIES_COUNT)
				context->selected_idx = PROFILER_MAX_ENTRIES_COUNT - 1;

			st_analysis_draw_list(context);
		}
	}
}

//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_analysis =
{
	(state_enter_func_ptr)st_analysis_enter,
	(state_exit_func_ptr)st_analysis_exit,

	NULL,
	NULL,

	(state_update_func_ptr)st_analysis_update,
	NULL,

	"st_analysis",
	sizeof(st_analysis_context_t)

};
