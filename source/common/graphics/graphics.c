
#include "graphics.h"

#include "common/math/fixed16.h"
#include "common/utils/profiler.h"
#include "common/debug/debug.h"
#include "common/debug/debug_helper.h"
#include "common/memory.h"
#include "common/containers/sort.h"
#include "common/codegen/codegen_armv4.h"
#include "common/compression/lz77.h"

#include "image.h"

//-----------------------------------------------------------------------------
graphics_context_t g_graphics_context;			//IWRAM_DATA 

//	this could all be part of the graphics context 
 image_ptr boundimage = NULL;				//	think about bandwidth for texture sampling  // make this an array
int invertedbitwiseWidth = 11;
int invertedbitwiseHeight = 11;
int bitwiseWidth = 5;
int bitwiseHeight = 5;	//eg . 4 => 16
int bitMaskWidth = 0x1F;
int bitMaskHeight = 0x1F;

depthmap_t* bound_depthmap = NULL;
uint8_t bound_depthlayer = 0;

uint8_t* texture_data_ptr = NULL;
//static uint8_t* texture_data_cache = NULL;		//	4K if possible ... 

const uint16_t graphics_screen_width = 240;
const uint16_t graphics_screen_height = 160;



//-----------------------------------------------------------------------------
void graphics_bind_rendertarget(rendertarget_ptr render_target)
{
	if (render_target)
	{
		g_graphics_context.bound_rendertarget = render_target;

		g_graphics_context.frame_buffer = render_target->frame_buffer;

		//	width and height ??? 
	}
	else
	{
		g_graphics_context.bound_rendertarget = NULL;

		g_graphics_context.frame_buffer = g_graphics_context.frame_pages[g_graphics_context.page_flip ^ 1];
	}
}

//-----------------------------------------------------------------------------
void graphics_bind_image(image_t* image)
{
	boundimage = image;
	
	bitwiseWidth = 0;
	bitwiseHeight = 0;
	bitMaskWidth = 0;
	bitMaskHeight = 0;

	int width = boundimage->width;
	while (width > 0)
	{
		width >>= 1;
		bitMaskWidth = bitMaskWidth | (1 << bitwiseWidth);		
		bitwiseWidth++;
	}
	bitMaskWidth >>= 1;

	int height = boundimage->height;
	while (height > 0)
	{
		height >>= 1;
		bitMaskHeight |= 1 << bitwiseHeight;
		bitwiseHeight++;
	}
	bitMaskHeight >>= 1;

	bitwiseWidth -= 1;
	bitwiseHeight -= 1;
	invertedbitwiseWidth = 16 - bitwiseWidth;
	invertedbitwiseHeight = 16 - bitwiseHeight;

	texture_data_ptr = &boundimage->data[0];

	//	consider copying to IWRAM, need to reserve texture memory to do this, 
	//		don't want to have to do loads of copies !!! 
}

//-----------------------------------------------------------------------------
void graphics_bind_depthmap(depthmap_t* depthmap)
{
	bound_depthmap = depthmap;
}

//-----------------------------------------------------------------------------
void graphics_set_depth_layer(uint8_t layer)
{
	bound_depthlayer = layer;
}

//-----------------------------------------------------------------------------
IWRAM_CODE void graphics_draw_image(image_t* image, uint16_t frame_idx, uint16_t x, uint16_t y)
{
	/**
		either flesh this out better, or maybe a graphics_draw_fullscreen_image would help with being more bespoke ... 
	*/

	if (image->format == 1)
	{
		debug_assert(frame_idx == 0, "graphics:draw_image - compressed images don't currently support multi-frame");
		debug_assert(g_graphics_context.width == image->width, "graphics:draw_image - image width doesn't match dest width");
		debug_assert(g_graphics_context.height == image->height, "graphics:draw_image - image height doesn't match dest height");

		lz77_decompress(image->data, g_graphics_context.frame_buffer);		//	can't be multi-frame and compressed 
	}
	else
	{
		int32_t	u, v;
		uint8_t* dest = g_graphics_context.frame_buffer + y * g_graphics_context.width + x;
		uint8_t* src = image->data + (frame_idx * image->width * image->height);

		if (g_graphics_context.width == image->width &&
			g_graphics_context.height == image->height)
		{
			memory_copy32(g_graphics_context.frame_buffer, image->data, image->size >> 2);
		}
		else
		{
			//	this can be faster 
			for (v = 0; v < image->height; v++, dest += g_graphics_context.width)
				for (u = 0; u < image->width; u++)
					dest[u] = src[(v * image->width) + u];
		}
	}
}

