
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

#include "package_7days.h"

//-----------------------------------------------------------------------------
typedef struct st_language_select_context_s
{
	uint8_t panel;
	int8_t selected_idx;

	uint8_t reserved[2];

	audioclip_ptr ui_sfx;

	palette_ptr palette;

} st_language_select_context_t;

typedef st_language_select_context_t* st_language_select_context_ptr;


//-----------------------------------------------------------------------------
void st_language_select_draw_menu(st_language_select_context_ptr context)
{
	overlay_clear(context->panel, 0);

	char flag_asset_name[23] = "lang/";
	uint32_t len = string_length(flag_asset_name);

	palette_ptr pal = resources_find_palette(RES__LANG_PAL);
	overlay_write_palette(pal);

	for (int8_t idx = 0; idx < 5; ++idx)
	{
		memory_copy(flag_asset_name + len, (const void_ptr)stringlocale_codes[idx], 2);
		flag_asset_name[len + 2] = 0;

		tiledimage_ptr img = resources_find_tiledimage_from_name(flag_asset_name);

		point2_t img_position = { 1, (idx * 3) };
		overlay_draw_tiledimage(context->panel, img, img_position);

		uint8_t coloridx = 2;
		if (context->selected_idx == idx)
			coloridx = 3;

		point2_t string_position = { 6, (idx * 3) + 1 };
		overlay_draw_string(context->panel, stringlocale_names[idx], coloridx, string_position);
	}
}

//-----------------------------------------------------------------------------
void st_language_select_enter(st_language_select_context_ptr context, uint32_t parameter)
{
	context->ui_sfx = resources_find_audioclip(RES__SND_SFX);

	palette_ptr ui_palette = resources_find_palette(RES__PAL_UI_DEFAULT);
	overlay_write_palette(ui_palette);


	context->panel = overlay_create_panel(4, 16);
	overlay_clear(context->panel, 0);

	overlay_set_palette_bank(context->panel, 1); 

	point2_t panel_position = { 56, 16 };
	overlay_set_position(context->panel, panel_position);

	context->selected_idx = 0;

	st_language_select_draw_menu(context);
}

//-----------------------------------------------------------------------------
void st_language_select_exit(st_language_select_context_ptr context)
{
	overlay_destroy_panel(context->panel);
}

//-----------------------------------------------------------------------------
void st_language_select_update(st_language_select_context_ptr context, fixed16_t dt)
{
	if (key_hit(KI_START) ||
		key_hit(KI_A))
	{
		debug_printf(DEBUG_LOG_INFO, "st_language_select::selected idx=%u", context->selected_idx);

		stringstore_set_language(context->selected_idx);

		state_goto(&st_title, 0);

		return;
	}
	else
	{
		if (key_hit(KI_UP))
		{
			if (--context->selected_idx < 0)
				context->selected_idx = 0;

			st_language_select_draw_menu(context);

			audio_sfx_play(context->ui_sfx);
		}
		if (key_hit(KI_DOWN))
		{
			if (++context->selected_idx >= 5)
				context->selected_idx = 4;

			st_language_select_draw_menu(context);

			audio_sfx_play(context->ui_sfx);
		}
	}
}

//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_language_select =
{
	(state_enter_func_ptr)st_language_select_enter,
	(state_exit_func_ptr)st_language_select_exit,

	NULL,
	NULL,

	(state_update_func_ptr)st_language_select_update,
	NULL,

	"st_lang_sel",
	sizeof(st_language_select_context_t)

};