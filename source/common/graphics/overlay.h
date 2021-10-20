
#ifndef OVERLAY_H
#define OVERLAY_H

#include "common/types.h"
#include "common/memory.h"
#include "common/graphics/image.h"
#include "common/utils/span.h"

#include "obj_attr.h"

//	concepts
//		tiles
//		sprite

#define PANEL_COUNT		8

typedef struct panel_s		//	improve aligment within this struct
{
	uint8_t attributes;
	point2_t position;
	//	size

	uint8_t sprite_width;
	uint8_t sprite_height;

	uint8_t tile_width;
	uint8_t tile_height;

	uint8_t sprite_idx;
	uint8_t sprite_count;

} panel_t;


#define OVERLAY_DIRTY_PALETTE	0x0001
#define OVERLAY_DIRTY_OVRAM		0x0002
#define OVERLAY_DIRTY_OAM		0x0008

#define OVERLAY_OVRAM_SIZE_IN_BYTES (sizeof(uint16_t) * (256 * 8 * 8))


//	loads of mark dirty functions

typedef struct overlay_buffer_s		
{
	//	Hardware Ptr
	color555_t* hw_pal;				// 0x05000200   (512 bytes)
	tile_t* hw_ovram;				// 16 Kilo-bytes
	obj_attr_t* hw_oam;

	//	EWRAM Ptr 
	color555_t* ewram_pal;
	tile_t* ewram_ovram;		//	change this to tiles
	obj_attr_t* ewram_oam;

	//	High levels tracking 
	panel_t* panels;
	uint16_t base_tile_idx;

	//	Dirty
	uint16_t dirty_mask;
	span_t palette_dirty_span;
	span_t ovram_dirty_span;
	span_t oam_dirty_span;

} overlay_buffer_t;

extern overlay_buffer_t g_overlaybuffer;


void overlay_initialize(void);	//	platform
void overlay_shutdown(void);		//	platform


int8_t overlay_create_panel(uint8_t tile_width, uint8_t tile_height);	
void overlay_destroy_panel(uint8_t panel_id);

//	panel atrributes 
void overlay_set_palette_bank(uint8_t panel_id, uint8_t palette_bank);
void overlay_set_position(uint8_t panel_id, point2_t position);
void overlay_set_priority(uint8_t panel_id, uint8_t priority);		//	emulate this on windows
void overlay_set_visiblity(uint8_t panel_id, uint8_t visiblity);	//	test this

void overlay_write_palette(palette_ptr palette);


//	draw tile	-- will help with frames, maybe a formal nine-slice image ? 

void overlay_draw_color(uint8_t panel_id, uint8_t color, point2_t block_position, point2_t size);
void overlay_draw_tile(uint8_t panel_id, tile_ptr tile, point2_t block_position, point2_t size);
void overlay_draw_tiledimage(uint8_t panel_id, tiledimage_ptr image, point2_t block_position);
void overlay_draw_tiledimage_nineslice(uint8_t panel_id, tiledimage_ptr image, point2_t block_position, point2_t block_size);

point2_t overlay_draw_character(uint8_t panel_id, char character, uint8_t color, point2_t tile_position);
void overlay_draw_string(uint8_t panel_id, const char* string, uint8_t color, point2_t position);

void overlay_clear(uint8_t panel_id, uint8_t color);	//	size ??? 

void overlay_present(void);

#endif // !OVERLAY_H
