
#include "overlay.h"

#include "text.h"
#include "common/memory.h"
#include "common/debug/debug.h"

overlay_buffer_t g_overlaybuffer = { 0, 0, 0, 0, 0 };

//	DIRTY TRACKING

void overlay_palette_mark_as_dirty(const uint16_t offset, const uint16_t size)
{
	debug_assert((offset + size) <= 256, "overlay::palette_mark_as_dirty palette exceeds 256");

	span_t* palette_dirty_span = &g_overlaybuffer.palette_dirty_span;

	int16_t new_end_pos = offset + size;

	if (palette_dirty_span->start == -1 ||
		offset < palette_dirty_span->start)
	{
		palette_dirty_span->start = offset;
	}

	if (palette_dirty_span->end == -1 ||
		new_end_pos > palette_dirty_span->end)
	{
		palette_dirty_span->end = new_end_pos;
	}

	g_overlaybuffer.dirty_mask |= OVERLAY_DIRTY_PALETTE;
}

void overlay_ovram_mark_as_dirty(const uint16_t offset, const uint16_t size)
{
	debug_assert((offset + size) < (OVERLAY_OVRAM_SIZE_IN_BYTES >> 1), "overlay::ovram_mark_as_dirty ovram exceeds size");

	span_t* ovram_dirty_span = &g_overlaybuffer.ovram_dirty_span;

	int16_t new_end_pos = offset + size;

	if (ovram_dirty_span->start == -1 ||
		offset < ovram_dirty_span->start)
	{
		ovram_dirty_span->start = offset;
	}

	if (ovram_dirty_span->end == -1 ||
		new_end_pos > ovram_dirty_span->end)
	{
		ovram_dirty_span->end = new_end_pos;
	}

	g_overlaybuffer.dirty_mask |= OVERLAY_DIRTY_OVRAM;
}

void overlay_oam_mark_as_dirty(const uint16_t offset, const uint16_t size)
{
	debug_assert((offset + size) < SPRITE_COUNT, "overlay::oam_mark_as_dirty sprite exceeds count");

	span_t* oam_dirty_span = &g_overlaybuffer.oam_dirty_span;

	int16_t new_end_pos = offset + size;

	if (oam_dirty_span->start == -1 ||
		offset < oam_dirty_span->start)
	{
		oam_dirty_span->start = offset;
	}

	if (oam_dirty_span->end == -1 ||
		new_end_pos > oam_dirty_span->end)
	{
		oam_dirty_span->end = new_end_pos;
	}

	g_overlaybuffer.dirty_mask |= OVERLAY_DIRTY_OAM;
}

//	DIRTY TRACKING


int8_t overlay_create_panel(uint8_t tile_width, uint8_t tile_height)	//pass palette bank idx
{
	//	find a panel to use otherwise fail
	int8_t panel_idx = -1;
	int16_t idx = 0;
	for (; idx < PANEL_COUNT; ++idx)
	{
		if ((g_overlaybuffer.panels[idx].attributes & 0x01) == 0)
		{
			panel_idx = idx;
			break;
		}
	}

	uint16_t obj_target_count = tile_width * tile_height;
	uint8_t obj_count = 0;
	int16_t obj_idx = -1;
	idx = 0;
	do
	{
		if (BFN_GET2(g_overlaybuffer.ewram_oam[idx].attr0, ATTR0_MODE) == ATTR0_HIDE)
		{
			if (obj_idx == -1)
				obj_idx = idx;

			obj_count++;
		}
		else
		{
			obj_idx = -1;
			obj_count = 0;
		}

		if (obj_count == obj_target_count)
		{
			break;
		}

	} while (++idx < SPRITE_COUNT);

	if (obj_count == obj_target_count)
	{
		panel_t* panel = &g_overlaybuffer.panels[panel_idx];
		panel->sprite_idx = (uint8_t)obj_idx;
		panel->sprite_width = tile_width;
		panel->sprite_height = tile_height;
		panel->tile_width = tile_width * TILES_PER_SPRITE;
		panel->tile_height = tile_height;
		panel->sprite_count = obj_count;
		panel->position.x = 0;
		panel->position.y = 0;

		idx = obj_idx;
		uint16_t obj_endidx = obj_idx + obj_count;
		do
		{
			obj_set_attr(
				&g_overlaybuffer.ewram_oam[idx],
				ATTR0_WIDE | ATTR0_4BPP,
				ATTR1_SIZE_32x8,
				g_overlaybuffer.base_tile_idx + (idx * TILES_PER_SPRITE));

		} while (++idx < obj_endidx);

		for (int y = 0; y < tile_height; ++y)
		{
			for (int x = 0; x < tile_width; ++x)
			{
				obj_set_pos(
					&g_overlaybuffer.ewram_oam[obj_idx + (y * tile_width) + x],
					x * SPRITE_WIDTH,
					y * SPRITE_HEIGHT);
			}
		}

		overlay_oam_mark_as_dirty(obj_idx, obj_count);

		panel->attributes = 0x01;
	}
	else
	{
		panel_idx = -1;
		debug_assert(0, "overlay:create_panel failed due to insufficent resources");
	}

	return panel_idx;
}

