
#ifndef GRAPHICS_H
#define GRAPHICS_H


#include "common/math/math.h"
#include "common/math/point.h"
#include "common/math/fixed16.h"
#include "common/math/matrix.h"

#include "common/graphics/model.h"
#include "common/graphics/image.h"
#include "common/graphics/highlight_field.h"

#include "common/utils/span.h"

#define RGB555(r,g,b)	( (((b)>>3)<<10) | (((g)>>3)<<5) | ((r)>>3) )	//	takes 888 and converts to 555

//-----------------------------------------------------------------------------
typedef struct rendertarget_s
{
	uint8_t width, height, stridebytes, reserved;
	uint8_t frame_buffer[0];

} rendertarget_t;

typedef rendertarget_t* rendertarget_ptr;

//-----------------------------------------------------------------------------
typedef struct graphics_context_s			//	this is more like graphics_context 
{	
	uint16_t* palette;				// 0x05000000   (512 bytes)
	uint8_t* frame_buffer;			// 0x06000000   (38400 bytes)
	uint8_t* frame_pages[2];			// 0x06000000,0x06009600		//	only needed internally

	rendertarget_ptr bound_rendertarget;
	// rendertarget_t		-- primary render target <<--- this is different as we have multiple pages for back_buffer 

	uint16_t width, height, stridebytes;		//	240,160		/    120,80		/ 120, 160
	uint16_t dirty_flags;		

	uint16_t page_flip;

	int8_t vblank_target;
	volatile int8_t vblank_count;

	volatile int32_t vblank_total_count;

	fixed16_t widthMinusOne, heightMinusOne;

	span_t frame_dirty_scanline_span;

} graphics_context_t;

//-----------------------------------------------------------------------------
//	can this be internal 
typedef struct vertex_s		// 8.8 fixed should be fine for storage
{
	vector4_t position;				//	4 words -> 16 bytes
	vector2_t uv;					//  2 words -> 8 bytes
	//	components ??? could be color or lighting	//	fixed at 4 components  - eg, color + lighting | texture + lighting
} vertex_t;

typedef struct polygon_s
{
	vertex_t v[4];
	uint32_t vertex_count;

} polygon_t;

//-----------------------------------------------------------------------------
typedef struct edge_s		//	 32 bytes
{
	fixed16_t x, dx;
	fixed16_t u, du;
	fixed16_t v, dv;
	fixed16_t y, height;
} edge_t;


extern graphics_context_t g_graphics_context;

const extern uint16_t graphics_screen_width;
const extern uint16_t graphics_screen_height;

//	support for rendertarget, so we can render out the screen for the savefiles 

//
//	Platform Specific Implementation
//

void graphics_initialize();		

void graphics_set_resolution(uint8_t width, uint8_t height);	

void graphics_set_vsync(uint8_t mode);

void graphics_bind_rendertarget(rendertarget_ptr render_target);




void graphics_pageflip();

void graphics_vsync();

void graphics_present();

void graphicsShutdown();


//
//	Bind for DrawCommands
//

void graphics_bind_image(image_t* image);

void graphics_bind_depthmap(depthmap_t* depthmap);

void graphics_set_depth_layer(uint8_t layer);

//
//	Pal
//

void graphics_write_palette(palette_ptr palette);

void graphics_read_palette(palette_ptr palette);

//
//	Draw, these we draw to the active render_target 
//

void graphics_draw_line(vector3_t* points, uint16_t point_count, uint8_t color, matrix4x4_t* m);

void graphics_draw_model(model_t* model, uint16_t animation_id, uint16_t frame, matrix4x4_t* m);	//	need to know the animation we wish to play,, pass animation id ?? 

void graphics_draw_image(image_t* image, uint16_t frame_idx, uint16_t x, uint16_t y);

void graphics_draw_highlightfield(highlight_field_ptr highlight, uint8_t color, uint16_t start_row, uint16_t end_row);

void graphics_draw_color(uint8_t color, point2_t pos, point2_t size);  //***


void graphics_clear(uint8_t color);

///
///		Dirty Region Tracking
///

void graphics_reset_dirty_scanline(void);

///

rendertarget_ptr rendertarget_new(uint8_t width, uint8_t height, uint8_t memory_sector);

void rendertarget_delete(rendertarget_ptr render_target);


#endif // !GRAPHICS_H