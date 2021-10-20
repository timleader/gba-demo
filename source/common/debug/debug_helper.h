
#ifndef DEBUG_HELPER_H
#define DEBUG_HELPER_H

#include "common/math/fixed16.h"
#include "common/graphics/graphics.h"

#include <stdio.h>

int32_t debug_fixed16_to_string(char* output_buffer, fixed16_t value);

int32_t debug_vector3_to_string(char* output_buffer, vector3_t* vec3);

int32_t debug_triangle_to_string(char* output_buffer, vertex_t* triangle);

int32_t debug_matrix4x4_to_string(char* output_buffer, matrix4x4_t* matrix);

#endif
