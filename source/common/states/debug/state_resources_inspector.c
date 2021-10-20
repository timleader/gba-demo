
#include "states.h"

#include "common/memory.h"
#include "common/math/point.h"
#include "common/graphics/graphics.h"
#include "common/graphics/overlay.h"
#include "common/resources/resources.h"
#include "common/utils/profiler.h"
#include "common/input/input.h"

#include "games/7days/states/states.h"


#include <stdio.h>	//	remove this 


typedef struct st_resources_inspector_context_s
{
	int16_t	selected_idx;

	uint8_t panel_resources_listview;
	uint8_t r_key_held_counter;

} st_resources_inspector_context_t;

typedef st_resources_inspector_context_t* st_resources_inspector_context_ptr;


#define NUM_ON_SCREEN 16

void st_resources_inspector_draw_listview(st_resources_inspector_context_ptr context)
{
	overlay_clear(context->panel_resources_listview, 0);

	int16_t start_idx = context->selected_idx - (NUM_ON_SCREEN - 1);
	if (start_idx < 0)
		start_idx = 0;

	int16_t last_idx = start_idx + NUM_ON_SCREEN;
	if (last_idx > resource_count)
		last_idx = resource_count;

	int16_t row = 0;
	point2_t text_pos = { 0, row };

	overlay_draw_string(context->panel_resources_listview, "ID", 1, text_pos);			//	resource_type

	text_pos.x = 3;
	overlay_draw_string(context->panel_resources_listview, "NAME", 1, text_pos);			//	resource_type

	text_pos.x = 20;
	overlay_draw_string(context->panel_resources_listview, "TYPE", 1, text_pos);			//	resource_type

	row++; row++;

	char id_str[4];

	for (uint8_t idx = start_idx; idx < last_idx; ++idx)
	{
		text_pos.x = 0;
		text_pos.y = row;

		uint8_t color_idx = 2;
		if (context->selected_idx == idx)
			color_idx = 3; 

		sprintf(id_str, "%d", (int)idx);
		overlay_draw_string(context->panel_resources_listview, id_str, color_idx, text_pos);			//	resource_type

		text_pos.x = 3;
		if (resources[idx].name[0] == 0)
		{
			overlay_draw_string(context->panel_resources_listview, "---", color_idx, text_pos);	//	resource_name
		}
		else
		{
			overlay_draw_string(context->panel_resources_listview, resources[idx].name, color_idx, text_pos);	//	resource_name
		}

		text_pos.x = 21;
		overlay_draw_string(context->panel_resources_listview, resource_type_names[resources[idx].type], color_idx, text_pos);			//	resource_type

		row++;
	}
}

void st_resources_inspector_create_overlay(st_resources_inspector_context_ptr context)
{
	context->panel_resources_listview = overlay_create_panel(6, 18);

	//	generate custom palette

	point2_t pos = { 22, 8 };
	overlay_set_position(context->panel_resources_listview, pos);

	st_resources_inspector_draw_listview(context);

	palette_ptr ui_palette = resources_find_palette_from_name("pal/ui/default");
	overlay_write_palette(ui_palette);
}

void st_resources_inspector_destroy_overlay(st_resources_inspector_context_ptr context)
{
	overlay_destroy_panel(context->panel_resources_listview);
}

void st_resources_inspector_enter(st_resources_inspector_context_ptr context, uint32_t parameter)
{
	context->selected_idx = 0;

	/*
		-----------------------------------------------

		List Resources in Res-Pack

		Scroll throught the Resource list

		Highlight currently selected resource

		Select Resource -> Push Appropriate STate based on Resource Type 

		-----------------------------------------------

		How do we pass target resource to newly pushed state ? 
			bespoke function / generalized parameter object .. 
		
		res manifest needs name for each resource and type !!! 
		
	*/
	context->r_key_held_counter = 0;

	graphics_clear(0);
	graphics_pageflip();

	st_resources_inspector_create_overlay(context);
}

void st_resources_inspector_exit(st_resources_inspector_context_ptr context)
{
	st_resources_inspector_destroy_overlay(context);
}

void st_resources_inspector_pause(st_resources_inspector_context_ptr context)
{
	//	hide UI
	//		probably need to destroy the UI and recreate later !!!
	st_resources_inspector_destroy_overlay(context);
}

void st_resources_inspector_resume(st_resources_inspector_context_ptr context)
{
	graphics_set_resolution(240, 160);

	graphics_clear(0);
	graphics_pageflip();

	//	unhide UI
	st_resources_inspector_create_overlay(context);
}

void st_resources_inspector_update(st_resources_inspector_context_ptr context, fixed16_t dt)
{
	const state_ptr resource_state_map[] =
	{
		&st_palette,
		&st_image,
		&st_tiledimage,		//	need to clean up the state
		&st_depthmap,		//	need to work on this
		&st_audio_stream, //,			//	packing
		&st_video,			//	packing
		&st_model,
		&st_stringstore,	//	packing	

		&st_level,			//&st_scene - ability to walk around as an actor - collision visualization 
		&st_dialogue,		//&st_dialogue,
		NULL,
		NULL,

		&st_highlightfield,

		NULL
	};

	if (key_hit(KI_START) ||
		key_hit(KI_A))
	{
		state_ptr state = resource_state_map[resources[context->selected_idx].type];
		state_push(state, context->selected_idx);
	}
	else if (key_hit(KI_B))
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
			//else
			//	sound_play(resources_find_audioclip(61), 1);

			st_resources_inspector_draw_listview(context);
		}

		if ((key_held(KI_DOWN) && context->r_key_held_counter >= 5) ||
			key_hit(KI_DOWN))
		{
			if (++context->selected_idx >= resource_count)
				context->selected_idx = resource_count - 1;
			//else
			//	sound_play(resources_find_audioclip(61), 1);

			st_resources_inspector_draw_listview(context);
		}
	}
}

EWRAM_DATA state_t st_resources_inspector =
{
	(state_enter_func_ptr)st_resources_inspector_enter,
	(state_exit_func_ptr)st_resources_inspector_exit,

	(state_pause_func_ptr)st_resources_inspector_pause,
	(state_resume_func_ptr)st_resources_inspector_resume,

	(state_update_func_ptr)st_resources_inspector_update,
	NULL,

	"st_res_inp",
	sizeof(st_resources_inspector_context_t)

};
