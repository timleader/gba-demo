
#include "games/7days/states/states.h"

#include "common/graphics/overlay.h"
#include "common/graphics/graphics.h"
#include "common/resources/resources.h"
#include "common/input/input.h"
#include "common/savegame/savegame.h"
#include "common/utils/profiler.h"
#include "common/utils/coroutine.h"

#include "games/7days/world.h"

#include "package_7days.h"

//	mode, load / save

//-----------------------------------------------------------------------------
typedef enum savegame_mode_e
{
	SAVEGAME_MODE_LOAD = 0,
	SAVEGAME_MODE_SAVE = 1

} savegame_mode_t;


//-----------------------------------------------------------------------------
typedef struct st_savegame_context_s
{
	palette_ptr savegame_palette;

	uint8_t panel_savegame_label[SAVEGAME_SLOT_COUNT];
	uint8_t panel_savegame_preview[SAVEGAME_SLOT_COUNT];
	uint8_t panel_savegame_controls;

	int8_t st_savegame_selected_idx;
	savegame_mode_t st_savegame_mode;

	tiledimage_ptr st_savegame_screenshot;
	coroutine_ptr screenshot_coroutine;

	uint8_t palette_remap[256];

} st_savegame_context_t;

typedef st_savegame_context_t* st_savegame_context_ptr;


//-----------------------------------------------------------------------------
void st_savegame_draw_save_slots(st_savegame_context_ptr context)
{
	char slot_str[8];

	for (uint8_t idx = 0; idx < SAVEGAME_SLOT_COUNT; ++idx)
	{
		uint8_t panel_id = context->panel_savegame_label[idx];

		uint8_t coloridx = 2;
		if (context->st_savegame_selected_idx == idx)
			coloridx = 3;

		overlay_clear(panel_id, 0);

		{
			point2_t pos = { 0, 0 };
			sprintf(slot_str, "SLOT %i", (idx + 1));
			overlay_draw_string(panel_id, slot_str, coloridx, pos);
		}

		{
			point2_t pos = { 0, 2 };

			char* savegame_name = "---";
			if (savegame_has_savedata(&savegame_slots->slots[idx]))
				savegame_name = savegame_slots->slots[idx].name;
		
			//	if not set, write ---

			overlay_draw_string(panel_id, savegame_name, coloridx, pos);
		}
	}
}

//-----------------------------------------------------------------------------
void st_savegame_generate_palette_remap(st_savegame_context_ptr context)
{
	uint8_t* src = g_graphics_context.frame_buffer;		
	uint8_t* palette_remap = context->palette_remap;

	palette_ptr graphics_pal = palette_new(256, MEMORY_EWRAM);		
	graphics_read_palette(graphics_pal);
	for (uint16_t idx = 0; idx < graphics_pal->size; ++idx)
	{
		color555_t color = graphics_pal->color555[idx];
		uint16_t sum =
			(color & 0x1F) +
			((color >> 5) & 0x1F) +
			((color >> 10) & 0x1F);

		sum >>= 3;

		palette_remap[idx] = sum + 1;
	}
}

//-----------------------------------------------------------------------------
void st_savegame_create_screenshot_coroutine(st_savegame_context_ptr context)
{
	uint8_t* src = 0;
	uint8_t* palette_remap = context->palette_remap;

	coroutine_yield();

	tile_ptr dest_tile = &context->st_savegame_screenshot->tiles[0];

	for (uint16_t y = 0; y < context->st_savegame_screenshot->tile_height; ++y)
	{
		for (uint16_t x = 0; x < context->st_savegame_screenshot->tile_width; ++x)
		{
			src = g_graphics_context.frame_buffer + (((y * 8) << 2) * g_graphics_context.width) + (((x * 8) + 4) << 2);

			for (uint16_t row = 0; row < 8; ++row)
			{
				uint32_t out = 0;

				out |= palette_remap[src[0 << 2]];
				out |= palette_remap[src[1 << 2]] << 4;
				out |= palette_remap[src[2 << 2]] << 8;
				out |= palette_remap[src[3 << 2]] << 12;

				out |= palette_remap[src[4 << 2]] << 16;
				out |= palette_remap[src[5 << 2]] << 20;
				out |= palette_remap[src[6 << 2]] << 24;
				out |= palette_remap[src[7 << 2]] << 28;

				dest_tile->data[row] = out;

				src += 4 * g_graphics_context.width;
			}
			dest_tile++;
		}

		coroutine_yield();
	}
}

