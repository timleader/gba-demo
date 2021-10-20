
#include "overlay.h"

#include "graphics.h"

#include "common/debug/debug.h"
#include "common/memory.h"

void overlay_initialize(void)
{
	//	begin -- windows specific
	g_overlaybuffer.hw_pal = (color555_t*)memory_allocate(sizeof(color555_t) * 256, MEMORY_DEVELOPMENT);
	g_overlaybuffer.hw_ovram = (tile_t*)memory_allocate(OVERLAY_OVRAM_SIZE_IN_BYTES, MEMORY_DEVELOPMENT);
	g_overlaybuffer.hw_oam = (obj_attr_t*)memory_allocate(sizeof(obj_attr_t) * SPRITE_COUNT, MEMORY_DEVELOPMENT);
	//	end -- windows specific

	//	generalized
	g_overlaybuffer.ewram_pal = (color555_t*)memory_allocate(sizeof(color555_t) * 256, MEMORY_EWRAM);
	g_overlaybuffer.ewram_ovram = (tile_t*)memory_allocate(OVERLAY_OVRAM_SIZE_IN_BYTES, MEMORY_EWRAM);
	g_overlaybuffer.ewram_oam = (obj_attr_t*)memory_allocate(sizeof(obj_attr_t) * SPRITE_COUNT, MEMORY_EWRAM);

	g_overlaybuffer.dirty_mask = OVERLAY_DIRTY_OAM;
	g_overlaybuffer.palette_dirty_span.start = -1;
	g_overlaybuffer.palette_dirty_span.end = -1;
	g_overlaybuffer.ovram_dirty_span.start = -1;
	g_overlaybuffer.ovram_dirty_span.end = -1;
	g_overlaybuffer.oam_dirty_span.start = 0;
	g_overlaybuffer.oam_dirty_span.end = SPRITE_COUNT;

	g_overlaybuffer.panels = (panel_t*)memory_allocate(sizeof(panel_t) * PANEL_COUNT, MEMORY_EWRAM);
	g_overlaybuffer.base_tile_idx = 512;

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
	memory_free(g_overlaybuffer.hw_pal);
	memory_free(g_overlaybuffer.hw_ovram);
	memory_free(g_overlaybuffer.hw_oam);

	memory_free(g_overlaybuffer.ewram_pal);
	memory_free(g_overlaybuffer.ewram_ovram);
	memory_free(g_overlaybuffer.ewram_oam);

	memory_free(g_overlaybuffer.panels);
}

//	this should be named differently, this should probably be in some hw emulation file ... 
void overlay_emulate_gba_present(uint16_t* sdlTextureData)	//	don't need to optimize this any further as it is a win32 only thing
{
	//	begin -- windows specific (gba sprite hw)
	obj_attr_t obj_copy;
	uint16_t obj_idx = 0;

	uint32_t* ovram = (uint32_t*)(&g_overlaybuffer.hw_ovram[0]);

	do
	{
		memory_copy(&obj_copy, &g_overlaybuffer.hw_oam[obj_idx], sizeof(obj_attr_t));

		if (BFN_GET2(obj_copy.attr0, ATTR0_MODE) == ATTR0_HIDE)
			continue;

		uint8_t palette_bank = BFN_GET(obj_copy.attr2, ATTR2_PALBANK);
		palette_bank <<= 4;

		uint16_t tile_idx_off = obj_copy.attr2 & ATTR2_ID_MASK;

		point2_t obj_position;
		obj_position.x = BFN_GET(obj_copy.attr1, ATTR1_X);
		obj_position.y = BFN_GET(obj_copy.attr0, ATTR0_Y);

		if (obj_position.x & (1 << 8))
		{
			obj_position.x ^= (1 << 8);
			obj_position.x -= 256;
		}

		/*
		if (obj_position.y & (1 << 7))
		{
			obj_position.y ^= (1 << 7);
			obj_position.y *= -1;
		}
		*/
		int32_t x = obj_position.x;
		int32_t row_offset = (obj_position.y * g_graphics_context.stridebytes);

		uint16_t* initialDestPtr = sdlTextureData + row_offset;

		/*
			designed to only handle 32x8 sprites 
				tiles 8x8

				tile size in bytes -> 8x8 => 64 pixels {4BPP => 32bytes}
		*/
		for (int tile_idx = 0; tile_idx < 4; ++tile_idx)	//	looping over tiles
		{
			//	get tile_t 

			for (int row = 0; row < 8; ++row)	//	looping over the rows within the tiles
			{
				if (obj_position.y + row < 0 ||
					obj_position.y + row >= 160)
				{
					continue;
				}

				//	offset by sprite_position
				uint16_t* destPtr = initialDestPtr + (row * g_graphics_context.stridebytes) + x;

				uint32_t p_row = ovram[((tile_idx_off + tile_idx) * (TILE_SIZE_IN_BYTES >> 2)) + row];

				for (int src_x = 0; src_x < 8; ++src_x)		//	unroll this  -> int32 * 2
				{
					if (x + src_x > fixed16_to_int(g_graphics_context.widthMinusOne) ||
						x + src_x < 0)
					{
						destPtr++;
					}
					else
					{
						if (p_row & 0x0F)
						{
							*destPtr++ = g_overlaybuffer.hw_pal[palette_bank | (p_row & 0x0F)] | (1 << 15);	//	add alpha of 1
						}
						else
						{
							destPtr++;
						}
					}

					p_row >>= 4;
				}
			}
			x += 8;
		}

	} while (++obj_idx < SPRITE_COUNT);
}