//-----------------------------------------------------------------------------
/*IWRAM_CODE*/ void graphics_draw_color(uint8_t color, point2_t pos, point2_t size)
{
	uint8_t			*dest;
	int				u, v;

	if (pos.x + size.x > g_graphics_context.width)
		size.x = g_graphics_context.width - pos.x;

	if (pos.y + size.y > g_graphics_context.height)
		size.y = g_graphics_context.height - pos.y;

	if (pos.x < 0)
	{
		size.x += pos.x;
		pos.x = 0;
	}

	if (pos.y < 0)
	{
		size.y += pos.y;
		pos.y = 0;
	}

	if (size.x < 0 || size.y < 0)
		return;

	dest = g_graphics_context.frame_buffer + pos.y * g_graphics_context.width + pos.x;
	for (v=0 ; v< size.y; v++, dest += g_graphics_context.width)
		for (u=0 ; u< size.x; u++)
			dest[u] = color;
}

//-----------------------------------------------------------------------------
void graphics_draw_highlightfield(highlight_field_ptr highlight, uint8_t color, uint16_t start_row, uint16_t end_row)
{
	highlight_span_t* last_span = &highlight->spans[highlight->span_count - 1];
	highlight_span_t* span = &highlight->spans[0];
	while (span < last_span && span->row < start_row)
		span++;

	while (span < last_span && span->row < end_row)
	{
		uint8_t idx = span->start;
		uint8_t end = span->end;

		debug_assert(span->row < 160, "graphics::draw_highlightfield row out of bounds");

		uint8_t* dest = g_graphics_context.frame_buffer + (span->row * g_graphics_context.width) + idx;
		while (idx++ < end)
		{
			*dest++ = color;
		}

		span++;
	}

	span_encapsulate(&g_graphics_context.frame_dirty_scanline_span, start_row, end_row);
}

#define CLIP_TO_SCREEN

//	alternative version of this for, vertex color, textured, textured_alpha, palette_textured, palette_textured_alpha, fixed_size_textures 
//		really need to clean out all the clipping from here to make this suggestion easier ... 

//-----------------------------------------------------------------------------
edge_t _graphics_left_edge, _graphics_right_edge;

//-----------------------------------------------------------------------------
IWRAM_CODE void create_edge_and_calculate_deltas(edge_t* edge, vertex_t* v1, vertex_t* v2)
{
	vertex_t* tv1 = v1;
	vertex_t* tv2 = v2;

	if (tv1->position.y > tv2->position.y)		//	surely we can make this deterministic from the caller func 
	{
		vertex_t* tmp = tv1;
		tv1 = tv2;
		tv2 = tmp;
	}

	edge->y = fixed16_ceil(tv1->position.y);
	edge->height = fixed16_ceil(tv2->position.y) - edge->y;

	debug_assert(fixed16_to_int(edge->height) < 1024, "graphics::createEdgeAndCalculateDeltas - edge->height >= 1024");
	fixed16_t overHeight = reciprocal_lut[fixed16_to_int(edge->height)];	//	can we improve precision here 

	edge->dx = fixed16_mul_approx2(tv2->position.x - tv1->position.x, overHeight);
	edge->du = fixed16_mul_approx2(tv2->uv.x - tv1->uv.x, overHeight);
	edge->dv = fixed16_mul_approx2(tv2->uv.y - tv1->uv.y, overHeight);

	fixed16_t subPix = edge->y - tv1->position.y;
	edge->x = tv1->position.x + fixed16_mul_approx2(edge->dx, subPix);
	edge->u = tv1->uv.x;
	edge->v = tv1->uv.y;
}

//-----------------------------------------------------------------------------
typedef void (*draw_span_func_ptr)(void);

draw_span_func_ptr draw_span_ptr;


//-----------------------------------------------------------------------------
void draw_spans(void);

//-----------------------------------------------------------------------------
void draw_spans_depth(void);


