
#include "graphics.h"


#include "common/debug/debug.h"


//-----------------------------------------------------------------------------
IWRAM_CODE uint8_t clip_polygon_right_edge(vertex_t* src, vertex_t* dest, uint8_t vertex_count, fixed16_t x_clip_value)
{
	vertex_t* v = &src[0];

	vertex_t* new_vertices = &dest[0];
	int32_t new_vertex_idx = 0;

	//	this is just right side, also need to do top, left and bottom 
	//		should this actually only go to 159 
	for (int32_t i = 0; i < vertex_count; ++i)
	{
		int32_t k = i + 1;
		if (i >= (vertex_count - 1))
			k = 0;

		vertex_t* i_v = &v[i];
		vertex_t* k_v = &v[k];

		if (i_v->position.x < x_clip_value &&
			k_v->position.x < x_clip_value)
		{
			mathVector3Copy((vector3_t*)&new_vertices[new_vertex_idx].position, (const vector3_t*)&k_v->position);
			mathVector2Copy(&new_vertices[new_vertex_idx].uv, &k_v->uv);
			new_vertex_idx++;
		}
		else if (
			i_v->position.x >= x_clip_value &&
			k_v->position.x < x_clip_value)	// Case 2: When only first point is outside
		{
			//	over clipping 
			vector2_t polygon_delta;
			mathVector2Substract(&polygon_delta, (vector2_t*)&k_v->position, (const vector2_t*)&i_v->position);
			fixed16_t original_x_diff = polygon_delta.x;

			fixed16_t scale_factor = fixed16_div(fixed16_one, polygon_delta.x);			//	lut is possible here
			mathVector2ScalarMultiply(&polygon_delta, &polygon_delta, scale_factor);	//	forces x=1 

			fixed16_t x_diff = x_clip_value - i_v->position.x;

			fixed16_t t = fixed16_div(x_diff, original_x_diff);		//	lut is possible here 

			debug_assert(t <= fixed16_one, "t is invalid");
			debug_assert(t >= fixed16_zero, "t is invalid");

			fixed16_t du = k_v->uv.x - i_v->uv.x;
			fixed16_t dv = k_v->uv.y - i_v->uv.y;

			new_vertices[new_vertex_idx].position.x = x_clip_value;
			new_vertices[new_vertex_idx].position.y = i_v->position.y + fixed16_mul(polygon_delta.y, x_diff);

			new_vertices[new_vertex_idx].uv.x = i_v->uv.x + fixed16_mul(du, t);
			new_vertices[new_vertex_idx].uv.y = i_v->uv.y + fixed16_mul(dv, t);

			new_vertex_idx++;

			mathVector3Copy(&new_vertices[new_vertex_idx].position, &k_v->position);
			mathVector2Copy(&new_vertices[new_vertex_idx].uv, &k_v->uv);
			new_vertex_idx++;

		}
		else if (
			i_v->position.x < x_clip_value &&
			k_v->position.x >= x_clip_value)	// Case 3: When only second point is outside
		{
			vector2_t polygon_delta;
			mathVector2Substract(&polygon_delta, &k_v->position, &i_v->position);
			fixed16_t original_x_diff = polygon_delta.x;

			fixed16_t scale_factor = fixed16_div(fixed16_one, polygon_delta.x);			//	lut is possible here
			mathVector2ScalarMultiply(&polygon_delta, &polygon_delta, scale_factor);	//	forces x=1 

			fixed16_t x_diff = x_clip_value - i_v->position.x;

			fixed16_t t = fixed16_div(x_diff, original_x_diff);		//	lut is possible here 

			debug_assert(t <= fixed16_one, "t is invalid");
			debug_assert(t >= fixed16_zero, "t is invalid");

			fixed16_t du = k_v->uv.x - i_v->uv.x;
			fixed16_t dv = k_v->uv.y - i_v->uv.y;


			new_vertices[new_vertex_idx].position.x = x_clip_value;
			new_vertices[new_vertex_idx].position.y = i_v->position.y + fixed16_mul(polygon_delta.y, x_diff);

			new_vertices[new_vertex_idx].uv.x = i_v->uv.x + fixed16_mul(du, t);
			new_vertices[new_vertex_idx].uv.y = i_v->uv.y + fixed16_mul(dv, t);
			new_vertex_idx++;
		}
		else // Case 4: When both points are outside
		{
			//No points are added
		}
	}

	return new_vertex_idx;
}

