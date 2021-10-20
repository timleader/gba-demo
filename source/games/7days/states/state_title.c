
#include "games/7days/states/states.h"

#include "common/math/matrix.h"
#include "common/math/point.h"

#include "common/graphics/graphics.h"
#include "common/graphics/overlay.h"
#include "common/graphics/image.h"
#include "common/graphics/camera.h"
#include "common/graphics/text.h"
#include "common/collision/collision.h"
#include "common/resources/resources.h"
#include "common/input/input.h"
#include "common/memory.h"
#include "common/states/debug/states.h"

#include "package_7days.h"

//-----------------------------------------------------------------------------
typedef struct st_title_context_s
{
	uint8_t panel_start;
	uint8_t panel_start_title;
	int8_t selected_idx;
	uint8_t reserved;

	audio_stream_handle_t background_music_handle;

} st_title_context_t;

typedef st_title_context_t* st_title_context_ptr;



//-----------------------------------------------------------------------------
void st_title_draw_menu(st_title_context_ptr context)
{
	overlay_clear(context->panel_start, 0);

	point2_t string_position = { 1, 0 };
	/*overlayDrawString(panel_start, "Continue", 2, string_position);
	string_position.y++;*/

	const uint16_t text[] = { 1, 2, 3, 4 };	//	hmmmm

	for (int8_t idx = 0; idx < 4; ++idx)
	{
		uint8_t coloridx = 2;
		if (context->selected_idx == idx)
			coloridx = 3;

		overlay_draw_string(context->panel_start, stringstore_get(text[idx]), coloridx, string_position);
		string_position.y++; string_position.y++;
	}
}

//-----------------------------------------------------------------------------
void st_title_update(st_title_context_ptr context, fixed16_t dt)
{
	if (key_hit(KI_SELECT))
	{
		state_push(&st_analysis, ~0);
	}
	else if (
		key_hit(KI_START) ||
		key_hit(KI_A))
	{
		audio_sfx_play(resources_find_audioclip(RES__SND_ACCEPT));

		switch (context->selected_idx)
		{
			case 0:
			{
				uint32_t parameter = 0x01;
				parameter |= RES__LVL_3 << 1;

				state_goto(&st_level, parameter);
				break;
			}
			case 1:
				state_push(&st_savegame, 0);
				break;
			case 2:
				state_push(&st_settings, 0);
				break;
			case 3:
				state_push(&st_resources_inspector, 0);
				break;
		}
	}
	else
	{
		if (key_hit(KI_UP))
		{
			if (--context->selected_idx < 0)
				context->selected_idx = 0;
			else
				audio_sfx_play(resources_find_audioclip(RES__SND_SELECT));

			st_title_draw_menu(context);
		}
		if (key_hit(KI_DOWN))
		{
			if (++context->selected_idx >= 4)
				context->selected_idx = 3;
			else
				audio_sfx_play(resources_find_audioclip(RES__SND_SELECT));

			st_title_draw_menu(context);
		}
	}
}

//-----------------------------------------------------------------------------
void st_title_pause(st_title_context_ptr context)
{
	overlay_destroy_panel(context->panel_start);
	overlay_destroy_panel(context->panel_start_title);
}

//-----------------------------------------------------------------------------
void st_title_exit(st_title_context_ptr context)
{
	st_title_pause(context);

	audio_stream_close(context->background_music_handle);	
}

//-----------------------------------------------------------------------------
void st_title_resume(st_title_context_ptr context, uint32_t parameter)
{
	palette_ptr ui_palette = resources_find_palette(RES__PAL_UI_DEFAULT);
	overlay_write_palette(ui_palette);

	context->panel_start = overlay_create_panel(3, 8);

	point2_t panel_position = { 80, 160 - 64 };
	overlay_set_position(context->panel_start, panel_position);

	st_title_draw_menu(context);

	context->panel_start_title = overlay_create_panel(6, 8);

	point2_t title_panel_position = { 24, 16 };
	overlay_set_position(context->panel_start_title, title_panel_position);
	overlay_set_palette_bank(context->panel_start_title, 1);

	tiledimage_ptr title_img = resources_find_tiledimage(RES__UI_TITLE);
	palette_ptr title_palette = resources_find_palette(title_img->palette_id);

	overlay_write_palette(title_palette);

	point2_t img_position = { 0, 0 };
	overlay_draw_tiledimage(context->panel_start_title, title_img, img_position);

	image_ptr image = resources_find_image(RES__IMG_TITLE);
	palette_ptr palette = resources_find_palette(image->palette_id);

	graphics_write_palette(palette);

	graphics_clear(0);
	 
	graphics_draw_image(image, 0, 0, 0);

	graphics_pageflip();
}

//-----------------------------------------------------------------------------
void st_title_enter(st_title_context_ptr context, uint32_t parameter)
{
	context->selected_idx = 0;

	st_title_resume(context, parameter);

	context->background_music_handle = audio_stream_open(resources_find_audioclip(RES__SND_MUSIC), 0);
	audio_stream_volume(context->background_music_handle, 128);
}

//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_title =
{
	(state_enter_func_ptr)st_title_enter,
	(state_exit_func_ptr)st_title_exit,

	(state_pause_func_ptr)st_title_pause,
	(state_resume_func_ptr)st_title_resume,

	(state_update_func_ptr)st_title_update,
	NULL,

	"st_title", 
	sizeof(st_title_context_t)

};
