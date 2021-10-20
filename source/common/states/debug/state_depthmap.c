
#include "states.h"

#include "common/memory.h"
#include "common/graphics/graphics.h"
#include "common/graphics/image.h"
#include "common/input/input.h"
#include "common/utils/bitstream.h"
#include "common/resources/resources.h"


//-----------------------------------------------------------------------------
typedef struct st_depthmap_context_s
{
	uint32_t reserved;

} st_depthmap_context_t;

typedef st_depthmap_context_t* st_depthmap_context_ptr;

//-----------------------------------------------------------------------------
void st_depthmap_enter(st_depthmap_context_ptr context, uint32_t parameter)
{
	uint16_t resource_id = parameter & 0x0000FFFF;

	depthmap_ptr depthmap = resources_find_depthmap(resource_id);
	
	palette_ptr depthmap_palette = (palette_ptr)memory_allocate(sizeof(palette_t) + (4 * sizeof(uint16_t)), MEMORY_EWRAM);
	
	depthmap_palette->id = 0;
	depthmap_palette->offset = 0;
	depthmap_palette->size = 4;

	depthmap_palette->color555[0] = RGB555(0, 0, 0);
	depthmap_palette->color555[1] = RGB555(85, 85, 85);
	depthmap_palette->color555[2] = RGB555(170, 170, 170);
	depthmap_palette->color555[3] = RGB555(255, 255, 255);

	graphics_write_palette(depthmap_palette);

	memory_free(depthmap_palette);


	memory_set(g_graphics_context.frame_buffer, 0, g_graphics_context.width * g_graphics_context.height * sizeof(uint8_t));

	uint16_t x_off = (graphics_screen_width - depthmap->width) >> 1;
	uint16_t y_off = (graphics_screen_height - depthmap->height) >> 1;

	bitstream_t stream;
	bitstream_initialize(&stream, (const void_ptr)depthmap->data, depthmap->size);

	for (uint16_t row = 0; row < depthmap->height; ++row)
	{
		uint8_t* dest = g_graphics_context.frame_buffer + ((row + y_off) * g_graphics_context.width) + x_off;

		for (uint16_t x = 0; x < depthmap->width; ++x)
		{
			uint8_t color_idx = 0;
			color_idx |= bitstream_read_1(&stream);
			color_idx |= bitstream_read_1(&stream) << 1;

			*dest++ = color_idx;
		}
	}

	graphics_pageflip();
}

//-----------------------------------------------------------------------------
void st_depthmap_exit(st_depthmap_context_ptr context)
{
	memory_set(g_graphics_context.frame_buffer, 0, g_graphics_context.width * g_graphics_context.height * sizeof(uint8_t));
}


//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_depthmap =
{
	(state_enter_func_ptr)st_depthmap_enter,
	(state_exit_func_ptr)st_depthmap_exit,

	NULL,
	NULL,

	(state_update_func_ptr)st_common_update,
	NULL,

	"st_depthmap",
	sizeof(st_depthmap_context_t)

};
