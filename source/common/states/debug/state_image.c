
#include "states.h"

#include "common/resources/resources.h"
#include "common/graphics/graphics.h"
#include "common/graphics/overlay.h"
#include "common/graphics/image.h"
#include "common/input/input.h"
#include "common/compression/lz77.h"
#include "common/utils/profiler.h"

#include <stdio.h>		//	try to remove this 

//-----------------------------------------------------------------------------
typedef struct st_image_context_s
{
	uint8_t st_image_panel_id;
	uint8_t reserved[3];

	uint16_t st_image_resource_id;
	int16_t st_image_frame_idx;

	int32_t vsync_target;

} st_image_context_t;

typedef st_image_context_t* st_image_context_ptr;

//-----------------------------------------------------------------------------
void st_image_draw_image(st_image_context_ptr context, uint16_t resource_id, int16_t frame_idx)
{
	image_ptr image = resources_find_image(context->st_image_resource_id);
	palette_ptr palette = resources_find_palette(image->palette_id);

	graphics_write_palette(palette);

	uint16_t x = (graphics_screen_width - image->width) >> 1;	//	use consts for screen resolution
	uint16_t y = (graphics_screen_height - image->height) >> 1;

	graphics_clear(0);

	if (image->format)
	{
		//	decompress into VRAM ? 
		uint8_t* uncompressed_data = memory_allocate(image->size, MEMORY_EWRAM);	

		uint32_t remaining = (*((uint32_t*)image->data) & 0xFFFFFF00) >> 8;
		debug_printf(DEBUG_LOG_INFO, "lz77::decode remaining=%i", remaining);

		profiler_sample_handle_t phandle;
		phandle = profiler_begin("lz77:decode");

		lz77_decompress(image->data, uncompressed_data);	

		profiler_end(phandle);

		memory_copy32(g_graphics_context.frame_buffer, uncompressed_data, image->size >> 2);		//	decode straight into frame_buffer ???

		memory_free(uncompressed_data);
	}
	else
	{
		graphics_draw_image(image, frame_idx, x, y);
	}

	graphics_pageflip();

	point2_t position = { 16, 0 };
	point2_t size = { 6, 1 };
	overlay_draw_color(context->st_image_panel_id, 0, position, size);

	char frame_count_text[32];
	sprintf(frame_count_text, "%i / %u", (int)frame_idx, (unsigned int)image->frame_count);
	overlay_draw_string(context->st_image_panel_id, frame_count_text, 1, position);
}

//-----------------------------------------------------------------------------
void st_image_enter(st_image_context_ptr context, uint32_t parameter)
{
	context->st_image_resource_id = parameter & 0x0000FFFF;
	context->st_image_frame_idx = 0;

	//	have a common function for this -??
	palette_ptr ui_palette = resources_find_palette_from_name("pal/ui/default");
	overlay_write_palette(ui_palette);

	context->st_image_panel_id = overlay_create_panel(8, 2);
	point2_t position = { 0, 0 };
	overlay_set_position(context->st_image_panel_id, position);

	overlay_clear(context->st_image_panel_id, 0);

	const char* resource_name = resources_get_name(context->st_image_resource_id);
	overlay_draw_string(context->st_image_panel_id, resource_name, 1, position);

	st_image_draw_image(context, context->st_image_resource_id, context->st_image_frame_idx);

	context->vsync_target = 1;
}

//-----------------------------------------------------------------------------
void st_image_exit(st_image_context_ptr context)
{
	overlay_destroy_panel(context->st_image_panel_id);
}

//-----------------------------------------------------------------------------
void st_image_update(st_image_context_ptr context, fixed16_t dt)
{
	if (key_hit(KI_B))
	{
		state_pop(0);
	}

	if (key_hit(KI_SELECT))
	{
		state_push(&st_analysis, 0);
	}

	if (key_hit(KI_LEFT))
	{
		if (--context->st_image_frame_idx < 0)
			context->st_image_frame_idx = 0;

		st_image_draw_image(context, context->st_image_resource_id, context->st_image_frame_idx);
	}
	else if (key_hit(KI_RIGHT))
	{
		if (++context->st_image_frame_idx > 20)
			context->st_image_frame_idx = 20;

		st_image_draw_image(context, context->st_image_resource_id, context->st_image_frame_idx);
	}

	if (key_hit(KI_UP))
	{
		if (++context->vsync_target > 3)
			context->vsync_target = 3;

		graphics_set_vsync(context->vsync_target);
	}
	else if (key_hit(KI_DOWN))
	{
		if (--context->vsync_target < 1)
			context->vsync_target = 1;

		graphics_set_vsync(context->vsync_target);
	}
}

//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_image =
{
	(state_enter_func_ptr)st_image_enter,
	(state_exit_func_ptr)st_image_exit,

	NULL,
	NULL,

	(state_update_func_ptr)st_image_update,
	NULL,

	"st_image",
	sizeof(st_image_context_t)

};