//-----------------------------------------------------------------------------
IWRAM_CODE uint8_t clip_polygon_left_edge(vertex_t* src, vertex_t* dest, uint8_t vertex_count, fixed16_t x_clip_value)
{
	vertex_t* v = &src[0];

	vertex_t* new_vertices = &dest[0];
	int32_t new_vertex_idx = 0;

	//	this is just right side, also need to do top, left and bottom 
	//		should this actually only go to 159 
	for (int32_t i = 0; i < vertex_count; ++i)
	{
		int32_t k = i + 1;
		if (i >= (vertex_count - 1))
			k = 0;

		vertex_t* i_v = &v[i];
		vertex_t* k_v = &v[k];

		if (i_v->position.x > x_clip_value &&
			k_v->position.x > x_clip_value)
		{
			mathVector3Copy(&new_vertices[new_vertex_idx].position, &k_v->position);
			mathVector2Copy(&new_vertices[new_vertex_idx].uv, &k_v->uv);
			new_vertex_idx++;
		}
		else if (
			i_v->position.x < x_clip_value &&
			k_v->position.x >= x_clip_value)	// Case 2: When only first point is outside
		{
			//	over clipping 
			vector2_t polygon_delta;
			mathVector2Substract(&polygon_delta, &k_v->position, &i_v->position);
			fixed16_t original_x_diff = polygon_delta.x;

			fixed16_t scale_factor = fixed16_div(fixed16_one, polygon_delta.x);			//	lut is possible here
			mathVector2ScalarMultiply(&polygon_delta, &polygon_delta, scale_factor);	//	forces x=1 

			fixed16_t x_diff = x_clip_value - i_v->position.x;

			fixed16_t t = fixed16_div(x_diff, original_x_diff);		//	lut is possible here 

			debug_assert(t <= fixed16_one, "t is invalid");
			debug_assert(t >= fixed16_zero, "t is invalid");

			fixed16_t du = k_v->uv.x - i_v->uv.x;
			fixed16_t dv = k_v->uv.y - i_v->uv.y;

			new_vertices[new_vertex_idx].position.x = x_clip_value;
			new_vertices[new_vertex_idx].position.y = i_v->position.y + fixed16_mul(polygon_delta.y, x_diff);

			new_vertices[new_vertex_idx].uv.x = i_v->uv.x + fixed16_mul(du, t);
			new_vertices[new_vertex_idx].uv.y = i_v->uv.y + fixed16_mul(dv, t);

			new_vertex_idx++;

			mathVector3Copy(&new_vertices[new_vertex_idx].position, &k_v->position);
			mathVector2Copy(&new_vertices[new_vertex_idx].uv, &k_v->uv);
			new_vertex_idx++;

		}
		else if (
			i_v->position.x >= x_clip_value &&
			k_v->position.x < x_clip_value)	// Case 3: When only second point is outside
		{
			vector2_t polygon_delta;
			mathVector2Substract(&polygon_delta, &k_v->position, &i_v->position);
			fixed16_t original_x_diff = polygon_delta.x;

			fixed16_t scale_factor = fixed16_div(fixed16_one, polygon_delta.x);			//	lut is possible here
			mathVector2ScalarMultiply(&polygon_delta, &polygon_delta, scale_factor);	//	forces x=1 

			fixed16_t x_diff = x_clip_value - i_v->position.x;

			fixed16_t t = fixed16_div(x_diff, original_x_diff);		//	lut is possible here 

			debug_assert(t <= fixed16_one, "t is invalid");
			debug_assert(t >= fixed16_zero, "t is invalid");

			fixed16_t du = k_v->uv.x - i_v->uv.x;
			fixed16_t dv = k_v->uv.y - i_v->uv.y;


			new_vertices[new_vertex_idx].position.x = x_clip_value;
			new_vertices[new_vertex_idx].position.y = i_v->position.y + fixed16_mul(polygon_delta.y, x_diff);

			new_vertices[new_vertex_idx].uv.x = i_v->uv.x + fixed16_mul(du, t);
			new_vertices[new_vertex_idx].uv.y = i_v->uv.y + fixed16_mul(dv, t);
			new_vertex_idx++;
		}
		else // Case 4: When both points are outside
		{
			//No points are added
		}
	}

	return new_vertex_idx;
}

