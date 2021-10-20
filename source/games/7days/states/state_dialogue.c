
#include "games/7days/states/states.h"

#include "common/math/matrix.h"
#include "common/math/point.h"

#include "common/memory.h"
#include "common/graphics/graphics.h"
#include "common/graphics/overlay.h"
#include "common/graphics/image.h"
#include "common/graphics/camera.h"
#include "common/graphics/text.h"
#include "common/collision/collision.h"
#include "common/resources/resources.h"
#include "common/input/input.h"
#include "common/utils/timer.h"
#include "common/math/easing.h"

#include "games/7days/dialogue.h"
#include "package_7days.h"

//-----------------------------------------------------------------------------
const int32_t dialogue_palette_offset = 3;

//-----------------------------------------------------------------------------
typedef enum dialogue_step_e
{
	DIALOGUE_WRITE_OUT,
	DIALOGUE_PLAYER_READ_TIME,
	DIALOGUE_OPTION_SELECT,
	DIALOGUE_MOVE_NEXT,
	DIALOGUE_FASTFORWARD,	

} dialogue_step_t;


//-----------------------------------------------------------------------------
typedef struct st_dialogue_context_s
{
	uint8_t panel_profile;
	uint8_t panel_textbox;

	char dialogue_text[256];
	char* dialogue_cursor;

	point2_t dialogue_text_position;

	dialogue_step_t g_dialogue_step;		//	define this as an enum

	dialogue_ptr g_dialogue;
	dialogue_entry_ptr g_dialogue_entry;
	int16_t g_dialogue_next;

	int8_t g_dialogue_auto_close;

	int16_t g_dialogue_option_idx;
	int16_t g_dialogue_option_count;

	game_timer_t g_dialogue_timer;

	game_timer_t g_dialogue_animation_timer;

	palette_ptr ui_palette;

} st_dialogue_context_t;

typedef st_dialogue_context_t* st_dialogue_context_ptr;

/*
	TODO:

		+ options dialogue (including selection)
			+ selection
			+	indicate if an option path has previously been read
			+ Icon ahead of each option, question can lead !!! 
			 
		+ continue/skip button

		+ text formatting (eg. line wrap) ***	<<< ---		still broken for options dialogue, works for normal text

		+ sprite transparency	??

*/

//-----------------------------------------------------------------------------
void st_dialogue_set_text(st_dialogue_context_ptr context, uint16_t string_id)
{
	const char* text = stringstore_get(string_id);
	string_wrap(text, context->dialogue_text, (5 * 4));
	context->dialogue_cursor = context->dialogue_text;
}

//-----------------------------------------------------------------------------
void st_dialogue_update_palette(st_dialogue_context_ptr context)
{
	debug_assert(context->g_dialogue_option_count >= 0, "");
	debug_assert(context->g_dialogue_option_count <= 255, "");

	for (int8_t idx = dialogue_palette_offset; idx < dialogue_palette_offset + context->g_dialogue_option_count; ++idx)
		context->ui_palette->color555[idx] = RGB555(127, 127, 127);

	context->ui_palette->color555[dialogue_palette_offset + context->g_dialogue_option_idx] = RGB555(255, 255, 255);

	overlay_write_palette(context->ui_palette);
}

//-----------------------------------------------------------------------------
void st_dialogue_draw_continue_icon(st_dialogue_context_ptr context)
{
	//	this doesn't work for multiple options 

	int32_t line_count = string_character_count(context->dialogue_text, '\n');

	//	only draw this on the wait for player to be able to read step
	tiledimage_ptr image_more = resources_find_tiledimage(RES__UI_ARROW);		//	control background + foreground color idx ,, monotone graphics for UI .. is it worth thinking about 
	point2_t pos_more = { 19, line_count + 2 };		//	number of lines 
	overlay_draw_tiledimage(context->panel_textbox, image_more, pos_more);

	context->g_dialogue_animation_timer = timer_start(10, TIMER_MODE_FLIP_FLOP);
}

