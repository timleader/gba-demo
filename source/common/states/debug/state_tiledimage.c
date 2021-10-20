
#include "states.h"

#include "common/graphics/graphics.h"
#include "common/graphics/overlay.h"
#include "common/graphics/image.h"
#include "common/input/input.h"
#include "common/resources/resources.h"


//-----------------------------------------------------------------------------
typedef struct st_tiledimage_context_s
{
	uint8_t panel_tiledimage;
	uint8_t reserved[3];

} st_tiledimage_context_t;

typedef st_tiledimage_context_t* st_tiledimage_context_ptr;


//-----------------------------------------------------------------------------
void st_tiledimage_enter(st_tiledimage_context_ptr context, uint32_t parameter)
{
	uint16_t resource_id = parameter & 0x0000FFFF;

	tiledimage_ptr image = resources_find_tiledimage(resource_id);
	palette_ptr palette = resources_find_palette(image->palette_id);

	overlay_write_palette(palette);

	uint8_t assumed_palette_bank = palette->offset >> 4;

	uint8_t panel_width = (image->tile_width >> 2);
	if (image->tile_width & 0x03)
		panel_width++;
	 
	context->panel_tiledimage = overlay_create_panel(panel_width, image->tile_height);	//	need to round up
	overlay_set_palette_bank(context->panel_tiledimage, assumed_palette_bank);

	uint16_t x = (240 - (image->tile_width * 8)) >> 1;	
	uint16_t y = (160 - (image->tile_height * 8)) >> 1;

	point2_t pos = { x, y };

	overlay_set_position(context->panel_tiledimage, pos);

	pos.x = 0;
	pos.y = 0;

	overlay_clear(context->panel_tiledimage, 0);

	overlay_draw_tiledimage(context->panel_tiledimage, image, pos);

	graphics_clear(0);
}

//-----------------------------------------------------------------------------
void st_tiledimage_exit(st_tiledimage_context_ptr context)
{
	overlay_destroy_panel(context->panel_tiledimage);
}

//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_tiledimage =
{
	(state_enter_func_ptr)st_tiledimage_enter,
	(state_exit_func_ptr)st_tiledimage_exit,

	NULL,
	NULL,

	(state_update_func_ptr)st_common_update,
	NULL,

	"st_tiledimg",
	sizeof(st_tiledimage_context_t)

};
