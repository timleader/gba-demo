
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

/*
	present a ui where the user can select any levels of the ship ... 

	as a result load the selected levels 
*/

//-----------------------------------------------------------------------------
typedef struct st_elevator_context_s
{
	uint8_t panel_elevator;
	int8_t st_elevator_selected_idx;

	uint8_t reserved[2];

	palette_ptr elevator_palette;

} st_elevator_context_t;

typedef st_elevator_context_t* st_elevator_context_ptr;


//-----------------------------------------------------------------------------
void st_elevator_draw_menu(st_elevator_context_ptr context)
{
	tiledimage_ptr img = resources_find_tiledimage(RES__EVL_1);
	palette_ptr pal = resources_find_palette(img->palette_id);

	overlay_write_palette(pal);

	overlay_clear(context->panel_elevator, 0);

	const char* names[5] = {	//	this should come from localization
		"operations",
		"residential",
		"hydroponics",
		"medical",
		"hanger"
	};

	for (int8_t idx = 0; idx < 5; ++idx)
	{
		point2_t img_position = { 1, (idx * 3) };
		overlay_draw_tiledimage(context->panel_elevator, img, img_position);

		uint8_t coloridx = 2;
		if (context->st_elevator_selected_idx == idx)
			coloridx = 3;

		point2_t string_position = { 5, (idx * 3) + 1 };
		overlay_draw_string(context->panel_elevator, names[idx], coloridx, string_position);
	}
}

//-----------------------------------------------------------------------------
void st_elevator_enter(st_elevator_context_ptr context, uint32_t parameter)
{
	palette_ptr ui_palette = resources_find_palette(RES__PAL_UI_DEFAULT);
	overlay_write_palette(ui_palette);


	context->panel_elevator = overlay_create_panel(4, 16);
	overlay_clear(context->panel_elevator, 0);

	point2_t panel_position = { 56, 16 };
	overlay_set_position(context->panel_elevator, panel_position);

	context->st_elevator_selected_idx = 0;

	st_elevator_draw_menu(context);

	//	have a background for this ??? 
}

//-----------------------------------------------------------------------------
void st_elevator_exit(st_elevator_context_ptr context)
{
	overlay_destroy_panel(context->panel_elevator);
}

//-----------------------------------------------------------------------------
void st_elevator_update(st_elevator_context_ptr context, fixed16_t dt)
{
	if (key_hit(KI_START) ||
		key_hit(KI_A))
	{
		debug_printf(DEBUG_LOG_INFO, "st_elevator::selected idx=%u", context->st_elevator_selected_idx);

		const int16_t resource_ids[5] = {
			RES__LVL_2,
			RES__LVL_4,
			RES__LVL_1,
			RES__LVL_3,
			~0
		};

		const uint8_t spawn_idxs[5] = {
			0, 0, 0, 3, 0
		};

		int16_t resource_id = resource_ids[context->st_elevator_selected_idx];

		uint8_t spawn_idx = spawn_idxs[context->st_elevator_selected_idx];		//	 this should come from data probably <<<<------- there is probably a nicer way to tag this within the data 

		uint32_t parameter = 0;
		parameter |= 3 << 24;
		parameter |= resource_id & 0x000003FF;
		parameter |= ((spawn_idx & 0x3F) << 10);

		//	this should probably just trigger a script ... 

		state_pop(parameter);
		return;
	}
	else if (key_hit(KI_B))
	{
		state_pop(~0);
		return;
	}
	else
	{
		if (key_hit(KI_UP))
		{
			if (--context->st_elevator_selected_idx < 0)
				context->st_elevator_selected_idx = 0;

			st_elevator_draw_menu(context);
		}
		if (key_hit(KI_DOWN))
		{
			if (++context->st_elevator_selected_idx >= 5)
				context->st_elevator_selected_idx = 4;

			st_elevator_draw_menu(context);
		}
	}
}


//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_elevator =
{
	(state_enter_func_ptr)st_elevator_enter,
	(state_exit_func_ptr)st_elevator_exit,

	NULL,
	NULL,

	(state_update_func_ptr)st_elevator_update,
	NULL,

	"st_elevator",
	sizeof(st_elevator_context_t)

};