//-----------------------------------------------------------------------------
void st_dialogue_next(st_dialogue_context_ptr context, int16_t id)
{
	//	is next id, a dialogue or do we need to pop this state 

	dialogue_entry_ptr dialogue = &context->g_dialogue->entries[id];
	context->g_dialogue_entry = dialogue;

	uint16_t attributes = dialogue->attributes;

	/**
		attributes

		count 0 - 2
		layout 3
		auto-continue 4

	 */

	context->g_dialogue_option_idx = 0;
	context->g_dialogue_option_count = (attributes) & 0x03;
	uint8_t layout = (attributes >> 2) & 0x03;

	uint16_t character_img = dialogue->character_image;

	st_dialogue_set_text(context, dialogue->options[context->g_dialogue_option_idx].text);
	int32_t line_count = string_character_count(context->dialogue_cursor, '\n');

	context->g_dialogue_next = dialogue->options[context->g_dialogue_option_idx].next;

	context->dialogue_text_position.x = 1;
	context->dialogue_text_position.y = 1;

	overlay_clear(context->panel_profile, 0);	//	isn't actually needed
	overlay_clear(context->panel_textbox, 0);// g_dialogue_option_count + 1);

	//	draw border on panel_textbox 
	tiledimage_ptr image = resources_find_tiledimage_from_name("ui/border");
	//	TODO: load palette for the above !! 
	point2_t background_position = { 0, 0 };
	point2_t background_size = { 22, line_count + 6 };
	overlay_draw_tiledimage_nineslice(context->panel_textbox, image, background_position, background_size);

	image = resources_find_tiledimage(character_img);
	palette_ptr palette = resources_find_palette(image->palette_id);

	if (layout & 0x01)	// right
	{
		point2_t pos = { 174, 10 };
		overlay_set_position(context->panel_profile, pos);	//	left or right

		point2_t pos2 = { 6, 6 };
		overlay_set_position(context->panel_textbox, pos2);

		point2_t position = { 1, 0 };
		overlay_draw_tiledimage(context->panel_profile, image, position);
	}
	else  //	left
	{
		point2_t pos = { 10, 10 };
		overlay_set_position(context->panel_profile, pos);	//	left or right

		point2_t pos2 = { 58, 6 };
		overlay_set_position(context->panel_textbox, pos2);

		point2_t position = { 0, 0 };
		overlay_draw_tiledimage(context->panel_profile, image, position);
	}

	//	calculate height required for text box

	overlay_write_palette(palette);
	overlay_set_palette_bank(context->panel_profile, 1);
	//	need to sort offsets and such like to correct displaying of 4-bit ui and use of color banks 

}

/*
	Palette
		0  - 32		= character_image
		33, 34, 35  = options text
		36 - 64		= various UI
*/

//-----------------------------------------------------------------------------
void st_dialogue_update(st_dialogue_context_ptr context, fixed16_t dt)
{
	if (context->g_dialogue_step == DIALOGUE_WRITE_OUT)
	{
		if (*context->dialogue_cursor == '\n')
		{
			context->dialogue_text_position.x = 1;
			context->dialogue_text_position.y++;
		}
		else
		{
			context->dialogue_text_position = overlay_draw_character(context->panel_textbox, *context->dialogue_cursor, dialogue_palette_offset + context->g_dialogue_option_idx, context->dialogue_text_position);
		}
	}
	else if (context->g_dialogue_step == DIALOGUE_FASTFORWARD)
	{
		while (*context->dialogue_cursor != '\n' && *context->dialogue_cursor != 0)
		{
			context->dialogue_text_position = overlay_draw_character(context->panel_textbox, *context->dialogue_cursor, dialogue_palette_offset + context->g_dialogue_option_idx, context->dialogue_text_position);
			context->dialogue_cursor++;
		}	

		if (*context->dialogue_cursor != 0)
		{
			context->dialogue_text_position.x = 1;
			context->dialogue_text_position.y++;
			context->dialogue_cursor++;
			overlay_draw_string(context->panel_textbox, context->dialogue_cursor, dialogue_palette_offset + context->g_dialogue_option_idx, context->dialogue_text_position);
		}

		while (++context->g_dialogue_option_idx < context->g_dialogue_option_count)
		{
			st_dialogue_set_text(context, context->g_dialogue_entry->options[context->g_dialogue_option_idx].text);

			context->dialogue_text_position.x = 1;
			context->dialogue_text_position.y++;

			overlay_draw_string(context->panel_textbox, context->dialogue_cursor, dialogue_palette_offset + context->g_dialogue_option_idx, context->dialogue_text_position);
		}

		//	need to write remaining options !!! 
	}



	if (context->g_dialogue_step == DIALOGUE_WRITE_OUT)		//	read out
	{
		context->dialogue_cursor++;
		if (*context->dialogue_cursor == NULL)
		{
			if (++context->g_dialogue_option_idx < context->g_dialogue_option_count)
			{
				st_dialogue_set_text(context, context->g_dialogue_entry->options[context->g_dialogue_option_idx].text);

				context->dialogue_text_position.x = 1;	//	default position
				context->dialogue_text_position.y += 2;

				//	draw line splits between options
			}
			else
			{
				// reached end of string
				context->g_dialogue_step = DIALOGUE_PLAYER_READ_TIME;

				context->g_dialogue_option_idx = 0;

				//	move to next option , if there is one 
				context->g_dialogue_timer = timer_start(20 * 2, TIMER_MODE_ONCE);

				st_dialogue_draw_continue_icon(context);
			}
		}

		if (key_hit(KI_A))
		{
			context->g_dialogue_step = DIALOGUE_FASTFORWARD;
		}
	}
	else if (context->g_dialogue_step == DIALOGUE_FASTFORWARD)		//	fastforward the text delivery 
	{
		context->g_dialogue_step = DIALOGUE_PLAYER_READ_TIME;

		context->g_dialogue_option_idx = 0;

		//	move to next option , if there is one 
		context->g_dialogue_timer = timer_start(20 * 2, TIMER_MODE_ONCE);

		st_dialogue_draw_continue_icon(context);
	}
	else if (context->g_dialogue_step == DIALOGUE_PLAYER_READ_TIME)		//	pause so player can have a chance to read
	{		
		fixed16_t progress = timer_progress(context->g_dialogue_animation_timer);
		progress = math_easeinoutquad(progress);

		context->ui_palette->color555[2] = color555_lerp(RGB555(153, 153, 153), RGB555(127, 255, 127), progress);			//	need to be in control of certain palettes to be able to animate them correctly 
		overlay_write_palette(context->ui_palette);


		int8_t step_finished = key_hit(KI_A);

		if (timer_expired(context->g_dialogue_timer))
		{
			if (context->g_dialogue_next < 0) // LAST dialogue entry
			{
				if (context->g_dialogue_auto_close)
				{
					step_finished |= 1;
				}
			}
			else
			{
				step_finished |= 1;
			}
		}

		if (step_finished)
		{
			if (context->g_dialogue_option_count > 1)
			{
				context->g_dialogue_step = DIALOGUE_OPTION_SELECT;
			}
			else
			{
				context->g_dialogue_step = DIALOGUE_MOVE_NEXT;
			}
		}
	}
	else if (context->g_dialogue_step == DIALOGUE_OPTION_SELECT)		//	option selection 
	{
		if (key_hit(KI_UP))
		{
			if (--context->g_dialogue_option_idx < 0)
				context->g_dialogue_option_idx = 0;

			st_dialogue_update_palette(context);
		}

		if (key_hit(KI_DOWN))
		{
			if (++context->g_dialogue_option_idx >= context->g_dialogue_option_count)
				context->g_dialogue_option_idx = context->g_dialogue_option_count - 1;

			st_dialogue_update_palette(context);
		}

		if (key_hit(KI_A))
		{
			context->g_dialogue_step = DIALOGUE_MOVE_NEXT;
			context->g_dialogue_next = context->g_dialogue_entry->options[context->g_dialogue_option_idx].next;
		}
	}
	else if (context->g_dialogue_step == DIALOGUE_MOVE_NEXT)		//	listen to player input for moving next
	{
		//	auto-close catch here 
		//		does this auto-close come from enter(parameter) / struct diag

		context->g_dialogue_step = DIALOGUE_WRITE_OUT;

		if (context->g_dialogue_next < 0)
		{
			//	will need to convert id to sequence Id 

			uint32_t parameter = 0;
			if (context->g_dialogue_next >= 0)
			{
				parameter |= 1 << 24;					//	command
				parameter |= context->g_dialogue_next;				//	sequeunce_id
			}

			state_pop(parameter);
		}
		else
		{
			st_dialogue_next(context, context->g_dialogue_next);
			//	sometimes next will want to pop state !!! 
		}
	}


}