//-----------------------------------------------------------------------------
IWRAM_CODE uint8_t clip_polygon(vertex_t* src, vertex_t* dest, uint8_t vertex_count);


//-----------------------------------------------------------------------------
IWRAM_CODE void draw_polygon(vertex_t* vertices, uint8_t vertex_count)
{
	profiler_sample_handle_t phandle;

	debug_assert(vertex_count == 3 || vertex_count == 4, "vertex_count isn't supported");


	phandle = profiler_begin("g:clip");

	vertex_t new_vertices[6];
	vertex_count = clip_polygon(vertices, new_vertices, vertex_count);	//	697 cycles per polygon .. 23,000 -> 40,000 cycles per model

	profiler_end(phandle);

	if (vertex_count == 0)
		return;

	vertex_t* v = &new_vertices[0];



	//profiler_end(phandle);

	int8_t left_current = 0, right_current = 0;
	int8_t left_next, right_next;

	for (int32_t i = 0; i < vertex_count; ++i)
	{
		if (v[i].position.y < v[left_current].position.y)
			left_current = i;
	}

	right_current = left_current;

	left_next = left_current - 1;
	if (left_next < 0)
		left_next = vertex_count - 1;

	right_next = right_current + 1;
	if (right_next > vertex_count - 1)
		right_next = 0;


	int32_t top_y = (v[left_current].position.y >> 16);
	int32_t bottom_y = 0;

	//	if we pull create_edge func inline then we can just calculate the heights first to determine if left edge is long 

	//	if gt backbuffer.height or lt 0

	//phandle = profiler_begin("g:tri_edge");		//	956 cycles

	//	this should turn into a loop until all verts are processed 



	//	convert below to a loop 

	create_edge_and_calculate_deltas(&_graphics_left_edge, &v[left_current], &v[left_next]);		//	left_next
	create_edge_and_calculate_deltas(&_graphics_right_edge, &v[right_current], &v[right_next]);	//	right_next

	int8_t left_edge_is_long = _graphics_left_edge.height > _graphics_right_edge.height;
	if (left_edge_is_long && _graphics_left_edge.height == 0)
		return;

	if (!left_edge_is_long && _graphics_right_edge.height == 0)
		return;

	//if (_graphics_right_edge.height > 0)
	draw_span_ptr();

	int32_t vertex_idx = vertex_count - 2;

	while (vertex_idx)
	{
		int8_t left_edge_is_long = _graphics_left_edge.height > _graphics_right_edge.height;
		if (left_edge_is_long)
		{
			right_current = right_next;
			right_next = right_current + 1;
			if (right_next > vertex_count - 1)
				right_next = 0;

			create_edge_and_calculate_deltas(&_graphics_right_edge, &v[right_current], &v[right_next]);

			if (_graphics_right_edge.height > 0)
				draw_span_ptr();
		}
		else
		{
			left_current = left_next;
			left_next = left_current - 1;
			if (left_next < 0)
				left_next = vertex_count - 1;

			create_edge_and_calculate_deltas(&_graphics_left_edge, &v[left_current], &v[left_next]);

			if (_graphics_left_edge.height > 0)
				draw_span_ptr();
		}

		vertex_idx--;
	}


	if (left_edge_is_long)
	{
		bottom_y = ((_graphics_left_edge.y + _graphics_left_edge.height) >> 16);
	}
	else
	{
		bottom_y = ((_graphics_right_edge.y + _graphics_right_edge.height) >> 16);
	}

	span_encapsulate(&g_graphics_context.frame_dirty_scanline_span, top_y, bottom_y);
}

