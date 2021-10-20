
#include "graphics.h"

#include "common/platform/gba/gba.h"	
#include "common/memory.h"
#include "common/debug/debug.h"

//	presentation - just a region of the screen?


//-----------------------------------------------------------------------------
IWRAM_CODE void graphics_vblank_interrupt()
{
	g_graphics_context.vblank_count++;
	g_graphics_context.vblank_total_count++;
}


//-----------------------------------------------------------------------------
void graphics_initialize() 
{
	/*REG_DISPCNT &= ~(0x0005 | 0x0400);
	REG_DISPCNT |= (0x0004 | 0x0400);	// Mode4 / Background2 -- */
	REG_DISPCNT = (0x0004 | 0x0400);
	REG_BG2CNT = 0x0003;

	uint16_t width = 240, height = 160;

	g_graphics_context.width = width;
	g_graphics_context.height = height;
	g_graphics_context.widthMinusOne = fixed16_from_int(g_graphics_context.width - 1);
	g_graphics_context.heightMinusOne = fixed16_from_int(g_graphics_context.height - 1);
	g_graphics_context.stridebytes = width * sizeof(uint8_t);

	g_graphics_context.palette = (uint16_t*)memory_allocate(sizeof(uint16_t) * 256, MEMORY_EWRAM);

	g_graphics_context.frame_pages[0] = (uint8_t*)0x06000000;
	g_graphics_context.frame_pages[1] = (uint8_t*)0x0600A000;
	g_graphics_context.frame_buffer = g_graphics_context.frame_pages[1];

	//	setup graphics vblank interrupt
	g_graphics_context.vblank_target = 1;

	irqSet(IRQ_VBLANK, graphics_vblank_interrupt);		//	this isn't working ...
	irqEnable(IRQ_VBLANK);

	debug_printf(DEBUG_LOG_INFO, "graphics::initialized");
}

//-----------------------------------------------------------------------------
void graphics_set_vsync(uint8_t mode)
{
	//	what are we aiming for 20fps, 30fps, 60fps
	g_graphics_context.vblank_target = mode;
}

//-----------------------------------------------------------------------------
IWRAM_CODE void graphics_vsync()
{
	//	what do we do if we have over ran ??? let the audio jitter??

	int8_t target = g_graphics_context.vblank_target;

	if (g_graphics_context.vblank_count >= target)		//	use REG_VCOUNT to determine if we should +1 
		target = g_graphics_context.vblank_count + 1;

	while (g_graphics_context.vblank_count < target);


	//while (REG_VCOUNT >= 200);   // wait till VDraw		//	REG_VCOUNT >= 200, leaves us a minimum of 28 scanlines worth of VBlank to do our present, would be nice to take advantage of this 
	//while (REG_VCOUNT < 160);    // wait till VBlank

	//	VBlank is 83,776 cycles, how many do we need ??? 


	//	assert in the presents that we actually presenting during VBlank 

			
	g_graphics_context.vblank_count = 0;
}

//-----------------------------------------------------------------------------
IWRAM_CODE void graphics_present()	
{
	//	could this be done on some interrupt to avoid vsync wasting too much cpu time 

	//	this isn't right, as we should probably try to target a constant FPS

	if (g_graphics_context.dirty_flags == 0)		
		return;

	//	palette write
	if (g_graphics_context.dirty_flags & 0x01)
	{
		uint16_t* hw_pal = (uint16_t*)0x05000000;
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
		//	if render_target is bound then shouldn't do this 

		g_graphics_context.frame_buffer = g_graphics_context.frame_pages[g_graphics_context.page_flip];
		REG_DISPCNT ^= 0x0010;

		g_graphics_context.page_flip ^= 1;

		g_graphics_context.dirty_flags &= ~0x02;
	}

	//	resolution change
	if (g_graphics_context.dirty_flags & 0x04)
	{
		fixed16_t width_factor = fixed16_div(fixed16_from_int(g_graphics_context.width), fixed16_from_int(graphics_screen_width));
		fixed16_t height_factor = fixed16_div(fixed16_from_int(g_graphics_context.height), fixed16_from_int(graphics_screen_height));

		REG_BG2PA = (width_factor >> 8) & 0xFFFF;	//	 (width * 1/240)
		REG_BG2PB = 0;
		REG_BG2PC = 0;
		REG_BG2PD = (height_factor >> 8) & 0xFFFF;	//   (height * 1/160)

		g_graphics_context.dirty_flags &= ~0x04;
	}

	debug_assert(REG_VCOUNT > 160, "graphics:present is still doing work after the VBlank window");
}

//-----------------------------------------------------------------------------
void graphicsShutdown()
{
	//	...
}