//-----------------------------------------------------------------------------
void st_savegame_enter(st_savegame_context_ptr context, uint32_t parameter)
{
	context->st_savegame_mode = parameter & 0x03;

	context->screenshot_coroutine = NULL;
	context->st_savegame_screenshot = tiledimage_new(7, 5, MEMORY_EWRAM);

	context->st_savegame_selected_idx = 0;

	palette_ptr gray_scale_pal = palette_new(12, MEMORY_IWRAM);	//	be cool to create this on the stack 
	gray_scale_pal->offset = 240;
	gray_scale_pal->color555[0] = RGB555(1, 1, 1);
	gray_scale_pal->color555[1] = RGB555(22, 22, 22);
	gray_scale_pal->color555[2] = RGB555(44, 44, 44);
	gray_scale_pal->color555[3] = RGB555(66, 66, 66);
	gray_scale_pal->color555[4] = RGB555(88, 88, 88);
	gray_scale_pal->color555[5] = RGB555(110, 110, 110);
	gray_scale_pal->color555[6] = RGB555(132, 132, 132);
	gray_scale_pal->color555[7] = RGB555(154, 154, 154);
	gray_scale_pal->color555[8] = RGB555(176, 176, 176);
	gray_scale_pal->color555[9] = RGB555(198, 198, 198);
	gray_scale_pal->color555[10] = RGB555(220, 220, 220);
	gray_scale_pal->color555[11] = RGB555(242, 242, 242);

	overlay_write_palette(gray_scale_pal);

	palette_delete(gray_scale_pal);

	/*
		3 save slots, 

		slot num, day, room, screenshot ??  240x160 -> 60x40 
	*/

	for (uint8_t idx = 0; idx < SAVEGAME_SLOT_COUNT; ++idx)
	{
		uint8_t panel_label_id = overlay_create_panel(4, 3);
		overlay_clear(panel_label_id, 0);
		{
			point2_t pos = { 68, 12 + (idx * 44) };
			overlay_set_position(panel_label_id, pos);
		}
		context->panel_savegame_label[idx] = panel_label_id;

		uint8_t panel_preview_id = overlay_create_panel(2, 5);
		overlay_clear(panel_preview_id, 0);
		{
			point2_t pos = { 4, 4 + (idx * 44) };
			overlay_set_position(panel_preview_id, pos);
		}
		overlay_set_palette_bank(panel_preview_id, 15);
		{
			if (savegame_has_savedata(&savegame_slots->slots[idx]))
			{
				point2_t pos = { 0, 0 };
				world_savegame_load_preview_image(&savegame_slots->slots[idx], context->st_savegame_screenshot);

				overlay_draw_tiledimage(panel_preview_id, context->st_savegame_screenshot, pos);
			}
		}
		context->panel_savegame_preview[idx] = panel_preview_id;
	}

	st_savegame_draw_save_slots(context);

	context->panel_savegame_controls = overlay_create_panel(5, 3);

	{
		point2_t pos = { 76, 136 };  
		overlay_set_position(context->panel_savegame_controls, pos);
	}

	overlay_clear(context->panel_savegame_controls, 0);

	tiledimage_ptr btn_img = resources_find_tiledimage(RES__UI_BTN_A);

	{
		point2_t pos = { 0, 0 };
		overlay_draw_tiledimage(context->panel_savegame_controls, btn_img, pos);
	}

	if (context->st_savegame_mode == SAVEGAME_MODE_SAVE)
	{
		st_savegame_generate_palette_remap(context);

		context->screenshot_coroutine = coroutine_new(st_savegame_create_screenshot_coroutine, context, MEMORY_EWRAM);
		coroutine_start(context->screenshot_coroutine);
	}
}