//-----------------------------------------------------------------------------
IWRAM_CODE uint8_t clip_polygon_top_edge(vertex_t* src, vertex_t* dest, uint8_t vertex_count, fixed16_t y_clip_value)
{
	vertex_t* v = &src[0];

	vertex_t* new_vertices = &dest[0];
	int32_t new_vertex_idx = 0;

	//	this is just right side, also need to do top, left and bottom 
	//		should this actually only go to 159 
	for (int32_t i = 0; i < vertex_count; ++i)
	{
		int32_t k = i + 1;
		if (i >= (vertex_count - 1))
			k = 0;

		vertex_t* i_v = &v[i];
		vertex_t* k_v = &v[k];

		if (i_v->position.y > y_clip_value &&
			k_v->position.y > y_clip_value)
		{
			mathVector3Copy(&new_vertices[new_vertex_idx].position, &k_v->position);
			mathVector2Copy(&new_vertices[new_vertex_idx].uv, &k_v->uv);
			new_vertex_idx++;
		}
		else if (
			i_v->position.y < y_clip_value &&
			k_v->position.y >= y_clip_value)	// Case 2: When only first point is outside
		{
			//	over clipping 
			vector2_t polygon_delta;
			mathVector2Substract(&polygon_delta, &k_v->position, &i_v->position);
			fixed16_t original_y_diff = polygon_delta.y;

			fixed16_t scale_factor = fixed16_div(fixed16_one, polygon_delta.y);			//	lut is possible here
			mathVector2ScalarMultiply(&polygon_delta, &polygon_delta, scale_factor);	//	forces x=1 

			fixed16_t y_diff = y_clip_value - i_v->position.y;

			fixed16_t t = fixed16_div(y_diff, original_y_diff);		//	lut is possible here 

			debug_assert(t <= fixed16_one, "t is invalid");
			debug_assert(t >= fixed16_zero, "t is invalid");

			fixed16_t du = k_v->uv.x - i_v->uv.x;
			fixed16_t dv = k_v->uv.y - i_v->uv.y;

			new_vertices[new_vertex_idx].position.x = i_v->position.x + fixed16_mul(polygon_delta.x, y_diff);
			new_vertices[new_vertex_idx].position.y = y_clip_value;

			new_vertices[new_vertex_idx].uv.x = i_v->uv.x + fixed16_mul(du, t);
			new_vertices[new_vertex_idx].uv.y = i_v->uv.y + fixed16_mul(dv, t);

			new_vertex_idx++;

			mathVector3Copy(&new_vertices[new_vertex_idx].position, &k_v->position);
			mathVector2Copy(&new_vertices[new_vertex_idx].uv, &k_v->uv);
			new_vertex_idx++;

		}
		else if (
			i_v->position.y >= y_clip_value &&
			k_v->position.y < y_clip_value)	// Case 3: When only second point is outside
		{
			vector2_t polygon_delta;
			mathVector2Substract(&polygon_delta, &k_v->position, &i_v->position);
			fixed16_t original_y_diff = polygon_delta.y;

			fixed16_t scale_factor = fixed16_div(fixed16_one, polygon_delta.y);			//	lut is possible here
			mathVector2ScalarMultiply(&polygon_delta, &polygon_delta, scale_factor);	//	forces x=1 

			fixed16_t y_diff = y_clip_value - i_v->position.y;

			fixed16_t t = fixed16_div(y_diff, original_y_diff);		//	lut is possible here 

			debug_assert(t <= fixed16_one, "t is invalid");
			debug_assert(t >= fixed16_zero, "t is invalid");

			fixed16_t du = k_v->uv.x - i_v->uv.x;
			fixed16_t dv = k_v->uv.y - i_v->uv.y;

			new_vertices[new_vertex_idx].position.x = i_v->position.x + fixed16_mul(polygon_delta.x, y_diff);
			new_vertices[new_vertex_idx].position.y = y_clip_value;

			new_vertices[new_vertex_idx].uv.x = i_v->uv.x + fixed16_mul(du, t);
			new_vertices[new_vertex_idx].uv.y = i_v->uv.y + fixed16_mul(dv, t);
			new_vertex_idx++;
		}
		else // Case 4: When both points are outside
		{
			//No points are added
		}
	}

	return new_vertex_idx;
}