void overlay_destroy_panel(uint8_t panel_id)
{
	panel_t* panel = &g_overlaybuffer.panels[panel_id];

	uint16_t obj_idx = panel->sprite_idx;
	uint16_t obj_count = panel->sprite_count;

	uint16_t idx = obj_idx;
	uint16_t obj_endidx = obj_idx + obj_count;
	do
	{
		obj_set_attr(
			&g_overlaybuffer.ewram_oam[idx],
			ATTR0_WIDE | ATTR0_HIDE | ATTR0_4BPP,
			ATTR1_SIZE_32x8,
			g_overlaybuffer.base_tile_idx + (idx * TILES_PER_SPRITE));

	} while (++idx < obj_endidx);

	overlay_oam_mark_as_dirty(obj_idx, obj_count);

	panel->attributes = 0x00;
}

void overlay_set_palette_bank(uint8_t panel_id, uint8_t palette_bank)
{
	panel_t* panel = &g_overlaybuffer.panels[panel_id];

	uint16_t obj_idx = panel->sprite_idx;
	uint16_t obj_count = panel->sprite_count;

	uint16_t idx = obj_idx;
	uint16_t obj_endidx = obj_idx + obj_count;
	do
	{
		BFN_SET(g_overlaybuffer.ewram_oam[idx].attr2, palette_bank & 0x0F, ATTR2_PALBANK);

	} while (++idx < obj_endidx);

	overlay_oam_mark_as_dirty(obj_idx, obj_count);
}

void overlay_set_priority(uint8_t panel_id, uint8_t priority)
{
	panel_t* panel = &g_overlaybuffer.panels[panel_id];

	uint16_t obj_idx = panel->sprite_idx;
	uint16_t obj_count = panel->sprite_count;

	uint16_t idx = obj_idx;
	uint16_t obj_endidx = obj_idx + obj_count;
	do
	{
		BFN_SET(g_overlaybuffer.ewram_oam[idx].attr2, priority & 0x03, ATTR2_PRIO);

	} while (++idx < obj_endidx);

	overlay_oam_mark_as_dirty(obj_idx, obj_count);
}

void overlay_set_position(uint8_t panel_id, point2_t position)
{
	g_overlaybuffer.panels[panel_id].position.x = position.x;
	g_overlaybuffer.panels[panel_id].position.y = position.y;

	uint16_t obj_idx = g_overlaybuffer.panels[panel_id].sprite_idx;
	uint16_t obj_count = g_overlaybuffer.panels[panel_id].sprite_count;
	uint16_t tile_height = g_overlaybuffer.panels[panel_id].sprite_height;
	uint16_t tile_width = g_overlaybuffer.panels[panel_id].sprite_width;

	for (int16_t y = 0; y < tile_height; ++y)
	{
		for (int16_t x = 0; x < tile_width; ++x)
		{
			obj_set_pos(
				&g_overlaybuffer.ewram_oam[obj_idx + (y * tile_width) + x],
				position.x + (x * SPRITE_WIDTH),
				position.y + (y * SPRITE_HEIGHT));
		}
	}

	overlay_oam_mark_as_dirty(obj_idx, obj_count);
}

