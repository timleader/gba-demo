
#include "graphics.h"

//-----------------------------------------------------------------------------
extern int invertedbitwiseWidth;
extern int invertedbitwiseHeight;
extern int bitwiseWidth;
extern int bitwiseHeight;	//eg . 4 => 16
extern int bitMaskWidth;
extern int bitMaskHeight;

extern uint8_t* texture_data_ptr;

extern depthmap_t* bound_depthmap; 
extern uint8_t bound_depthlayer;

//-----------------------------------------------------------------------------
extern edge_t _graphics_left_edge, _graphics_right_edge;


//-----------------------------------------------------------------------------
int32_t sample_texture(fixed16_t* u, fixed16_t* v, fixed16_t du, fixed16_t dv)
{
	int32_t texel_x = (*u >> invertedbitwiseWidth) & bitMaskWidth;		//	move these to IWRAM DATA or ensure they are REGs
	int32_t texel_y = (*v >> invertedbitwiseHeight) & bitMaskHeight;		//	16 - X can be done once ahead of time

	//int32_t texel_x = (*u >> 10) & 0x003F;		//	move these to IWRAM DATA or ensure they are REGs
	//int32_t texel_y = (*v >> 4) & 0x0FC0;		//	16 - X can be done once ahead of time

	int32_t sample_idx = (texel_y << bitwiseWidth) + texel_x;

	*u += du;
	*v += dv;

	return texture_data_ptr[sample_idx];
}

//-----------------------------------------------------------------------------
void draw_spans(void)			
{
	fixed16_t fheight = fixed16_min(_graphics_left_edge.height, _graphics_right_edge.height);	//	one of the edges might continue to be used for the next draw_spans, so heights might be different 

	uint16_t* output_scanline_ptr = g_graphics_context.frame_buffer + (fixed16_to_int(_graphics_left_edge.y) * g_graphics_context.width);		//	output_scanline_ptr is 16-bit wide as VRAM requires writes of 16-bit wide

	_graphics_left_edge.height -= fheight;
	_graphics_right_edge.height -= fheight;

	_graphics_left_edge.y += fheight;
	_graphics_right_edge.y += fheight;

	int32_t iheight = fixed16_to_int(fheight);

	while (iheight-- > 0)
	{
		int32_t left_edge_floor_x_int = fixed16_to_int(_graphics_left_edge.x);	//	could we be happy to floor it would be faster 
		int32_t right_edge_floor_x_int = fixed16_to_int(_graphics_right_edge.x);

		//	Calculate Vertex Attribute Gradients
		int32_t width = right_edge_floor_x_int - left_edge_floor_x_int;

		//debug_printf(DEBUG_LOG_DEBUG, "width = %i", width);

		if (width <= 0 &&						//	is this worth having 
			_graphics_left_edge.dx > 0 &&
			_graphics_right_edge.dx < 0)
		{
			return;
		}

		if (width > 0)		//	why are we getting a load of zero width ... 
		{
			fixed16_t overWidth = reciprocal_lut[width];	//	IWRAM lookup table ??? 

			fixed16_t u = _graphics_left_edge.u;
			fixed16_t v = _graphics_left_edge.v;
			fixed16_t du = fixed16_mul_approx2(_graphics_right_edge.u - _graphics_left_edge.u, overWidth);	//fixed16_mul_approx(right_edge.u - left_edge.u, overWidth);				// can we do more approx mul here ??	//	will this be safe to run in fixed8_t
			fixed16_t dv = fixed16_mul_approx2(_graphics_right_edge.v - _graphics_left_edge.v, overWidth);


			uint16_t* output_span_ptr = output_scanline_ptr + (left_edge_floor_x_int >> 1);		//	this should really be uint16_t as VRAM writes are meant to be limited to 16-bit width



			//	2 bits per depth value ...  uint8_t => 4 values 
			//	VRAM output buffer is  16-bit wide ... uint16_t  => 2 values 

			//	if we can be in sync with wrapping of both depth value and VRAM output that would be ideal. 


			if (left_edge_floor_x_int & 0x01)
			{
				uint16_t output_value = *output_span_ptr & 0x00FF;

				//	Write Texel to DrawBuffer's scanline
				output_value |= sample_texture(&u, &v, du, dv) << 8;

				*output_span_ptr = output_value;

				output_span_ptr++;
				width -= 1;

				left_edge_floor_x_int++;
			}

			while (width >= 2)
			{
				uint16_t output_value = *output_span_ptr;

				//	Write Texel to DrawBuffer's scanline
				output_value = (output_value & 0xFF00) | sample_texture(&u, &v, du, dv);

				//	Write Texel to DrawBuffer's scanline
				output_value = (output_value & 0x00FF) | (sample_texture(&u, &v, du, dv) << 8);

				*output_span_ptr = output_value;

				left_edge_floor_x_int += 2;

				output_span_ptr++;

				width -= 2;
			}

			if (width)
			{
				// Can cut some corners on the last pixel

				uint16_t output_value = *output_span_ptr & 0xFF00;

				//	Write Texel to DrawBuffer's scanline
				output_value |= sample_texture(&u, &v, du, dv);
				*output_span_ptr = output_value;
			}

		}

		_graphics_left_edge.x += _graphics_left_edge.dx;
		_graphics_left_edge.u += _graphics_left_edge.du;
		_graphics_left_edge.v += _graphics_left_edge.dv;

		_graphics_right_edge.x += _graphics_right_edge.dx;
		_graphics_right_edge.u += _graphics_right_edge.du;
		_graphics_right_edge.v += _graphics_right_edge.dv;

		output_scanline_ptr += g_graphics_context.width >> 1;	//	cache this width var, shifting here as scanline is 16bit wide, which represents 2 pixels 
	}
}