//-----------------------------------------------------------------------------
IWRAM_CODE uint8_t clip_polygon_bottom_edge(vertex_t* src, vertex_t* dest, uint8_t vertex_count, fixed16_t y_clip_value)
{
	vertex_t* v = &src[0];

	vertex_t* new_vertices = &dest[0];
	int32_t new_vertex_idx = 0;

	//	this is just right side, also need to do top, left and bottom 
	//		should this actually only go to 159 
	for (int32_t i = 0; i < vertex_count; ++i)
	{
		int32_t k = i + 1;
		if (i >= (vertex_count - 1))
			k = 0;

		vertex_t* i_v = &v[i];
		vertex_t* k_v = &v[k];

		if (i_v->position.y < y_clip_value &&
			k_v->position.y < y_clip_value)
		{
			mathVector3Copy(&new_vertices[new_vertex_idx].position, &k_v->position);
			mathVector2Copy(&new_vertices[new_vertex_idx].uv, &k_v->uv);
			new_vertex_idx++;
		}
		else if (
			i_v->position.y >= y_clip_value &&
			k_v->position.y < y_clip_value)	// Case 2: When only first point is outside
		{
			//	over clipping 
			vector2_t polygon_delta;
			mathVector2Substract(&polygon_delta, &k_v->position, &i_v->position);
			fixed16_t original_y_diff = polygon_delta.y;

			fixed16_t scale_factor = fixed16_div(fixed16_one, polygon_delta.y);			//	lut is possible here
			mathVector2ScalarMultiply(&polygon_delta, &polygon_delta, scale_factor);	//	forces x=1 

			fixed16_t y_diff = y_clip_value - i_v->position.y;

			fixed16_t t = fixed16_div(y_diff, original_y_diff);		//	lut is possible here 

			debug_assert(t <= fixed16_one, "t is invalid");
			debug_assert(t >= fixed16_zero, "t is invalid");

			fixed16_t du = k_v->uv.x - i_v->uv.x;
			fixed16_t dv = k_v->uv.y - i_v->uv.y;

			new_vertices[new_vertex_idx].position.x = i_v->position.x + fixed16_mul(polygon_delta.x, y_diff);
			new_vertices[new_vertex_idx].position.y = y_clip_value;

			new_vertices[new_vertex_idx].uv.x = i_v->uv.x + fixed16_mul(du, t);
			new_vertices[new_vertex_idx].uv.y = i_v->uv.y + fixed16_mul(dv, t);

			new_vertex_idx++;

			mathVector3Copy(&new_vertices[new_vertex_idx].position, &k_v->position);
			mathVector2Copy(&new_vertices[new_vertex_idx].uv, &k_v->uv);
			new_vertex_idx++;

		}
		else if (
			i_v->position.y < y_clip_value &&
			k_v->position.y >= y_clip_value)	// Case 3: When only second point is outside
		{
			vector2_t polygon_delta;
			mathVector2Substract(&polygon_delta, &k_v->position, &i_v->position);
			fixed16_t original_y_diff = polygon_delta.y;

			fixed16_t scale_factor = fixed16_div(fixed16_one, polygon_delta.y);			//	lut is possible here
			mathVector2ScalarMultiply(&polygon_delta, &polygon_delta, scale_factor);	//	forces x=1 

			fixed16_t y_diff = y_clip_value - i_v->position.y;

			fixed16_t t = fixed16_div(y_diff, original_y_diff);		//	lut is possible here 

			debug_assert(t <= fixed16_one, "t is invalid");
			debug_assert(t >= fixed16_zero, "t is invalid");

			fixed16_t du = k_v->uv.x - i_v->uv.x;
			fixed16_t dv = k_v->uv.y - i_v->uv.y;

			new_vertices[new_vertex_idx].position.x = i_v->position.x + fixed16_mul(polygon_delta.x, y_diff);
			new_vertices[new_vertex_idx].position.y = y_clip_value;

			new_vertices[new_vertex_idx].uv.x = i_v->uv.x + fixed16_mul(du, t);
			new_vertices[new_vertex_idx].uv.y = i_v->uv.y + fixed16_mul(dv, t);
			new_vertex_idx++;
		}
		else // Case 4: When both points are outside
		{
			//No points are added
		}
	}

	return new_vertex_idx;
}

//-----------------------------------------------------------------------------
IWRAM_CODE uint8_t clip_polygon(vertex_t* src, vertex_t* dest, uint8_t vertex_count)
{
	vertex_t tmp0[6];
	vertex_t tmp1[6];

	vertex_count = clip_polygon_top_edge(src, tmp0, vertex_count, fixed16_zero);
	vertex_count = clip_polygon_bottom_edge(tmp0, tmp1, vertex_count, g_graphics_context.heightMinusOne);
	vertex_count = clip_polygon_left_edge(tmp1, tmp0, vertex_count, fixed16_zero);
	vertex_count = clip_polygon_right_edge(tmp0, dest, vertex_count, g_graphics_context.widthMinusOne);

	return vertex_count;
}