//  void overlay_draw_tiledimage(uint8_t panel_id, tiledimage_t* image, point2_t block_position)

//	position relative to panel origin
//		should image be forced to be aligned to 8x8 blocks, same for position ??? do this !!! 
void overlay_draw_tiledimage(uint8_t panel_id, tiledimage_ptr image, point2_t block_position)			//	 OPTIMIZE
{
	debug_assert(image != NULL, "overlay::draw_tiledimage image is NULL");

	obj_attr_t* sPtr = &g_overlaybuffer.ewram_oam[g_overlaybuffer.panels[panel_id].sprite_idx];
	//	get the sprites for this panel, 
	//	need to know what sprite occupies which position

	tile_ptr src = image->tiles;

	uint16_t tile_idx = sPtr[0].attr2 & ATTR2_ID_MASK;
	uint16_t base_idx = tile_idx + (block_position.y * g_overlaybuffer.panels[panel_id].tile_width) + block_position.x;

	for (uint16_t row = 0; row < image->tile_height; ++row)
	{
		uint16_t idx = base_idx + (row * g_overlaybuffer.panels[panel_id].tile_width);
		tile_t* dest = &g_overlaybuffer.ewram_ovram[idx];

		for (uint16_t p = 0; p < image->tile_width; ++p)
			*dest++ = *src++;
	}
	
	//	MARK AS DIRTY
	uint16_t tile_start = base_idx;
	uint16_t tile_count = image->tile_height * g_overlaybuffer.panels[panel_id].tile_width;
	overlay_ovram_mark_as_dirty(tile_start, tile_count);


	//	clamp - bounds check  --- simplify the fuck out of this 
}

void overlay_draw_tiledimage_nineslice(uint8_t panel_id, tiledimage_ptr image, point2_t block_position, point2_t block_size)	
{
	debug_assert(image != NULL, "overlay::draw_tiledimage_nineslice image is NULL");

	//	assert the image is the correct number of blocks

	obj_attr_t* sPtr = &g_overlaybuffer.ewram_oam[g_overlaybuffer.panels[panel_id].sprite_idx];
	//	get the sprites for this panel, 
	//	need to know what sprite occupies which position

	tile_ptr src = &image->tiles[0];

	uint16_t tile_idx = sPtr[0].attr2 & ATTR2_ID_MASK;
	uint16_t base_idx = tile_idx + (block_position.y * g_overlaybuffer.panels[panel_id].tile_width) + block_position.x;

	/*
		Each corner (4)
		Each edge strip
		Fill center 
	*/

	uint16_t idx = base_idx;
	tile_ptr dest = &g_overlaybuffer.ewram_ovram[idx];

	//	top row
	*dest++ = *src++;

	for (uint16_t blockIdx = 1; blockIdx < block_size.x - 1; ++blockIdx)
		*dest++ = *src;
	src++;

	*dest++ = *src++;

	//	middle rows 
	for (uint16_t row = 1; row < block_size.y - 2; ++row)
	{
		idx = base_idx + (row * g_overlaybuffer.panels[panel_id].tile_width);
		dest = &g_overlaybuffer.ewram_ovram[idx];
		src = &image->tiles[3];

		*dest++ = *src++;

		for (uint16_t blockIdx = 1; blockIdx < block_size.x - 1; ++blockIdx)
			*dest++ = *src;
		src++;

		*dest++ = *src++;
	}

	//	bottom row 
	idx = base_idx + ((block_size.y - 2) * g_overlaybuffer.panels[panel_id].tile_width);
	dest = &g_overlaybuffer.ewram_ovram[idx];

	*dest++ = *src++;

	for (uint16_t blockIdx = 1; blockIdx < block_size.x - 1; ++blockIdx)
		*dest++ = *src;
	src++;

	*dest++ = *src++;

	//	MARK AS DIRTY
	uint16_t tile_start = base_idx;
	uint16_t tile_count = image->tile_height * g_overlaybuffer.panels[panel_id].tile_width;
	overlay_ovram_mark_as_dirty(tile_start, tile_count);

	//	clamp - bounds check  --- simplify the fuck out of this 
}