//-----------------------------------------------------------------------------
void st_savegame_exit(st_savegame_context_ptr context)
{
	for (uint8_t idx = 0; idx < SAVEGAME_SLOT_COUNT; ++idx)
		overlay_destroy_panel(context->panel_savegame_label[idx]);

	for (uint8_t idx = 0; idx < SAVEGAME_SLOT_COUNT; ++idx)
		overlay_destroy_panel(context->panel_savegame_preview[idx]);

	overlay_destroy_panel(context->panel_savegame_controls);

	tiledimage_delete(context->st_savegame_screenshot);

	if (context->screenshot_coroutine != NULL)
		coroutine_delete(context->screenshot_coroutine);
}

//-----------------------------------------------------------------------------
void st_savegame_update(st_savegame_context_ptr context, fixed16_t dt)
{
	if (context->st_savegame_mode == SAVEGAME_MODE_SAVE)
	{
		profiler_sample_handle_t phandle;

		phandle = profiler_begin("sg:screenshot");
		
		if (context->screenshot_coroutine)
		{
			bool8_t has_next = coroutine_next(context->screenshot_coroutine);

			if (!has_next)
			{
				coroutine_delete(context->screenshot_coroutine);
				context->screenshot_coroutine = NULL;
			}
		}

		profiler_end(phandle);
	}

	if (key_hit(KI_B))
	{
		state_pop(~0);
	}
	else if (key_hit(KI_A))
	{
		switch (context->st_savegame_mode)
		{
			case SAVEGAME_MODE_LOAD:
			{
				if (savegame_has_savedata(&savegame_slots->slots[context->st_savegame_selected_idx]))
				{
					audio_sfx_play(resources_find_audioclip(RES__SND_ACCEPT));

					uint32_t parameter = 0;
					parameter |= context->st_savegame_selected_idx << 1;
					state_goto(&st_level, parameter);
				}
				else
				{
					//	failed sound
				}
				break;
			}
			case SAVEGAME_MODE_SAVE:
			{
				debug_assert(g_main_world != NULL, "st_savegame::update g_main_world == NULL");

				if (context->screenshot_coroutine)
				{
					while (coroutine_next(context->screenshot_coroutine));

					coroutine_delete(context->screenshot_coroutine);
					context->screenshot_coroutine = NULL;
				}

				audio_sfx_play(resources_find_audioclip(RES__SND_ACCEPT));

				world_savegame(g_main_world, context->st_savegame_screenshot, &savegame_slots->slots[context->st_savegame_selected_idx]);
				state_pop(~0);
				break;
			}
		}
	}
	else
	{
		if (key_hit(KI_UP))
		{
			if (--context->st_savegame_selected_idx < 0)
				context->st_savegame_selected_idx = 0;
			else
				audio_sfx_play(resources_find_audioclip(RES__SND_SELECT));

			st_savegame_draw_save_slots(context); 
		}
		if (key_hit(KI_DOWN))
		{
			if (++context->st_savegame_selected_idx >= SAVEGAME_SLOT_COUNT)
				context->st_savegame_selected_idx = SAVEGAME_SLOT_COUNT - 1;
			else
				audio_sfx_play(resources_find_audioclip(RES__SND_SELECT));

			st_savegame_draw_save_slots(context);
		}
	}
}

//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_savegame =
{
	(state_enter_func_ptr)st_savegame_enter,
	(state_exit_func_ptr)st_savegame_exit,

	NULL,
	NULL,

	(state_update_func_ptr)st_savegame_update,
	NULL,

	"st_savegame",
	sizeof(st_savegame_context_t)

};