//-----------------------------------------------------------------------------
void graphics_draw_line(vector3_t* points, uint16_t point_count, uint8_t color, matrix4x4_t* m)
{
	uint16_t i;
	vector4_t transformedPosition;
	fixed16_t w, half_w;
	vector3_t p2, p1;

	matrix4x4_t* matrix_working_copy = memory_allocate(sizeof(matrix4x4_t), MEMORY_IWRAM);	

	memory_copy(matrix_working_copy, m, sizeof(matrix4x4_t));

	mathMatrix4x4MultiplyPoint3(&transformedPosition, matrix_working_copy, &points[0]);

	w = fixed16_div(fixed16_one, transformedPosition.w);		//	big reciprocal lookup table  16.16 ---> 5.6 --- (X >> 10) & 0x07FF
	half_w = w >> 1;

	p2.x = g_graphics_context.widthMinusOne - fixed16_mul(fixed16_mul(transformedPosition.x, half_w) + fixed16_half, g_graphics_context.widthMinusOne);	//	can probably do some shift magic instead of mul		///fixed16_mul(fixed16_mul_approx2(p1.x, half_w) + fixed16_half, g_graphics_context.widthMinusOne)
	p2.y = fixed16_mul(fixed16_mul(transformedPosition.y, half_w) + fixed16_half, g_graphics_context.heightMinusOne);
	p2.z = fixed16_mul(transformedPosition.z, w);	// fixed16_mul_approx2(p1.z, w);

	mathVector3Copy(&p1, &p2);

	for (i = 1; i < point_count; ++i)
	{
		mathMatrix4x4MultiplyPoint3(&transformedPosition, matrix_working_copy, &points[i]);

		w = fixed16_div(fixed16_one, transformedPosition.w);		//	big reciprocal lookup table  16.16 ---> 5.6 --- (X >> 10) & 0x07FF
		half_w = w >> 1;

		p2.x = g_graphics_context.widthMinusOne - fixed16_mul(fixed16_mul(transformedPosition.x, half_w) + fixed16_half, g_graphics_context.widthMinusOne);	//	can probably do some shift magic instead of mul		///fixed16_mul(fixed16_mul_approx2(p1.x, half_w) + fixed16_half, g_graphics_context.widthMinusOne)
		p2.y = fixed16_mul(fixed16_mul(transformedPosition.y, half_w) + fixed16_half, g_graphics_context.heightMinusOne);
		p2.z = fixed16_mul(transformedPosition.z, w);	// fixed16_mul_approx2(p1.z, w);

		//	draw line in screen space !!! 
		//		clamp 

		fixed16_t x_diff = p2.x - p1.x;
		fixed16_t y_diff = p2.y - p1.y;

		if (fixed16_abs(x_diff) > fixed16_abs(y_diff))
		{
			int16_t x_min, x_max;

			if (p1.x < p2.x)
			{
				x_min = fixed16_to_int(p1.x);
				x_max = fixed16_to_int(p2.x);
			}
			else 
			{
				x_min = fixed16_to_int(p2.x);
				x_max = fixed16_to_int(p1.x);
			}
			
			fixed16_t slope = fixed16_div(y_diff, x_diff);
			for (int16_t x = x_min; x <= x_max; x += 1)
			{
				int16_t y = fixed16_to_int(p1.y + fixed16_mul(F16(x) - p1.x, slope));
				if (y < 0 || y >= g_graphics_context.height || x < 0 || x > g_graphics_context.width)
					continue;
				g_graphics_context.frame_buffer[(y * g_graphics_context.width) + x] = color;
			}
		}
		else
		{
			int16_t y_min, y_max;

			if (p1.y < p2.y)
			{
				y_min = fixed16_to_int(p1.y);
				y_max = fixed16_to_int(p2.y);
			}
			else
			{
				y_min = fixed16_to_int(p2.y);
				y_max = fixed16_to_int(p1.y);
			}

			fixed16_t slope = fixed16_div(x_diff, y_diff);
			for (int16_t y = y_min; y <= y_max; y += 1)
			{
				int16_t x = fixed16_to_int(p1.x + fixed16_mul(F16(y) - p1.y, slope));
				if (y < 0 || y >= g_graphics_context.height || x < 0 || x > g_graphics_context.width)
					continue;
				g_graphics_context.frame_buffer[(y * g_graphics_context.width) + x] = color;
			}
		}

		mathVector3Copy(&p1, &p2);
	}

	memory_free(matrix_working_copy);
}

uint32_t* program_destination = 0;

typedef struct draw_span_parameter_s
{
	uint32_t* uv_mask_assignment;
	uint32_t* texture_sample[4];

} draw_span_parameter_t;

