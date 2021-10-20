
#include "debug_helper.h"


int32_t debug_fixed16_to_string(char* output_buffer, fixed16_t value)
{
	int16_t whole_part = value >> 16;
	uint16_t fractional_part = value & 0x0000FFFF;

	int32_t output_count = sprintf(output_buffer, "%i.%u", whole_part, fractional_part);

	return output_count;
}

int32_t debug_vector3_to_string(char* output_buffer, vector3_t* vec3)
{
	int32_t output_total_count = 0;

	int32_t output_count = sprintf(output_buffer, "{");
	output_buffer += output_count;
	output_total_count += output_count;

	output_count = debug_fixed16_to_string(output_buffer, vec3->x);
	output_buffer += output_count;
	output_total_count += output_count;

	output_count = sprintf(output_buffer, ",");
	output_buffer += output_count;
	output_total_count += output_count;

	output_count = debug_fixed16_to_string(output_buffer, vec3->y);
	output_buffer += output_count;
	output_total_count += output_count;

	output_count = sprintf(output_buffer, ",");
	output_buffer += output_count;
	output_total_count += output_count;

	output_count = debug_fixed16_to_string(output_buffer, vec3->z);
	output_buffer += output_count;
	output_total_count += output_count;

	output_count = sprintf(output_buffer, "}");
	output_buffer += output_count;
	output_total_count += output_count;

	return output_total_count;
}

int32_t debug_vector4_to_string(char* output_buffer, vector4_t* vec4)
{
	int32_t output_total_count = 0;

	int32_t output_count = sprintf(output_buffer, "{");
	output_buffer += output_count;
	output_total_count += output_count;

	output_count = debug_fixed16_to_string(output_buffer, vec4->x);
	output_buffer += output_count;
	output_total_count += output_count;

	output_count = sprintf(output_buffer, ",");
	output_buffer += output_count;
	output_total_count += output_count;

	output_count = debug_fixed16_to_string(output_buffer, vec4->y);
	output_buffer += output_count;
	output_total_count += output_count;

	output_count = sprintf(output_buffer, ",");
	output_buffer += output_count;
	output_total_count += output_count;

	output_count = debug_fixed16_to_string(output_buffer, vec4->z);
	output_buffer += output_count;
	output_total_count += output_count;

	output_count = sprintf(output_buffer, ",");
	output_buffer += output_count;
	output_total_count += output_count;

	output_count = debug_fixed16_to_string(output_buffer, vec4->w);
	output_buffer += output_count;
	output_total_count += output_count;

	output_count = sprintf(output_buffer, "}");
	output_buffer += output_count;
	output_total_count += output_count;

	return output_total_count;
}

int32_t debug_triangle_to_string(char* output_buffer, vertex_t* triangle)
{
	int32_t output_count = 0;

	output_count = sprintf(output_buffer, "{");
	output_buffer += output_count;

	for (uint8_t triangle_idx = 0; triangle_idx < 3; ++triangle_idx)
	{
		output_count = sprintf(output_buffer, "\n\tv%u=", triangle_idx);
		output_buffer += output_count;

		output_count = debug_vector4_to_string(output_buffer, &triangle[triangle_idx].position);
		output_buffer += output_count;
	}

	output_count = sprintf(output_buffer, "\n}");
	output_buffer += output_count;

	return output_count;
}

int32_t debug_matrix4x4_to_string(char* output_buffer, matrix4x4_t* matrix)
{
	int32_t output_count = 0;

	output_count = sprintf(output_buffer, "{");
	output_buffer += output_count;

	vector4_t* columns = (vector4_t*)matrix;

	for (uint8_t col_idx = 0; col_idx < 4; ++col_idx)
	{
		output_count = sprintf(output_buffer, "\n\tcolumn%u=", col_idx);
		output_buffer += output_count;

		output_count = debug_vector4_to_string(output_buffer, &columns[col_idx]);
		output_buffer += output_count;
	}

	output_count = sprintf(output_buffer, "\n}");
	output_buffer += output_count;

	return output_count;
}
