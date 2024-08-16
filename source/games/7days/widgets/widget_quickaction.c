
#include "widget_quickaction.h"

#include "common/math/matrix.h"

#include "common/graphics/graphics.h"
#include "common/graphics/image.h"
#include "common/graphics/camera.h"
#include "common/graphics/text.h"
#include "common/collision/collision.h"
#include "common/resources/resources.h"
#include "common/input/input.h"
#include "common/math/easing.h"

#include "package_7days.h"

/*
	TODO: 
		+ Animated A Button

		+ Nice graphic for background

		+ Semi Transparent Background

		+ Tailor Background for centered txt
			//	requires name of interaction point !!! 

		+ destroy when state pauses 
		+ re-create when state resumes

		//	Should this always launch the quick inventory !! 
			//	then can select talk, interact, look, or use item... 
	
*/


//	next and current interaction item !!! 

const point2_t g_widget_quickaction_origin = 
{
	(240 - 96) >> 1,		//	screen width - panel width
	160 + 4
};

const int32_t g_widget_quickaction_y_diff = 32;

#define QUICKACTION_MODE_IDLE				0
#define QUICKACTION_MODE_TRANSITION_IN		1
#define QUICKACTION_MODE_READY				2
#define QUICKACTION_MODE_TRANSITION_OUT		3


void widget_quickaction_schedule_interaction(widget_quickaction_state_ptr state, interaction_point_t* interaction)
{
	if (state->mode == QUICKACTION_MODE_IDLE)	
	{
		state->mode = QUICKACTION_MODE_TRANSITION_IN;
		state->timer = timer_start(10, TIMER_MODE_ONCE);
		state->current_interaction = interaction;

		if (state->current_interaction)
		{
			const char* text = stringstore_get(state->current_interaction->name_text_id);

			uint32_t length = string_length(text);

			point2_t dialogue_text_position = { 8 - length, 1 };

			tiledimage_ptr image = resources_find_tiledimage(RES__UI_BORDER);

			point2_t draw_position = { dialogue_text_position.x, 0 };

			point2_t draw_size = { length + 1, 1 };


			point2_t p1 = { 0, 0 };
			point2_t s1 = { 9, 3 };
			overlay_draw_color(state->panel_id, 0, p1, s1);	//	partial clear 

			//	TODO: optimize this, there is over draw 


			draw_position.x = dialogue_text_position.x - 2;
			draw_size.x = 1;
			overlay_draw_tile(state->panel_id, &image->tiles[0], draw_position, draw_size);
			draw_position.x++;
			draw_size.x = length + 2;
			overlay_draw_tile(state->panel_id, &image->tiles[1], draw_position, draw_size);

			draw_position.x = dialogue_text_position.x - 2;
			draw_position.y++;
			draw_size.x = 1;
			overlay_draw_tile(state->panel_id, &image->tiles[3], draw_position, draw_size);
			draw_position.x++;
			draw_size.x = length + 2;
			overlay_draw_tile(state->panel_id, &image->tiles[4], draw_position, draw_size);

			draw_position.x = dialogue_text_position.x - 2;
			draw_position.y++;
			draw_size.x = 1;
			overlay_draw_tile(state->panel_id, &image->tiles[6], draw_position, draw_size);
			draw_position.x++;
			draw_size.x = length + 2;
			overlay_draw_tile(state->panel_id, &image->tiles[7], draw_position, draw_size);

			overlay_draw_string(state->panel_id, text, 2, dialogue_text_position); 
		}
	}

	if (state->mode == QUICKACTION_MODE_TRANSITION_OUT)	
	{
		state->mode = QUICKACTION_MODE_TRANSITION_IN;
		state->timer = timer_start(10, TIMER_MODE_ONCE);	//	start mid way through
	}
}

void widget_quickaction_snap_to_ready(widget_quickaction_state_ptr state)
{
	if (state->current_interaction)
	{
		//fixed16_t offset = fixed16_from_int(96);

		point2_t pos = g_widget_quickaction_origin;
		pos.y -= g_widget_quickaction_y_diff;

		overlay_set_position(state->panel_id, pos);

		state->mode = QUICKACTION_MODE_READY;
	}
}

void widget_quickaction_clear(widget_quickaction_state_ptr state)
{
	if (state->mode == QUICKACTION_MODE_TRANSITION_IN)	
	{

	}

	if (state->mode == QUICKACTION_MODE_READY)
	{
		state->mode = QUICKACTION_MODE_TRANSITION_OUT;
		state->timer = timer_start(10, TIMER_MODE_ONCE);
	}
}


void widget_quickaction_initialize(widget_quickaction_state_ptr state)	//	assign callback
{
	debug_printf(DEBUG_LOG_DEBUG, "widget_quickaction::initialize");

	state->panel_id = overlay_create_panel(3, 3);
	state->mode = QUICKACTION_MODE_IDLE;
	
	overlay_set_position(state->panel_id, g_widget_quickaction_origin);	

	tiledimage_ptr img = resources_find_tiledimage(RES__UI_BTN_A);
	palette_ptr pal = resources_find_palette(img->palette_id);
	overlay_write_palette(pal);

	point2_t p = { 9, 0 };
	overlay_draw_tiledimage(state->panel_id, img, p);

	point2_t p1 = { 0, 0 };
	point2_t s1 = { 9, 3 };
	overlay_draw_color(state->panel_id, 0, p1, s1);	//	this should be draw tile !!! 
}

void widget_quickaction_shutdown(widget_quickaction_state_ptr state)
{
	debug_printf(DEBUG_LOG_DEBUG, "widget_quickaction::shutdown");

	overlay_destroy_panel(state->panel_id);
	state->mode = QUICKACTION_MODE_IDLE;
}

void widget_quickaction_update(widget_quickaction_state_ptr state)
{
	//	if input(Key_A)
	//		


	if (state->mode == QUICKACTION_MODE_TRANSITION_IN)
	{
		fixed16_t progress = timer_progress(state->timer);
		progress = math_easeoutquad(progress);

		fixed16_t offset = fixed16_mul(progress, fixed16_from_int(g_widget_quickaction_y_diff));

		point2_t pos = g_widget_quickaction_origin;	
		pos.y -= fixed16_to_int(offset);

		overlay_set_position(state->panel_id, pos);	

		if (timer_expired(state->timer))
		{
			state->mode = QUICKACTION_MODE_READY;	
		}
	}

	if (state->mode == QUICKACTION_MODE_TRANSITION_OUT)	
	{ 
		fixed16_t progress = timer_progress(state->timer);
		progress = math_easeinquad(progress);

		fixed16_t offset = fixed16_mul(progress, fixed16_from_int(g_widget_quickaction_y_diff));

		point2_t pos = g_widget_quickaction_origin;
		pos.y -= g_widget_quickaction_y_diff - fixed16_to_int(offset);

		overlay_set_position(state->panel_id, pos);	

		if (timer_expired(state->timer))
		{
			state->mode = QUICKACTION_MODE_IDLE;
		}
	}

}