void overlay_draw_tile(uint8_t panel_id, tile_ptr tile, point2_t block_position, point2_t size)
{
	obj_attr_t* sPtr = &g_overlaybuffer.ewram_oam[g_overlaybuffer.panels[panel_id].sprite_idx];
	//	get the sprites for this panel, 
	//	need to know what sprite occupies which position

	uint16_t tile_idx = sPtr[0].attr2 & ATTR2_ID_MASK;
	uint16_t base_idx = tile_idx + (block_position.y * g_overlaybuffer.panels[panel_id].tile_width);

	for (uint16_t row = 0; row < size.y; ++row)
	{
		uint16_t idx = base_idx + (row * g_overlaybuffer.panels[panel_id].tile_width) + block_position.x;
		tile_ptr dest = &g_overlaybuffer.ewram_ovram[idx];

		for (uint16_t p = 0; p < size.x; ++p)
		{
			*dest++ = *tile;
		}
	}

	//	MARK AS DIRTY
	uint16_t tile_start = base_idx;
	uint16_t tile_count = size.y * g_overlaybuffer.panels[panel_id].tile_width;
	overlay_ovram_mark_as_dirty(tile_start, tile_count);

	//	assert on bounds exceeded 
}

void overlay_draw_color(uint8_t panel_id, uint8_t color, point2_t block_position, point2_t size)
{
	tile_t src;
	src.data[0] = color | (color << 4) | (color << 8) | (color << 12);
	src.data[0] |= (src.data[0] << 16);
	for (uint8_t idx = 1; idx < 8; ++idx)
		src.data[idx] = src.data[0];

	overlay_draw_tile(panel_id, &src, block_position, size);
}

//	support backcolor !!! 
void overlay_draw_string(uint8_t panel_id, const char* string, uint8_t color, point2_t tile_position)	//	consider line width ?? 
{
	obj_attr_t* sPtr = &g_overlaybuffer.ewram_oam[g_overlaybuffer.panels[panel_id].sprite_idx];
	//	get the sprites for this panel, 
	//	need to know what sprite occupies which position

	uint8_t* str_data = (uint8_t*)string;

	uint8_t c;
	int32_t x = tile_position.x, y = tile_position.y;

	uint16_t base_idx = sPtr[0].attr2 & ATTR2_ID_MASK;
	uint16_t tile_idx = base_idx + ((y * (g_overlaybuffer.panels[panel_id].tile_width)) + x);
	uint32_t* dst = (uint32_t * )&g_overlaybuffer.ewram_ovram[tile_idx];

	while ((c = *str_data++) != 0)
	{
		if (c == '\n')		// line break
		{
			y += 1;
			x = tile_position.x;	//	this isn't great, this shouldn't be reset to zero, it should be reset to something else 
			uint16_t tile_idx = base_idx + ((y * (g_overlaybuffer.panels[panel_id].tile_width)) + x);
			dst = (uint32_t*)&g_overlaybuffer.ewram_ovram[tile_idx];
		}
		else				// normal character
		{
			uint8_t row;
			uint16_t ix, iy;
			uint32_t pxs;

			// point to glyph; each line is one byte
			char* pch = (char*)& font8x8_basic[c][0];
			for (iy = 0; iy < 8; ++iy)
			{
				row = pch[iy];
				pxs = dst[iy];

				for (ix = 0; row > 0; row >>= 1, ++ix)		//	each character is stored in a 8x8 pixel block each pixel is 1 bit
				{
					if (row & 1)
						pxs |= (color << (ix * 4));

					/*else //backing color
						pxs = (pxs & 0xFF00);*/

				}

				dst[iy] = pxs;
			}

			dst += 8;

			//	on tile wrap, step back
		}
	}

	//	MARK AS DIRTY
	uint16_t tile_start = base_idx;
	uint16_t tile_count = (y+1) * g_overlaybuffer.panels[panel_id].tile_width;
	overlay_ovram_mark_as_dirty(tile_start, tile_count);
}

