
#include "overlay.h"

#include "common/platform/gba/gba.h"	//	have my own gba.h
#include "common/memory.h"
#include "common/debug/debug.h"

/*
	If PageFlipping is active we need to limit tiles to top half 
	of VRAM, so can only use tile 512 -> 1024

	However if PageFlipping isn't active we can use 0 -> 1024

	Actually have to go to Mode 0 -> 2 to enable these additional sprites...

	Should we even use Sprites ??? 

	Maybe dialogue sys should use more of the graphics sys ?? 


	-----------------------------------------------

	solution:
		+ overlay to use 4BPP , this will unlock the required amount of space we need
		+ convert tiledimage to all be 4BPP 
	
*/

void overlay_initialize(void)
{
	//	setup sprites to point to tiles in order ... 
	REG_DISPCNT |= 0x1000 | 0x0040;//;DCNT_OBJ | DCNT_OBJ_1D;

	//	hide all sprites


	//	double buffer all of this, 
	//		write to EWRAM, 
	//		during vsync copy to OVRAM, etc...
	//			(can this be done in time)
	//	
	//		dirty tracking so we can minimize memory transfer
	//

	g_overlaybuffer.hw_pal = (color555_t*)0x05000200;	//	512 bytes 	
	g_overlaybuffer.hw_ovram = (tile_t*)0x06010000;		//	16,384 bytes, we only use top half of this
	g_overlaybuffer.hw_oam = (obj_attr_t*)0x07000000;	//	(sizeof(obj_attr_t) * 128) bytes

	g_overlaybuffer.ewram_pal = (color555_t*)memory_allocate(sizeof(color555_t) * 256, MEMORY_EWRAM);
	g_overlaybuffer.ewram_ovram = (tile_t*)memory_allocate(sizeof(uint16_t) * (256 * 8 * 8), MEMORY_EWRAM);	//	half this 
	g_overlaybuffer.ewram_oam = (obj_attr_t*)memory_allocate(sizeof(obj_attr_t) * SPRITE_COUNT, MEMORY_EWRAM);

	g_overlaybuffer.dirty_mask = OVERLAY_DIRTY_OAM;
	g_overlaybuffer.palette_dirty_span.start = -1;
	g_overlaybuffer.palette_dirty_span.end = -1;
	g_overlaybuffer.ovram_dirty_span.start = -1;
	g_overlaybuffer.ovram_dirty_span.end = -1;
	g_overlaybuffer.oam_dirty_span.start = 0;
	g_overlaybuffer.oam_dirty_span.end = SPRITE_COUNT;

	g_overlaybuffer.panels = (panel_t*)memory_allocate(sizeof(panel_t) * PANEL_COUNT, MEMORY_EWRAM);
	g_overlaybuffer.base_tile_idx = 512;	//	this is going to fuck with things

	// I think we can get away with half the sprite in bitmap mode 

	uint16_t obj_idx = 0;
	do
	{
		obj_set_attr(
			&g_overlaybuffer.ewram_oam[obj_idx],
			ATTR0_WIDE | ATTR0_HIDE | ATTR0_4BPP,	
			ATTR1_SIZE_32x8,
			g_overlaybuffer.base_tile_idx + (obj_idx * TILES_PER_SPRITE));

	} while (++obj_idx < SPRITE_COUNT);

	debug_printf(DEBUG_LOG_INFO, "overlay::initialized");
}

void overlay_shutdown(void)
{
	memory_free(g_overlaybuffer.ewram_pal);
	memory_free(g_overlaybuffer.ewram_ovram);
	memory_free(g_overlaybuffer.ewram_oam);

	memory_free(g_overlaybuffer.panels);
}
