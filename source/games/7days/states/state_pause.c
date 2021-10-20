
#include "games/7days/states/states.h"

#include "common/math/matrix.h"
#include "common/math/point.h"
#include "common/math/easing.h"

#include "common/graphics/graphics.h"
#include "common/graphics/overlay.h"
#include "common/graphics/image.h"
#include "common/graphics/camera.h"
#include "common/graphics/text.h"
#include "common/collision/collision.h"
#include "common/resources/resources.h"
#include "common/input/input.h"
#include "common/memory.h"

#include "package_7days.h"


//-----------------------------------------------------------------------------
typedef struct st_pause_context_s
{
	palette_ptr pause_palette;

	perf_timer_t pause_timer;

	uint8_t panel_pause;
	int8_t st_pause_selected_idx;

	uint8_t panel_title;

	uint8_t reserved;

} st_pause_context_t;

typedef st_pause_context_t* st_pause_context_ptr;


//-----------------------------------------------------------------------------
void st_pause_draw_menu(st_pause_context_ptr context)
{
	overlay_clear(context->panel_pause, 0);

	point2_t string_position = { 1, 0 };

	const char* text[] = 
	{ 
		"Resume", 
		"Save", 
		"Load",
		"Settings",
		"Quit"
	};	//	hmmmm

	for (int8_t idx = 0; idx < 5; ++idx)
	{
		uint8_t coloridx = 2;
		if (context->st_pause_selected_idx == idx)
			coloridx = 3;

		string_position.x = (12 - string_length(text[idx])) >> 1;

		overlay_draw_string(context->panel_pause, text[idx], coloridx, string_position);
		string_position.y++; string_position.y++;
	}
}

//-----------------------------------------------------------------------------
void st_pause_create_ui(st_pause_context_ptr context)
{
	context->panel_pause = overlay_create_panel(3, 9);
	point2_t panel_position = { 72, 60 };
	overlay_set_position(context->panel_pause, panel_position);

	context->panel_title = overlay_create_panel(3, 1);

	panel_position.x = 72;
	panel_position.y = 32;
	overlay_set_position(context->panel_title, panel_position);
	point2_t string_position = { 0, 0 };
	overlay_draw_string(context->panel_title, "-- Paused --", 1, string_position);
}

//-----------------------------------------------------------------------------
void st_pause_enter(st_pause_context_ptr context, uint32_t parameter)
{
	palette_ptr original_pal = resources_find_palette(RES__PAL_UI_DEFAULT);
	context->pause_palette = palette_new_from_copy(original_pal, MEMORY_EWRAM);

	overlay_write_palette(context->pause_palette);

	context->st_pause_selected_idx = 0;

	st_pause_create_ui(context);

	st_pause_draw_menu(context);

	context->pause_timer = timer_start(10, TIMER_MODE_FLIP_FLOP);
}

//-----------------------------------------------------------------------------
void st_pause_update(st_pause_context_ptr context, fixed16_t dt)
{
	fixed16_t progress = timer_progress(context->pause_timer);
	progress = math_easeinoutquad(progress);

	context->pause_palette->color555[1] = color555_lerp(RGB555(64, 64, 255), RGB555(255, 255, 255), progress);
	overlay_write_palette(context->pause_palette);

	if (
		key_hit(KI_A) || 
		key_hit(KI_START))
	{
		audio_sfx_play(resources_find_audioclip(RES__SND_ACCEPT));

		switch (context->st_pause_selected_idx)
		{
		case 0:
			state_pop(~0);
			break;
		case 1:
			state_push(&st_savegame, 1);
			break;
		case 2:
			state_push(&st_savegame, 0);
			break;
		case 3:
			//	options
			break;
		case 4:
			state_goto(&st_title, ~0);
			break;
		}
	}
	else
	{
		if (key_hit(KI_UP))
		{
			if (--context->st_pause_selected_idx < 0)
				context->st_pause_selected_idx = 0;
			else
				audio_sfx_play(resources_find_audioclip(RES__SND_SELECT));

			st_pause_draw_menu(context);
		}
		if (key_hit(KI_DOWN))
		{
			if (++context->st_pause_selected_idx >= 5)
				context->st_pause_selected_idx = 4;
			else
				audio_sfx_play(resources_find_audioclip(RES__SND_SELECT));

			st_pause_draw_menu(context);
		}
	}
}

//-----------------------------------------------------------------------------
void st_pause_exit(st_pause_context_ptr context)
{
	overlay_destroy_panel(context->panel_pause);
	overlay_destroy_panel(context->panel_title);

	memory_free(context->pause_palette);
}

//-----------------------------------------------------------------------------
void st_pause_pause(st_pause_context_ptr context)
{
	overlay_destroy_panel(context->panel_pause);
	overlay_destroy_panel(context->panel_title);
}

//-----------------------------------------------------------------------------
void st_pause_resume(st_pause_context_ptr context, uint32_t parameter)
{
	st_pause_create_ui(context);

	st_pause_draw_menu(context);
}

//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_pause =
{
	(state_enter_func_ptr)st_pause_enter,
	(state_exit_func_ptr)st_pause_exit,

	(state_pause_func_ptr)st_pause_pause,
	(state_resume_func_ptr)st_pause_resume,

	(state_update_func_ptr)st_pause_update,
	NULL,

	"st_pause",
	sizeof(st_pause_context_t)

};