//-----------------------------------------------------------------------------
void prepare_draw_instructions()
{
#ifdef __GBA__

	//	cache key for what is currently loaded 
	//		program_id + params_checksum 

	if (program_destination == 0)
	{
		extern draw_span_parameter_t draw_spans__params;

		uint8_t* draw_spans__start = (uint8_t*)draw_spans;
		uint8_t* draw_spans__end = (uint8_t*)(&draw_spans__params);

		uint32_t program_size = (uint32_t)draw_spans__end - (uint32_t)draw_spans__start;
		program_size += 10 * sizeof(uint32_t);

		//	this relies on the assembly handling global ldr with a pc offset to the end of the function for absolute address
		//		is this something we should / could rely on 

		program_destination = memory_allocate(program_size, MEMORY_IWRAM);

		memory_copy32(program_destination, draw_spans__start, program_size >> 2);

		//	retarget these pointers to the new location 
		//		only support relocation from cart, destination address has to be less than source address 

		uint32_t program_relocation_offset = (uint32_t)draw_spans__start - (uint32_t)program_destination;
		program_relocation_offset >>= 2;

		uint32_t* instruction_ptr = (draw_spans__params.uv_mask_assignment - program_relocation_offset);

		instruction_ptr[0] = codegen_armv4_mov_instruction(armv4_register_r5, bitMaskWidth, 0);
		instruction_ptr[1] = codegen_armv4_mov_instruction(armv4_register_r7, bitMaskHeight, 32 - bitwiseWidth);

		for (uint32_t i = 0; i < 4; ++i)
		{
			instruction_ptr = (draw_spans__params.texture_sample[i] - program_relocation_offset);

			instruction_ptr[0] = codegen_armv4_and_instruction(armv4_register_r2, armv4_register_r5, armv4_register_r4, 0, (invertedbitwiseWidth & 0x1F));
			instruction_ptr[1] = codegen_armv4_and_instruction(armv4_register_r8, armv4_register_r7, armv4_register_r6, 0, ((invertedbitwiseHeight - bitwiseWidth) & 0x1F));
		}
	}

	draw_span_ptr = (draw_span_func_ptr)program_destination;

#else

	draw_span_ptr = (draw_span_func_ptr)draw_spans_depth;// draw_spans;		//	draw_spans_depth

#endif
}

