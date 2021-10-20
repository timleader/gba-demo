
#include "games/7days/states/states.h"

#include "common/memory.h"
#include "common/graphics/graphics.h"
#include "common/graphics/overlay.h"
#include "common/resources/resources.h"
#include "common/utils/timer.h"
#include "common/math/easing.h"
#include "common/debug/debug.h"

#include "package_7days.h"

//-----------------------------------------------------------------------------
typedef enum st_splash_state_e
{
	ST_SPLASH_PAUSE,
	ST_SPLASH_FADE_OUT

} st_splash_state_t;

//-----------------------------------------------------------------------------
typedef struct st_splash_context_s
{
	palette_ptr bg_origin_palette;
	palette_ptr bg_palette;
	image_ptr bg_image;

	perf_timer_t bg_timer;

	st_splash_state_t bg_inner_state;

} st_splash_context_t;

typedef st_splash_context_t* st_splash_context_ptr;


//-----------------------------------------------------------------------------
void st_splash_enter(st_splash_context_ptr context, uint32_t parameter)
{
	context->bg_image = resources_find_image(RES__IMG_SPLASH);

	context->bg_origin_palette = resources_find_palette(context->bg_image->palette_id);
	context->bg_palette = palette_new_from_copy(context->bg_origin_palette, MEMORY_EWRAM);

	memory_dma_copy32(g_graphics_context.frame_buffer, context->bg_image->data, (g_graphics_context.width * g_graphics_context.height * sizeof(uint8_t)) >> 2);

	graphics_write_palette(context->bg_palette);

	context->bg_timer = timer_start(60, TIMER_MODE_ONCE);
	context->bg_inner_state = ST_SPLASH_PAUSE;

	graphics_pageflip();
}

//-----------------------------------------------------------------------------
void st_splash_exit(st_splash_context_ptr context)
{
	memory_free(context->bg_palette);

	context->bg_image = NULL;
	context->bg_palette = NULL;
}

//-----------------------------------------------------------------------------
void st_splash_update(st_splash_context_ptr context, fixed16_t t)		
{
	switch (context->bg_inner_state)
	{
	case ST_SPLASH_PAUSE:
		if (timer_expired(context->bg_timer))
		{
			context->bg_timer = timer_start(60, TIMER_MODE_ONCE);
			context->bg_inner_state = ST_SPLASH_FADE_OUT;
		}
		break;
	case ST_SPLASH_FADE_OUT:
		{
			fixed16_t t = timer_progress(context->bg_timer);
			t = math_easeinquad(t);
			
			palette_lerp(context->bg_origin_palette, context->bg_palette, RGB555(0, 0, 0), t);

			// GBA Blending would provide a more efficient fade to black

			graphics_write_palette(context->bg_palette);

			if (timer_expired(context->bg_timer))
			{
				state_goto(&st_language_select, 0);
			}
			break;
		}
	}
}

//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_splash =
{
	(state_enter_func_ptr)st_splash_enter,
	(state_exit_func_ptr)st_splash_exit,

	NULL,
	NULL,

	(state_update_func_ptr)st_splash_update,
	NULL,

	"st_splash",
	sizeof(st_splash_context_t)
};

