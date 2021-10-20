
#include "games/7days/states/states.h"

#include "common/math/point.h"
#include "common/resources/resources.h"
#include "common/graphics/overlay.h"
#include "common/graphics/image.h"
#include "common/input/input.h"

#include "games/7days/inventory.h"
#include "games/7days/itemstore.h"
#include "games/7days/world.h"

#include "package_7days.h"


//-----------------------------------------------------------------------------
#define INVENTORY_COLUMN_COUNT 12

//-----------------------------------------------------------------------------
typedef struct st_inventory_context_s
{
	uint8_t panel_inventory;
	uint8_t panel_inventory_highlight;
	uint8_t panel_inventory_label;
	uint8_t reserved;

	int16_t inventory_current_idx;
	int16_t inventory_scroll_idx;

} st_inventory_context_t;

typedef st_inventory_context_t* st_inventory_context_ptr;


//	different states for 'inventory inspection' vs 'inventory selection'
	//	do we actually need inspection


//	doesn't consider inventory column count vs current inventory count 

//-----------------------------------------------------------------------------
void st_inventory_set_highlight(st_inventory_context_ptr context, item_ptr item)
{
	tiledimage_ptr img = resources_find_tiledimage(item->active_image_id);
	palette_ptr pal = resources_find_palette(img->palette_id);
	overlay_write_palette(pal);

	point2_t i_pos = { 0, 0 };
	overlay_draw_tiledimage(context->panel_inventory_highlight, img, i_pos);
	overlay_set_palette_bank(context->panel_inventory_highlight, 2);
}

//-----------------------------------------------------------------------------
void st_inventory_enter(st_inventory_context_ptr context, uint32_t parameter)		//	should this just return the selected item? 
{
	//	darken bottom of the screen		//	how do we do this ??? 
	//		checker-board approach maybe 

	/*
		look in parameter to handle:
			
			+ new item added
			+ or default launch
	*/

	//	pass cur_interaction in 

	inventory_ptr inventory = &g_main_world->persistent.inventory;


	context->inventory_current_idx = 0;

	context->panel_inventory = overlay_create_panel(INVENTORY_COLUMN_COUNT, 4);

	overlay_set_palette_bank(context->panel_inventory, 1);

	point2_t panel_position = { (graphics_screen_width - (6 * 32)) >> 1, graphics_screen_height - (32 + 4) };
	overlay_set_position(context->panel_inventory, panel_position);
	overlay_set_priority(context->panel_inventory, 1);

	overlay_clear(context->panel_inventory, 0);

	palette_ptr pal = resources_find_palette(RES__INV_INACTIVE_PAL);

	overlay_write_palette(pal);

	//	create from inventory

	uint8_t item_count = inventory_count(inventory);

	debug_assert(item_count <= INVENTORY_COLUMN_COUNT, "st_inventory::enter item_count > INVENTORY_COLUMN_COUNT");

	uint8_t x = 0, y = 0;
	for (int8_t idx = 0; idx < item_count; ++idx)
	{
		point2_t img_pos = { x * 4, y * 4 };

		debug_assert(inventory->items[idx] >= 0, "st_inventory::enter inventory->items[idx] < 0");

		item_ptr item = itemstore_get(inventory->items[idx]);

		tiledimage_ptr img = resources_find_tiledimage(item->inactive_image_id);

		overlay_draw_tiledimage(context->panel_inventory, img, img_pos);

		++x;
	}


	//	highlight overlay panel.. may need priority to work
	item_ptr item = itemstore_get(inventory->items[0]); 

	context->panel_inventory_highlight = overlay_create_panel(1, 4);

	point2_t highlight_pos = { 0, 0 };
	overlay_set_position(context->panel_inventory_highlight, highlight_pos);

	st_inventory_set_highlight(context, item);

	//-------------------------------------------------------------

	context->panel_inventory_label = overlay_create_panel(3, 1);

	point2_t label_pos = { 0, 0 };
	overlay_set_position(context->panel_inventory_label, label_pos);

	const char* text = stringstore_get(item->name_text_id);
	overlay_draw_string(context->panel_inventory_label, text, 2, label_pos);
	
}

//-----------------------------------------------------------------------------
void st_inventory_exit(st_inventory_context_ptr context)
{
	overlay_destroy_panel(context->panel_inventory);
	overlay_destroy_panel(context->panel_inventory_highlight);
	overlay_destroy_panel(context->panel_inventory_label);
}

//-----------------------------------------------------------------------------
void st_inventory_update(st_inventory_context_ptr context)
{
	inventory_ptr inventory = &g_main_world->persistent.inventory;
	uint8_t item_count = inventory_count(inventory);

	if (key_hit(KI_LEFT))
	{
		context->inventory_current_idx--;

		if (context->inventory_current_idx < 0)
			context->inventory_current_idx = 0;
		else
			audio_sfx_play(resources_find_audioclip(RES__SND_SELECT));
	}

	if (key_hit(KI_RIGHT))
	{
		context->inventory_current_idx++;

		if (context->inventory_current_idx >= item_count)
			context->inventory_current_idx = item_count - 1;
		else
			audio_sfx_play(resources_find_audioclip(RES__SND_SELECT));
	}


	item_ptr item = itemstore_get((uint16_t)inventory->items[context->inventory_current_idx]);

	if ((context->inventory_current_idx - 5) > context->inventory_scroll_idx)
	{
		context->inventory_scroll_idx = (context->inventory_current_idx - 5);
	}
	else if (context->inventory_current_idx < context->inventory_scroll_idx)
	{
		context->inventory_scroll_idx = context->inventory_current_idx;
	}

	point2_t panel_position = 
	{
		((graphics_screen_width - (6 * 32)) >> 1) - (context->inventory_scroll_idx * 32),
		graphics_screen_height - (32 + 4)
	};

	overlay_set_position(context->panel_inventory, panel_position);


	point2_t highlight_pos = 
	{ 
		panel_position.x + (context->inventory_current_idx * 32),
		panel_position.y
	};
	overlay_set_position(context->panel_inventory_highlight, highlight_pos);

	point2_t label_pos = 
	{
		panel_position.x + (context->inventory_current_idx * 32),
		panel_position.y - 10
	};
	overlay_set_position(context->panel_inventory_label, label_pos);

	point2_t label_text_pos = { 0, 0 };
	const char* text = stringstore_get(item->name_text_id);
	overlay_clear(context->panel_inventory_label, 0);
	overlay_draw_string(context->panel_inventory_label, text, 2, label_text_pos);
	

	st_inventory_set_highlight(context, item);

	if (key_hit(KI_A))
	{
		debug_printf(DEBUG_LOG_INFO, "st_inventory::selected item_id=%u", context->inventory_current_idx);

		audio_sfx_play(resources_find_audioclip(RES__SND_ACCEPT));

		uint32_t parameter = 0;
		parameter |= 2 << 24;						//	command type
		parameter |= context->inventory_current_idx;			//	item_id

		state_pop(parameter);
		return;
	}

	if (key_hit(KI_B))
	{
		state_pop(0);
		return;
	}
}

//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_inventory =
{
	(state_enter_func_ptr)st_inventory_enter,
	(state_exit_func_ptr)st_inventory_exit,

	NULL,
	NULL,

	(state_update_func_ptr)st_inventory_update,
	NULL,

	"st_inventory",
	sizeof(st_inventory_context_t)

};