//-----------------------------------------------------------------------------
IWRAM_CODE void graphics_draw_model(model_t* model, uint16_t animation_id, uint16_t frame, matrix4x4_t* m)
{
	//	Support pre-transformed meshes - could be useful for animations

	//	Meshes should be limited to 256 vertices.

	profiler_sample_handle_t phandle;

	//	Copy Matrix to working data ??? or at least to stack

	int32_t i, count, offset;			//	These should all be IWRAM data, could collate into rendering working data structure
	
	fixed16_t half_w;
	packed_vector3_t packedPosition;
	vertex_t polygon_vertices[4];

	//	load texture into IWRAM	-> 64x64 = 4KB
	//	load submesh into IWRAM	-> 256*20 = 5KB

	//	limit to 128 polygons ??? 
	// OPTIMIZATION 1: Reduce polygons in a cluster and move to IWRAM
	vertex_t* processed_vertices = memory_allocate(sizeof(vertex_t) * 512, MEMORY_EWRAM);				//	more efficient allocator would be useful here 
	matrix4x4_t* matrix_working_copy = memory_allocate(sizeof(matrix4x4_t), MEMORY_IWRAM);// );

	//debug_printf(DEBUG_LOG_DEBUG, "graphics::draw_model - processed_vertices=0x%x", processed_vertices);

	memory_copy(matrix_working_copy, m, sizeof(matrix4x4_t));
	fixed16_t* p = (fixed16_t*)matrix_working_copy;
	for (i = 0; i < 16; ++i)
		p[i] >>= 6;

	//	transformed geometry cache x, y, z 12 bytes per vertex, 256 vertices , ~3KB geometry cache ...  maybe split by face normals (view angle) 
	//		try to sort back to front within sub mesh 

	//	sign_bit.5.10		// -32m -> +32m
	//		- must exist within a -8m to 8m range (small world, could be fine for us) ??? 

	count = model->vertices_count;
	offset = frame * count;		//	need to cluster the meshes in blocks of x verteices !!! 

	vector3_t unpack_scale = model->scale;
	vector3_t unpack_origin = model->origin;
	animation_t* animation = model_find_animation(model, animation_id);
	packed_vector3_t* packed_positions = &animation->frames[offset];	//	need to inform what animation to play 
	vertex_t* output_vertex = &processed_vertices[0];

	unpack_origin.x <<= 8;
	unpack_origin.y <<= 8;
	unpack_origin.z <<= 8;

	phandle = profiler_begin("g:unpak_vert");			//	17,280 for 380 vertices

	//	unpack vertices 
	for (i = 0; i < count; ++i)								//	do this once, unpack into EWRAM ... 
	{
		packedPosition = *packed_positions++;

		output_vertex->position.x = ((unpack_scale.x * packedPosition.x) + unpack_origin.x) >> 14;	//	// (8 + 6)	MLA result, scale, position, origin, lsl #8
		output_vertex->position.y = ((unpack_scale.y * packedPosition.y) + unpack_origin.y) >> 14;
		output_vertex->position.z = ((unpack_scale.z * packedPosition.z) + unpack_origin.z) >> 14;

		//	16 => 6.10		64 Meters Max Size -> Millimeter precision		// this is great for world space 
		//		What about sign bit ??? 

		output_vertex++;
	}

	profiler_end(phandle);

	phandle = profiler_begin("g:prep_vert_1");			//	26,843 for 380 vertices

#define FIXED_16_MUL(x, y) ((x * y) >> 10)

	//	transform vertices to view space. 
	//	move to it's own func (vertex_transform)

	output_vertex = &processed_vertices[0];
	for (i = 0; i < count; ++i)						
	{
		//	this is now fixed 6.10 

		//	right_edge-write in asm
		//		using MLA and ADC   --- 

		fixed16_t cached_position_x = output_vertex->position.x;
		fixed16_t cached_position_y = output_vertex->position.y;
		fixed16_t cached_position_z = output_vertex->position.z;

		output_vertex->position.x =
			FIXED_16_MUL(matrix_working_copy->col0.x, cached_position_x) +			//	fixed16_mul_approx	-- this doesn't work for some reason 
			FIXED_16_MUL(matrix_working_copy->col1.x, cached_position_y) +
			FIXED_16_MUL(matrix_working_copy->col2.x, cached_position_z) +
			matrix_working_copy->col3.x;

		output_vertex->position.y =
			FIXED_16_MUL(matrix_working_copy->col0.y, cached_position_x) +
			FIXED_16_MUL(matrix_working_copy->col1.y, cached_position_y) +
			FIXED_16_MUL(matrix_working_copy->col2.y, cached_position_z) +
			matrix_working_copy->col3.y;

		output_vertex->position.z =		
			FIXED_16_MUL(matrix_working_copy->col0.z, cached_position_x) +
			FIXED_16_MUL(matrix_working_copy->col1.z, cached_position_y) +
			FIXED_16_MUL(matrix_working_copy->col2.z, cached_position_z) +
			matrix_working_copy->col3.z;

		output_vertex->position.w =		
			FIXED_16_MUL(matrix_working_copy->col0.w, cached_position_x) +
			FIXED_16_MUL(matrix_working_copy->col1.w, cached_position_y) +
			FIXED_16_MUL(matrix_working_copy->col2.w, cached_position_z) +
			matrix_working_copy->col3.w;


		output_vertex++;
	}

	profiler_end(phandle);


	int32_t backbuffer_width_minus_one = g_graphics_context.widthMinusOne >> 8;
	int32_t backbuffer_height_minus_one = g_graphics_context.heightMinusOne >> 8;
	int32_t fixed_6_10_half = fixed16_half >> 6;
	
	phandle = profiler_begin("g:prep_vert_2");			//	25,268 for 380 vertices

	//	transform vertices into pixel space & unpack uvs 
	output_vertex = &processed_vertices[0];
	for (i = 0; i < count; ++i)						
	{
		//	some asserts would be useful to ensure things are within expectations 

		uint16_t idx = fixed16_abs(output_vertex->position.w >> 6);// & 0x03FF;		// 6.10  (>> 6)  6.4

		half_w = reciprocal_lut[idx] >> 3;	//	 << 4 (undo the 6.4), >> 6 (shift to 6.10), >> 1 (divide by 2) 
		if (output_vertex->position.w < 0)
			half_w = -half_w;

		//	should we just keep this in 0 -> 1 space ?? 

		fixed16_t x = (((output_vertex->position.x * half_w) >> 10) + fixed_6_10_half) >> 2;		// can I optimize this to one shift 
		output_vertex->position.x = (backbuffer_width_minus_one << 8) - (x * backbuffer_width_minus_one);	

		fixed16_t y = (((output_vertex->position.y * half_w) >> 10) + fixed_6_10_half)  >> 2;	//	6.10 multiplication, then final conversion to 8.8
		output_vertex->position.y = y * backbuffer_height_minus_one;		//	8.8 multiplication, don't shift right so will result in a 16.16

		//	need z to determine, triangle sorting 

		//		Packed UVs range from { -2 <-> +2 }  ===  sign needs to carry across !! 
		output_vertex->uv.x = model->vertices[i].uv.x << 10;	//	Shift 8 to bring into 0 <-> 1 // Shift another 2 to get to 0 <-> 4
		output_vertex->uv.y = model->vertices[i].uv.y << 10;

		//output_vertex->uv.x -= fixed16_one << 1;		//	this just isn't needed
		//output_vertex->uv.y -= fixed16_one << 1;

		output_vertex++;
	}

	profiler_end(phandle);


	phandle = profiler_begin("g:poly_cull");		//

	//	sort and cull polygons	 move to it's own func 

	polygon_t* polygons_to_draw = memory_allocate(sizeof(polygon_t) * 192, MEMORY_EWRAM);	//	linked list might make alot of sense here .
	polygon_t* polygon_ptr = &polygons_to_draw[0];

	uint16_t polygons_to_draw_idx = 0;

	sort_index_t* polygon_indices = memory_allocate(sizeof(sort_index_t) * 192, MEMORY_IWRAM);
	sort_index_t* index_ptr = &polygon_indices[0];

	vertex_t* v;

	count = model->polygon_count;		//	limit number of polygons for, being able to have a fixed sized buffer for triangle sorting 
	for (i = 0; i < count; ++i)							//	PVS will affect how many polygons we have to deal with here
	{
		int16_t depth_value = 0;

		{
			v = &processed_vertices[model->polygons[i].v[0]];
			mathVector3Copy((vector3_t*)&polygon_vertices[0].position, (const vector3_t*)&v->position);
			mathVector2Copy(&polygon_vertices[0].uv, &v->uv);

			//	just uses the depth of the first vertex to sort the polygons, not ideal, but should be good enough
			depth_value = v->position.z;

			v = &processed_vertices[model->polygons[i].v[1]];
			mathVector3Copy((vector3_t*)&polygon_vertices[1].position, (const vector3_t*)&v->position);
			mathVector2Copy(&polygon_vertices[1].uv, &v->uv);

			depth_value += v->position.z;

			v = &processed_vertices[model->polygons[i].v[2]];
			mathVector3Copy((vector3_t*)&polygon_vertices[2].position, (const vector3_t*)&v->position);
			mathVector2Copy(&polygon_vertices[2].uv, &v->uv);

			depth_value += v->position.z;

			if (model->polygons[i].vertex_count > 3)
			{
				v = &processed_vertices[model->polygons[i].v[3]];
				mathVector3Copy((vector3_t*)&polygon_vertices[3].position, (const vector3_t*)&v->position);
				mathVector2Copy(&polygon_vertices[3].uv, &v->uv);
			}
		}

		//	if this face culling correct, how do we test this !!?? 

		//	can we use a more approx mul here 

		fixed16_t d = fixed16_mul((polygon_vertices[1].position.x - polygon_vertices[0].position.x), (polygon_vertices[2].position.y - polygon_vertices[0].position.y));
		d -= fixed16_mul((polygon_vertices[1].position.y - polygon_vertices[0].position.y), (polygon_vertices[2].position.x - polygon_vertices[0].position.x));

		if (d > fixed16_zero)//(fixed16_one << 2))		//should be zero, but increasing this will only remove polygons that are at a sharp angle and probably don't take up many pixlelse
		{
			sort_index_t index = { depth_value, polygons_to_draw_idx };

			*index_ptr++ = index;

			polygon_ptr->vertex_count = model->polygons[i].vertex_count;

			polygon_ptr->v[0] = polygon_vertices[0];
			polygon_ptr->v[1] = polygon_vertices[1];
			polygon_ptr->v[2] = polygon_vertices[2];
			polygon_ptr->v[3] = polygon_vertices[3];

			polygon_ptr++;

			polygons_to_draw_idx++;
		}
	}

	profiler_end(phandle);

	phandle = profiler_begin("g:poly_sort");	//	

	//	sort polygons back to front 
	count = polygons_to_draw_idx;
	sort_insertion(polygon_indices, count);

	profiler_end(phandle);

	//	here we can use processed_vertices block as texture_cache 


	phandle = profiler_begin("g:poly_draw");	//	

	prepare_draw_instructions();		//	need to pass the type of draw required 

	//	draw loop 
	index_ptr = &polygon_indices[0];
	polygon_ptr = &polygons_to_draw[0];
	count = polygons_to_draw_idx;
	for (i = 0; i < count; ++i)		
	{
		//	track dirty region 
			//	will need to keep 2, depending on page 
			//	should probably just keep this in the graphics_context 

		polygon_ptr = &polygons_to_draw[(index_ptr++)->idx];
		
		draw_polygon(polygon_ptr->v, polygon_ptr->vertex_count);
	}

	profiler_end(phandle);

	memory_free(polygon_indices);
	memory_free(polygons_to_draw);		//	use this as scratch space... 
	memory_free(processed_vertices);		//	can we just keep these around ??? 
	memory_free(matrix_working_copy);
}

