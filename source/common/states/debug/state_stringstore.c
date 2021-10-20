
#include "states.h"

#include "common/math/matrix.h"
#include "common/graphics/graphics.h"
#include "common/graphics/overlay.h"
#include "common/graphics/image.h"
#include "common/graphics/camera.h"
#include "common/graphics/text.h"
#include "common/collision/collision.h"
#include "common/resources/resources.h"
#include "common/input/input.h"
#include "common/audio/audio.h"
#include "common/utils/bitstream.h"
#include "common/debug/debug.h"
#include "common/memory.h"


#include <stdio.h>		// todo remove this 


#define NUM_ON_SCREEN 15

//-----------------------------------------------------------------------------
typedef struct st_stringstore_context_s
{
	uint8_t panel_string_listview;
	int16_t	st_selected_idx;

	stringstore_ptr _stringstore;

} st_stringstore_context_t;

typedef st_stringstore_context_t* st_stringstore_context_ptr;


/*
	should we generalize ui item selection ?? 
		should we have a higher levels ui system ??? 
*/

//-----------------------------------------------------------------------------
void st_stringstore_draw_listview(st_stringstore_context_ptr context)
{
	overlay_clear(context->panel_string_listview, 0);

	int16_t start_idx = context->st_selected_idx - (NUM_ON_SCREEN - 1);
	if (start_idx < 0)
		start_idx = 0;

	int16_t last_idx = start_idx + NUM_ON_SCREEN;
	if (last_idx > context->_stringstore->count)
		last_idx = context->_stringstore->count;

	int16_t row = 0;
	point2_t text_pos = { 0, row };

	overlay_draw_string(context->panel_string_listview, "ID", 1, text_pos);			//	resource_type

	text_pos.x = 3;
	overlay_draw_string(context->panel_string_listview, "TEXT", 1, text_pos);			//	resource_type

	row++;

	char str_buf[256];

	char id_str[4];
	for (uint8_t idx = start_idx; idx < last_idx; ++idx)
	{
		text_pos.x = 0;
		text_pos.y = row;

		uint8_t color_idx = 2;
		if (context->st_selected_idx == idx)
			color_idx = 3;

		sprintf(id_str, "%d", (int)idx);
		overlay_draw_string(context->panel_string_listview, id_str, color_idx, text_pos);			//	resource_type

		text_pos.x = 3;

		const char* str = stringstore_get(idx);

		//	24 wide
		string_wrap(str, str_buf, 21);
		row += string_character_count(str_buf, '\n');

		overlay_draw_string(context->panel_string_listview, str_buf, color_idx, text_pos);	//	resource_name

		row++;

		if (row >= 15)
			break;
	}
}

void st_stringstore_enter(st_stringstore_context_ptr context, uint32_t parameter)
{
	uint16_t resource_id = parameter & 0x0000FFFF;

	context->_stringstore = resources_find_stringstore(resource_id);

	context->panel_string_listview = overlay_create_panel(6, 18);

	//	generate custom palette

	point2_t pos = { 22, 8 };
	overlay_set_position(context->panel_string_listview, pos);

	st_stringstore_draw_listview(context);

	//	need a clear option 

	palette_ptr palette = resources_find_palette_from_name("pal/ui/default");
	overlay_write_palette(palette);
}

void st_stringstore_exit(st_stringstore_context_ptr context)
{
	overlay_destroy_panel(context->panel_string_listview);
}

void st_stringstore_update(st_stringstore_context_ptr context, fixed16_t dt)
{
	if (key_hit(KI_B))
	{
		state_pop(0);
	}
	else
	{
		if (key_hit(KI_UP))
		{
			if (--context->st_selected_idx < 0)
				context->st_selected_idx = 0;

			st_stringstore_draw_listview(context);
		}
		if (key_hit(KI_DOWN))
		{
			if (++context->st_selected_idx >= resource_count)
				context->st_selected_idx = resource_count - 1;

			st_stringstore_draw_listview(context);
		}
	}
}

EWRAM_DATA state_t st_stringstore =
{
	(state_enter_func_ptr)st_stringstore_enter,
	(state_exit_func_ptr)st_stringstore_exit,

	NULL,
	NULL,

	(state_update_func_ptr)st_stringstore_update,
	NULL,

	"st_strstore",
	sizeof(st_stringstore_context_t)

};