point2_t overlay_draw_character(uint8_t panel_id, char character, uint8_t color, point2_t tile_position)	//	consider line width ?? 
{
	panel_t* panel = &g_overlaybuffer.panels[panel_id];
	obj_attr_t* sPtr = &g_overlaybuffer.ewram_oam[panel->sprite_idx];

	//	get the sprites for this panel, 
	//	need to know what sprite occupies which position

	point2_t next_position = tile_position;

	int c, y = tile_position.y;

	uint16_t base_idx = sPtr[0].attr2 & ATTR2_ID_MASK;
	uint16_t tile_idx = base_idx + ((tile_position.y * (g_overlaybuffer.panels[panel_id].tile_width)) + tile_position.x);
	uint32_t * dst = (uint32_t*)&g_overlaybuffer.ewram_ovram[tile_idx];

	if ((c = character) != 0)
	{
		if (c == '\n')		// line break
		{
			y += 1;
			uint16_t tile_idx = base_idx + ((y * (g_overlaybuffer.panels[panel_id].tile_width)) + tile_position.x);
			dst = (uint32_t*)&g_overlaybuffer.ewram_ovram[tile_idx];
		}
		else				// normal character
		{
			uint8_t row;
			uint16_t ix, iy;
			uint32_t pxs;

			// point to glyph; each line is one byte
			char* pch = (char*)&font8x8_basic[c][0];
			for (iy = 0; iy < 8; ++iy)
			{
				row = pch[iy];
				pxs = dst[iy];

				for (ix = 0; row > 0; row >>= 1, ++ix)		//	each character is stored in a 8x8 pixel block each pixel is 1 bit
				{
					if (row & 1)
						pxs |= (color << (ix * 4));

					/*else //backing color
						pxs = (pxs & 0xFF00);*/

				}

				dst[iy] = pxs;
			}

			dst += 8;

			//	on tile wrap, step back
		}
	}

	//	MARK AS DIRTY
	uint16_t tile_start = tile_idx;
	uint16_t tile_count = 1;
	overlay_ovram_mark_as_dirty(tile_start, tile_count);

	next_position.x += 1;
	next_position.y = y;

	return next_position;
}

void overlay_clear(uint8_t panel_id, uint8_t color)
{
	uint16_t sprite_idx = g_overlaybuffer.panels[panel_id].sprite_idx;
	uint16_t sprite_count = g_overlaybuffer.panels[panel_id].sprite_count;
	uint16_t last_idx = sprite_idx + sprite_count;

	tile_t src;
	src.data[0] = color | (color << 4) | (color << 8) | (color << 12);
	src.data[0] |= (src.data[0] << 16);
	for (uint8_t idx = 1; idx < 8; ++idx)
		src.data[idx] = src.data[0];

	uint16_t idx = sprite_idx;
	while (idx < last_idx)
	{
		obj_attr_t* sprite = &g_overlaybuffer.ewram_oam[idx];
		//	get the sprites for this panel, 
		//	need to know what sprite occupies which position

		uint16_t tile_idx = sprite[0].attr2 & ATTR2_ID_MASK; 

		tile_ptr dest = &g_overlaybuffer.ewram_ovram[tile_idx];
		for (uint16_t i = 0; i < 4; ++i)
			*dest++ = src;

		idx++;
	}


	//	MARK AS DIRTY
	uint16_t tile_start = g_overlaybuffer.ewram_oam[sprite_idx].attr2 & ATTR2_ID_MASK;
	uint16_t tile_count = g_overlaybuffer.panels[panel_id].sprite_count * TILES_PER_SPRITE;
	overlay_ovram_mark_as_dirty(tile_start, tile_count);
}