//-----------------------------------------------------------------------------
void draw_spans_depth(void)
{
	fixed16_t fheight = fixed16_min(_graphics_left_edge.height, _graphics_right_edge.height);	//	one of the edges might continue to be used for the next draw_spans, so heights might be different 

	uint16_t* output_scanline_ptr = g_graphics_context.frame_buffer + (fixed16_to_int(_graphics_left_edge.y) * g_graphics_context.width);		//	output_scanline_ptr is 16-bit wide as VRAM requires writes of 16-bit wide

	uint8_t* depthmap_scanline_ptr = (fixed16_to_int(_graphics_left_edge.y) * (g_graphics_context.width >> 2)) + bound_depthmap->data;

	_graphics_left_edge.height -= fheight;
	_graphics_right_edge.height -= fheight;

	_graphics_left_edge.y += fheight;
	_graphics_right_edge.y += fheight;

	int32_t iheight = fixed16_to_int(fheight);

	while (iheight-- > 0)
	{
		int32_t left_edge_floor_x_int = fixed16_to_int(_graphics_left_edge.x);	//	could we be happy to floor it would be faster 
		int32_t right_edge_floor_x_int = fixed16_to_int(_graphics_right_edge.x);

		//	Calculate Vertex Attribute Gradients
		int32_t width = right_edge_floor_x_int - left_edge_floor_x_int;

		//debug_printf(DEBUG_LOG_DEBUG, "width = %i", width);

		if (width <= 0 &&						//	is this worth having 
			_graphics_left_edge.dx > 0 &&
			_graphics_right_edge.dx < 0)
		{
			return;
		}

		if (width > 0)		//	why are we getting a load of zero width ... 
		{
			fixed16_t overWidth = reciprocal_lut[width];	//	IWRAM lookup table ??? 

			fixed16_t u = _graphics_left_edge.u;
			fixed16_t v = _graphics_left_edge.v;
			fixed16_t du = fixed16_mul_approx2(_graphics_right_edge.u - _graphics_left_edge.u, overWidth);	//fixed16_mul_approx(right_edge.u - left_edge.u, overWidth);				// can we do more approx mul here ??	//	will this be safe to run in fixed8_t
			fixed16_t dv = fixed16_mul_approx2(_graphics_right_edge.v - _graphics_left_edge.v, overWidth);


			uint16_t* output_span_ptr = output_scanline_ptr + (left_edge_floor_x_int >> 1);		//	this should really be uint16_t as VRAM writes are meant to be limited to 16-bit width


			//	depthmap idx 
			uint32_t depthvalue = ((uint32_t*)(depthmap_scanline_ptr + ((left_edge_floor_x_int & ~0x0F) >> 2)))[0];		//	convert to uint32_t
			depthvalue >>= (left_edge_floor_x_int & 0x0F) << 1;


			//	2 bits per depth value ...  uint8_t => 4 values 
			//	VRAM output buffer is  16-bit wide ... uint16_t  => 2 values 

			//	if we can be in sync with wrapping of both depth value and VRAM output that would be ideal. 


			if (left_edge_floor_x_int & 0x01)
			{
				if ((depthvalue & 0x03) <= bound_depthlayer) //second pixel 	
				{
					uint16_t output_value = *output_span_ptr & 0x00FF;

					//	Write Texel to DrawBuffer's scanline
					output_value |= sample_texture(&u, &v, du, dv) << 8;

					*output_span_ptr = output_value;
				}

				output_span_ptr++;
				width -= 1;

				left_edge_floor_x_int++;
				depthvalue >>= 2;
				if ((left_edge_floor_x_int & 0x0F) == 0)
					depthvalue = ((uint32_t*)(depthmap_scanline_ptr + ((left_edge_floor_x_int & ~0x0F) >> 2)))[0];
			}

			while (width >= 2)
			{
				uint16_t output_value = *output_span_ptr;

				if ((depthvalue & 0x03) <= bound_depthlayer) //	first pixel
				{
					//	Write Texel to DrawBuffer's scanline
					output_value = (output_value & 0xFF00) | sample_texture(&u, &v, du, dv);
				}
				depthvalue >>= 2;


				if ((depthvalue & 0x03) <= bound_depthlayer) //second pixel
				{
					//	Write Texel to DrawBuffer's scanline
					output_value = (output_value & 0x00FF) | (sample_texture(&u, &v, du, dv) << 8);

				}
				depthvalue >>= 2;

				*output_span_ptr = output_value;

				left_edge_floor_x_int += 2;
				if ((left_edge_floor_x_int & 0x0F) == 0)
					depthvalue = ((uint32_t*)(depthmap_scanline_ptr + ((left_edge_floor_x_int & ~0x0F) >> 2)))[0];

				output_span_ptr++;

				width -= 2;
			}

			if (width)
			{
				// Can cut some corners on the last pixel

				if ((depthvalue & 0x03) <= bound_depthlayer) //second pixel 
				{
					uint16_t output_value = *output_span_ptr & 0xFF00;

					//	Write Texel to DrawBuffer's scanline
					output_value |= sample_texture(&u, &v, du, dv);
					*output_span_ptr = output_value;
				}
			}

		}

		_graphics_left_edge.x += _graphics_left_edge.dx;
		_graphics_left_edge.u += _graphics_left_edge.du;
		_graphics_left_edge.v += _graphics_left_edge.dv;

		_graphics_right_edge.x += _graphics_right_edge.dx;
		_graphics_right_edge.u += _graphics_right_edge.du;
		_graphics_right_edge.v += _graphics_right_edge.dv;

		output_scanline_ptr += g_graphics_context.width >> 1;	//	cache this width var, shifting here as scanline is 16bit wide, which represents 2 pixels 

		depthmap_scanline_ptr += g_graphics_context.width >> 2;
	}
}
