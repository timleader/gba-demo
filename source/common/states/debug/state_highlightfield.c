
#include "states.h"

#include "common/memory.h"
#include "common/graphics/graphics.h"
#include "common/graphics/image.h"
#include "common/input/input.h"
#include "common/utils/bitstream.h"
#include "common/resources/resources.h"


//-----------------------------------------------------------------------------
typedef struct st_highlightfield_context_s
{
	highlight_field_ptr highlight;
	perf_timer_t highlight_scroll_timer; 

} st_highlightfield_context_t;

typedef st_highlightfield_context_t* st_highlightfield_context_ptr;

//-----------------------------------------------------------------------------
void st_highlightfield_enter(st_highlightfield_context_ptr context, uint32_t parameter)
{
	uint16_t resource_id = parameter & 0x0000FFFF;

	context->highlight = resources_find_highlightfield(resource_id);

	palette_ptr highlight_palette = palette_new(3, MEMORY_EWRAM);

	highlight_palette->color555[0] = RGB555(0, 0, 0);
	highlight_palette->color555[1] = RGB555(0, 0, 0);
	highlight_palette->color555[2] = RGB555(255, 0, 255);

	graphics_write_palette(highlight_palette);

	palette_delete(highlight_palette);

	context->highlight_scroll_timer = timer_start(40, TIMER_MODE_LOOP);
}

//-----------------------------------------------------------------------------
void st_highlightfield_exit(st_highlightfield_context_ptr context)
{
	memory_set(g_graphics_context.frame_buffer, g_graphics_context.width * g_graphics_context.height * sizeof(uint8_t), 1);
}

//-----------------------------------------------------------------------------
void st_highlightfield_draw(st_highlightfield_context_ptr context, fixed16_t dt)
{
	memory_set(g_graphics_context.frame_buffer, g_graphics_context.width * g_graphics_context.height * sizeof(uint8_t), 1);

	fixed16_t scroll_progress = timer_progress(context->highlight_scroll_timer);

	int32_t start = fixed16_to_int(fixed16_mul(scroll_progress, fixed16_from_int(graphics_screen_height)));
	int32_t end = start + 8;

	graphics_draw_highlightfield(context->highlight, 2, start, end);

	graphics_pageflip();
}

//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_highlightfield =
{
	(state_enter_func_ptr)st_highlightfield_enter,
	(state_exit_func_ptr)st_highlightfield_exit,

	NULL,
	NULL,

	(state_update_func_ptr)st_common_update,
	(state_draw_func_ptr)st_highlightfield_draw,

	"st_highlight",
	sizeof(st_highlightfield_context_t)

};