void overlay_write_palette(palette_ptr palette)
{
	memory_copy16((g_overlaybuffer.ewram_pal + palette->offset), palette->color555, palette->size);	

	overlay_palette_mark_as_dirty(palette->offset, palette->size);
}

void overlay_present(void)
{
	if (g_overlaybuffer.dirty_mask & OVERLAY_DIRTY_PALETTE)
	{
		uint16_t count = g_overlaybuffer.palette_dirty_span.end - g_overlaybuffer.palette_dirty_span.start;

		memory_copy16(
			g_overlaybuffer.hw_pal + g_overlaybuffer.palette_dirty_span.start,
			g_overlaybuffer.ewram_pal + g_overlaybuffer.palette_dirty_span.start,
			count);

		g_overlaybuffer.dirty_mask &= ~OVERLAY_DIRTY_PALETTE;
		g_overlaybuffer.palette_dirty_span.start = -1;
		g_overlaybuffer.palette_dirty_span.end = -1;
	}

	if (g_overlaybuffer.dirty_mask & OVERLAY_DIRTY_OVRAM)
	{
		debug_assert(g_overlaybuffer.ovram_dirty_span.start >= 0, "overlay::present");
		debug_assert(g_overlaybuffer.ovram_dirty_span.end >= 0, "overlay::present");
		debug_assert(g_overlaybuffer.ovram_dirty_span.end > g_overlaybuffer.ovram_dirty_span.start, "overlay::present");

		int16_t count = (g_overlaybuffer.ovram_dirty_span.end - g_overlaybuffer.ovram_dirty_span.start);
		int16_t size_in_bytes = count * sizeof(tile_t);

		debug_assert(((sizeof(tile_t) * g_overlaybuffer.ovram_dirty_span.start) + size_in_bytes) <= OVERLAY_OVRAM_SIZE_IN_BYTES, "overlay::present");

		memory_copy32(
			g_overlaybuffer.hw_ovram + g_overlaybuffer.ovram_dirty_span.start,
			g_overlaybuffer.ewram_ovram + g_overlaybuffer.ovram_dirty_span.start,
			size_in_bytes >> 2);

		g_overlaybuffer.dirty_mask &= ~OVERLAY_DIRTY_OVRAM;
		g_overlaybuffer.ovram_dirty_span.start = -1;
		g_overlaybuffer.ovram_dirty_span.end = -1;
	}

	if (g_overlaybuffer.dirty_mask & OVERLAY_DIRTY_OAM)
	{
		debug_assert(g_overlaybuffer.oam_dirty_span.start >= 0, "overlay::present");
		debug_assert(g_overlaybuffer.oam_dirty_span.end >= 0, "overlay::present");
		debug_assert(g_overlaybuffer.oam_dirty_span.end > g_overlaybuffer.oam_dirty_span.start, "overlay::present");

		uint16_t count = g_overlaybuffer.oam_dirty_span.end - g_overlaybuffer.oam_dirty_span.start;
		int16_t size_in_bytes = count * sizeof(obj_attr_t);

		debug_assert(((sizeof(obj_attr_t) * g_overlaybuffer.oam_dirty_span.start) + size_in_bytes) <= (sizeof(obj_attr_t) * SPRITE_COUNT), "overlay::present");

		memory_copy32(
			g_overlaybuffer.hw_oam + g_overlaybuffer.oam_dirty_span.start,
			g_overlaybuffer.ewram_oam + g_overlaybuffer.oam_dirty_span.start,
			size_in_bytes >> 2);

		g_overlaybuffer.dirty_mask &= ~OVERLAY_DIRTY_OAM;
		g_overlaybuffer.oam_dirty_span.start = -1;
		g_overlaybuffer.oam_dirty_span.end = -1;
	}
}