//-----------------------------------------------------------------------------
void graphics_write_palette(palette_ptr palette) 
{
	//	dirty range

	memory_copy16((g_graphics_context.palette + palette->offset), palette->color555, palette->size);		

	g_graphics_context.dirty_flags |= 0x01;

	/*
		palette fade ? 
			Should only happen within a range !!! 
				can we fade with just a 
	 */
}

//-----------------------------------------------------------------------------
void graphics_read_palette(palette_ptr palette)
{
	memory_copy16(palette->color555, (g_graphics_context.palette + palette->offset), palette->size);
}

//-----------------------------------------------------------------------------
void graphics_set_resolution(uint8_t width, uint8_t height)
{
	debug_assert(width <= 240, "graphics::set_resolution width > 240");
	debug_assert(height <= 160, "graphics::set_resolution height > 160");

	g_graphics_context.width = width;
	g_graphics_context.height = height;
	g_graphics_context.widthMinusOne = F16(g_graphics_context.width - 1);
	g_graphics_context.heightMinusOne = F16(g_graphics_context.height - 1);
	g_graphics_context.stridebytes = width * sizeof(uint8_t);

	g_graphics_context.dirty_flags |= 0x04;
}

//-----------------------------------------------------------------------------
void graphics_pageflip(void)
{
	g_graphics_context.dirty_flags |= 0x02;
}

