
#include "states.h"

#include "common/memory.h"
#include "common/math/matrix.h"
#include "common/math/trigonometry.h"
#include "common/math/point.h"
#include "common/graphics/graphics.h"
#include "common/graphics/image.h"
#include "common/graphics/camera.h"
#include "common/input/input.h"
#include "common/resources/resources.h"
#include "common/utils/random1k.h"

//-----------------------------------------------------------------------------
typedef struct st_model_context_s	
{
	camera_t camera;
	matrix4x4_t WVP;

	vector3_t character_forwards;
	vector3_t character_position;
	vector3_t character_rotation;

	model_ptr g_character;
	image_ptr g_character_img;

	int32_t animation_idx;
	int32_t frame_idx;

} st_model_context_t;

typedef st_model_context_t* st_model_context_ptr;


//-----------------------------------------------------------------------------
void st_modelviewer_update(st_model_context_ptr context, fixed16_t dt)
{
	//	fly camera !!! 

	if (key_is_down(KI_UP))
	{
		context->character_rotation.x -= F16(0.05);
	}
	if (key_is_down(KI_DOWN))
	{
		context->character_rotation.x += F16(0.05);
	}
	if (key_is_down(KI_LEFT))
	{
		context->character_rotation.y += F16(0.05);
	}
	if (key_is_down(KI_RIGHT))
	{
		context->character_rotation.y -= F16(0.05);
	}

	if (key_is_down(KI_L))
	{
		context->animation_idx--;

		if (context->animation_idx < 0)
			context->animation_idx = context->g_character->animation_count - 1;

		context->frame_idx = 0;
	}

	if (key_is_down(KI_R))
	{
		context->animation_idx++;

		if (context->animation_idx > context->g_character->animation_count - 1)
			context->animation_idx = 0;

		context->frame_idx = 0;
	}

	if (key_hit(KI_B))
	{
		/*
		vector3_t move = { 0, 0, F16(1) };
		mathVector3Add(&st_modelviewer_data->camera.position, &st_modelviewer_data->camera.position, &move);
		*/
		state_pop(0);
	}

	if (key_hit(KI_SELECT))
	{
		state_push(&st_analysis, 0);
	}

	animation_t* animation = model_find_animation(context->g_character, context->animation_idx);

	context->frame_idx++;
	if (context->frame_idx > animation->frame_count - 1)
		context->frame_idx = 0;
}

//-----------------------------------------------------------------------------
void st_modelviewer_draw(st_model_context_ptr context, fixed16_t dt)
{
	//	graphics render 3D model with depth...

	/*
		test depthmap
		test triangle culling
		test more optimial draw
		test draw order
	*/

	matrix4x4_t M, R;

	memory_set32(g_graphics_context.frame_buffer, (g_graphics_context.width * g_graphics_context.height * sizeof(uint8_t)) >> 2, 0);

	graphics_bind_image(context->g_character_img);

	graphics_bind_depthmap((depthmap_t*)g_graphics_context.frame_buffer);	//	throw dummy data into here

	camera_build(&context->camera);
	mathMatrix4x4Copy(&context->WVP, &context->camera.projection);
	mathMatrix4x4Multiply(&context->WVP, &context->WVP, &context->camera.model);

	mathMatrix4x4MakeTranslation(&M, &context->character_position);
	mathMatrix4x4MakeRotationZYX(&R, &context->character_rotation);
	mathMatrix4x4Multiply(&M, &M, &R);

	mathMatrix4x4Multiply(&M, &context->WVP, &M);

	graphics_draw_model(context->g_character, (uint16_t)context->animation_idx, (uint16_t)context->frame_idx, &M);

	graphics_pageflip();
}

//-----------------------------------------------------------------------------
void st_modelviewer_enter(st_model_context_ptr context, uint32_t parameter)
{
	uint16_t resource_id = parameter & 0x0000FFFF;

	const fixed16_t deg2rad = 0x00000478;		//	F16(PI / 180)

	vector3_t camera_position = { 0, 0, 0 };

	cameraInitialize(&context->camera);
	camera_translate_local(&context->camera, &camera_position);
	camera_build(&context->camera);

	mathMatrix4x4Copy(&context->WVP, &context->camera.projection);
	mathMatrix4x4Multiply(&context->WVP, &context->camera.model, &context->WVP);

	mathVector3MakeFromElements(&context->character_position, fixed16_zero, fixed16_zero, fixed16_from_int(4));
	mathVector3MakeFromElements(&context->character_rotation, fixed16_mul(fixed16_zero, deg2rad), fixed16_mul(fixed16_from_int(-180), deg2rad), fixed16_zero);

	context->g_character = resources_find_model(resource_id);		
	context->g_character_img = resources_find_image(context->g_character->image_id);	

	palette_ptr pal = resources_find_palette(context->g_character_img->palette_id);
	graphics_write_palette(pal);

	context->animation_idx = 0;
	context->frame_idx = 0;
}


//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_model =
{
	(state_enter_func_ptr)st_modelviewer_enter,
	NULL,

	NULL,
	NULL,

	(state_update_func_ptr)st_modelviewer_update,
	(state_draw_func_ptr)st_modelviewer_draw,

	"st_model",
	sizeof(st_model_context_t)

};

