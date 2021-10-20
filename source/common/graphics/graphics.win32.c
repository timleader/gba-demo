
#include "graphics.h"

#include "common/memory.h"
#include "common/debug/debug.h"

#include <SDL.h>


static int32_t g_vsync_current_time;
static int32_t g_vsync_last_time;

uint16_t g_graphics_palette[256];

//-----------------------------------------------------------------------------
void graphics_initialize()	
{
	uint16_t width = 240, height = 160;

	g_graphics_context.width = width;
	g_graphics_context.height = height;
	g_graphics_context.widthMinusOne = F16(g_graphics_context.width - 1);
	g_graphics_context.heightMinusOne = F16(g_graphics_context.height - 1);
	g_graphics_context.stridebytes = width * sizeof(uint8_t);

	g_graphics_context.palette = (uint16_t*)memory_allocate(sizeof(uint16_t) * 256, MEMORY_EWRAM);

	g_graphics_context.frame_pages[0] = (uint8_t*)memory_allocate(g_graphics_context.width * g_graphics_context.height * sizeof(uint8_t), MEMORY_DEVELOPMENT);
	g_graphics_context.frame_pages[1] = (uint8_t*)memory_allocate(g_graphics_context.width * g_graphics_context.height * sizeof(uint8_t), MEMORY_DEVELOPMENT);
	g_graphics_context.frame_buffer = g_graphics_context.frame_pages[1];

	g_graphics_context.vblank_total_count = 0;
	g_graphics_context.vblank_target = 1;

	//	Engine Loop
	g_vsync_current_time = SDL_GetTicks();
	g_vsync_last_time = g_vsync_current_time;

	debug_printf(DEBUG_LOG_INFO, "graphics::initialized");
}

//-----------------------------------------------------------------------------
void graphics_set_vsync(uint8_t mode)
{
	//	what are we aiming for 20fps, 30fps, 60fps
	g_graphics_context.vblank_target = mode;
}

//-----------------------------------------------------------------------------
void graphics_vsync()
{
	int8_t target_ms = g_graphics_context.vblank_target * 16;

	int32_t waiting_time_ms = 0;
	while ((waiting_time_ms = ((g_vsync_current_time = SDL_GetTicks()) - g_vsync_last_time)) < target_ms)
		SDL_Delay(waiting_time_ms);

	g_vsync_last_time = g_vsync_current_time;

	g_graphics_context.vblank_total_count += g_graphics_context.vblank_target;
}

//-----------------------------------------------------------------------------
void graphics_present()
{
	if (g_graphics_context.dirty_flags == 0)
		return;

	//	palette write
	if (g_graphics_context.dirty_flags & 0x01)
	{
		uint16_t* hw_pal = (uint16_t*)g_graphics_palette;		//	hardware palette
		uint16_t idx = 0;
		while (idx < 256)
		{
			hw_pal[idx] = g_graphics_context.palette[idx];
			idx++;
		}
		g_graphics_context.dirty_flags &= ~0x01;
	}

	//	page flip
	if (g_graphics_context.dirty_flags & 0x02)
	{
		g_graphics_context.frame_buffer = g_graphics_context.frame_pages[g_graphics_context.page_flip];
		g_graphics_context.page_flip ^= 1;

		g_graphics_context.dirty_flags &= ~0x02;
	}

	//	resolution change
	if (g_graphics_context.dirty_flags & 0x04)
	{
		//	no need to do anything on pc 

		g_graphics_context.dirty_flags &= ~0x04;
	}
}

//-----------------------------------------------------------------------------
void graphicsShutdown()
{
	memory_free(g_graphics_context.palette);

	memory_free(g_graphics_context.frame_pages[0]);
	memory_free(g_graphics_context.frame_pages[1]);
}
