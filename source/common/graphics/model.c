
#include "model.h"

#include "common/debug/debug.h"
#include "common/string.h"


animation_t* model_find_animation_from_name(model_ptr model, const char* name)
{
	debug_assert(string_length(name) < 12, "model::find_animation_from_name name too long");

	uint16_t animation_clip_size = sizeof(animation_t) + (model->vertices_count * sizeof(packed_vector3_t));

	for (int32_t i = 0; i < model->animation_count; ++i)
	{
		uint8_t* animations_byte_ptr = (uint8_t*)model->animations;
		animation_t* animation = (animation_t*)(animations_byte_ptr + (animation_clip_size * i));
		if (string_compare(name, animation->name) == 0)
		{
			return animation;
		}
	}

	return NULL;
}

int16_t model_find_animation_index_of(model_ptr model, const char* name)
{
	debug_assert(string_length(name) < 12, "model::find_animation_index_of name too long");

	uint16_t animation_clip_size = sizeof(animation_t) + (model->vertices_count * sizeof(packed_vector3_t));

	for (int32_t i = 0; i < model->animation_count; ++i)
	{
		uint8_t* animations_byte_ptr = (uint8_t*)model->animations;
		animation_t* animation = (animation_t*)(animations_byte_ptr + (animation_clip_size * i));
		if (string_compare(name, animation->name) == 0)
		{
			return i;
		}
	}

	debug_assert(0, "model::find_animation_index_of not found");
	return -1;
}