//-----------------------------------------------------------------------------
void graphics_reset_dirty_scanlines(void)
{
	/*
		currently only works with draw_polygon 
	*/

	g_graphics_context.frame_dirty_scanline_span.start = g_graphics_context.height;
	g_graphics_context.frame_dirty_scanline_span.end = 0;
}

//-----------------------------------------------------------------------------
void graphics_clear(uint8_t color)
{
	memory_set(g_graphics_context.frame_buffer, g_graphics_context.width * g_graphics_context.height * sizeof(uint8_t), color);
}

//-----------------------------------------------------------------------------
void graphics_set_vsync(uint8_t mode)
{
	//	what are we aiming for 20fps, 30fps, 60fps
	g_graphics_context.vblank_target = mode;
}

//-----------------------------------------------------------------------------
uint8_t graphics_get_vsync()
{
	//	what are we aiming for 20fps, 30fps, 60fps
	return g_graphics_context.vblank_target;
}

//-----------------------------------------------------------------------------
rendertarget_ptr rendertarget_new(uint8_t width, uint8_t height, uint8_t memory_sector)
{
	rendertarget_ptr render_target = (rendertarget_ptr)memory_allocate(sizeof(rendertarget_t) + (width * height * sizeof(uint8_t)), memory_sector);

	render_target->width = width;
	render_target->height = height;
	render_target->stridebytes = width;

	return render_target;
}

//-----------------------------------------------------------------------------
void rendertarget_delete(rendertarget_ptr render_target)
{
	debug_assert(render_target != NULL, "");

	memory_free(render_target);
}