//-----------------------------------------------------------------------------
void st_dialogue_enter(st_dialogue_context_ptr context, uint32_t parameter)
{
	//	0xFFFF0000 <== resource_idx
	//	0x0000FFFF <== dialogue_idx

	//	22 bits when running via sequence player 

	uint16_t resource_id = parameter & 0x000003FF;		// 10 bit -> 1024 
	int16_t dialogue_idx = (parameter >> 10) & 0x000000FF;	//	8 bit -> 256 

	int8_t options = (parameter >> 18) & 0x0000000F;

	context->g_dialogue_auto_close = options & 0x01;

	context->g_dialogue_option_idx = 0;
	context->g_dialogue_option_count = 0;
	context->g_dialogue_step = DIALOGUE_WRITE_OUT;

	//	cutscene  - auto_close
	//	interaction points - no auto close

	context->g_dialogue = resources_find_dialogue(resource_id);


	const uint16_t palette_size = 16;

	context->ui_palette = palette_new(palette_size, MEMORY_EWRAM);

	/*
		background [1]
		arrow [2]
		dialogue text  [3,4,5,6]
	*/

	context->ui_palette->color555[1] = RGB555(153, 153, 153);
	context->ui_palette->color555[2] = RGB555(153, 153, 153);
	st_dialogue_update_palette(context);		

	context->panel_profile = overlay_create_panel(2, 6);
	context->panel_textbox = overlay_create_panel(6, 16);

	overlay_set_palette_bank(context->panel_textbox, 0);

	overlay_clear(context->panel_profile, 0);

	st_dialogue_next(context, dialogue_idx);

	//	fade graphics palette !! 
}

//-----------------------------------------------------------------------------
void st_dialogue_exit(st_dialogue_context_ptr context)
{
	overlay_destroy_panel(context->panel_profile);
	overlay_destroy_panel(context->panel_textbox);

	palette_delete(context->ui_palette);
}


//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_dialogue =
{
	(state_enter_func_ptr)st_dialogue_enter,
	(state_exit_func_ptr)st_dialogue_exit,

	NULL,
	NULL,

	(state_update_func_ptr)st_dialogue_update,
	NULL,

	"st_dialogue",
	sizeof(st_dialogue_context_t)

};
