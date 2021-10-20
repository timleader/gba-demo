
#include "states.h"

#include "common/math/matrix.h"
#include "common/graphics/graphics.h"
#include "common/graphics/image.h"
#include "common/graphics/camera.h"
#include "common/graphics/text.h"
#include "common/collision/collision.h"
#include "common/resources/resources.h"
#include "common/input/input.h"
#include "common/audio/audio.h"
#include "common/utils/bitstream.h"
#include "common/debug/debug.h"
#include "common/memory.h"

//-----------------------------------------------------------------------------
typedef struct st_audio_sfx_context_s
{
	audio_sfx_handle_t sfx_handle;
	audioclip_ptr soundclip;

	camera_t camera;
	matrix4x4_t WVP;

	vector3_t emitter_position;
	vector3_t emitter_rotation;

	vector3_t listener_position;
	vector3_t listener_rotation;

	model_ptr debug_mdl;
	model_ptr floor_mdl;

	perf_timer_t timer;

} st_audio_sfx_context_t;

typedef st_audio_sfx_context_t* st_audio_sfx_context_ptr;

//-----------------------------------------------------------------------------
void st_audio_sfx_enter(st_audio_sfx_context_ptr context, uint32_t parameter)
{
	uint16_t resource_id = parameter & 0x0000FFFF;

	context->soundclip = resources_find_audioclip(resource_id);

	//	configure for 3D 
	const fixed16_t deg2rad = 0x00000478;		//	F16(PI / 180)

	vector3_t camera_position = { 0, fixed16_from_int(2), fixed16_from_int(-8) };

	cameraInitialize(&context->camera);
	camera_translate_local(&context->camera, &camera_position);
	camera_build(&context->camera);
	mathMatrix4x4Copy(&context->WVP, &context->camera.projection);
	mathMatrix4x4Multiply(&context->WVP, &context->camera.model, &context->WVP);

	mathVector3MakeFromElements(&context->emitter_position, fixed16_from_int(1) , fixed16_zero, fixed16_zero);
	mathVector3MakeFromElements(&context->emitter_rotation, fixed16_zero, fixed16_zero, fixed16_zero);

	context->debug_mdl = resources_find_model_from_name("mdl/cube");
	image_ptr image = resources_find_image(context->debug_mdl->image_id);
	palette_ptr palette = resources_find_palette(image->palette_id);

	context->floor_mdl = resources_find_model_from_name("mdl/floor");

	graphics_write_palette(palette);




	context->timer = timer_start(4 * 30, TIMER_MODE_LOOP);

	// x = sin(timer.progress);
	// z = cos(timer.progress);


	//audio_sfx_position(context->sfx_handle, pos);



	//	over time rotate around the camera ... 
	//	


	//	need 3D visualization ... 


	//	sound efx [4]  eg. Walking, etc..		
	//		Mono 3D sound, distance, falloff curve etc... 

	vector3_t listener_pos = { 0, 0, 0 };
	audio_sfx_set_listener_position(listener_pos);

	vector3_t emitter_pos = { fixed16_one, 0, 0 };
	context->sfx_handle = audio_sfx_play_3d(context->soundclip, emitter_pos, 0);		//	should we just pass resource_id into here ??? 

	//audio_sfx_volume()

}

//-----------------------------------------------------------------------------
void st_audio_sfx_exit(st_audio_sfx_context_ptr context)
{
	audio_sfx_cancel(context->sfx_handle);
}


//-----------------------------------------------------------------------------
void st_audio_sfx_update(st_audio_sfx_context_ptr context, fixed16_t dt)
{
	if (key_hit(KI_B))
	{
		state_pop(0);
	}
	else if (key_hit(KI_SELECT))
	{
		state_push(&st_analysis, 0);
	}

	if (key_hit(KI_LEFT))
	{
		vector3_t emitter_pos = { -fixed16_one, 0, 0 };
		context->sfx_handle = audio_sfx_play_3d(context->soundclip, emitter_pos, 0);		//	should we just pass resource_id into here ??? 
	}
	if (key_hit(KI_RIGHT))
	{
		vector3_t emitter_pos = { fixed16_one, 0, 0 };
		context->sfx_handle = audio_sfx_play_3d(context->soundclip, emitter_pos, 0);		//	should we just pass resource_id into here ??? 
	}
}

//-----------------------------------------------------------------------------
void st_audio_sfx_draw(st_audio_sfx_context_ptr context, fixed16_t dt)
{
	matrix4x4_t M, R;
	image_ptr image = resources_find_image(context->debug_mdl->image_id);


	graphics_clear(0);

	graphics_bind_image(image);
	graphics_bind_depthmap((depthmap_t*)g_graphics_context.frame_buffer);	//	throw dummy data into here



	camera_build(&context->camera);
	mathMatrix4x4Copy(&context->WVP, &context->camera.projection);
	mathMatrix4x4Multiply(&context->WVP, &context->WVP, &context->camera.model);

	mathMatrix4x4MakeTranslation(&M, &context->emitter_position);
	mathMatrix4x4MakeRotationZYX(&R, &context->emitter_rotation);
	mathMatrix4x4Multiply(&M, &M, &R);

	mathMatrix4x4Multiply(&M, &context->WVP, &M);

	//graphics_draw_model(context->debug_mdl, 0, &M);



	graphics_pageflip();
}

//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_audio_sfx =
{
	(state_enter_func_ptr)st_audio_sfx_enter,
	(state_exit_func_ptr)st_audio_sfx_exit,

	NULL,
	NULL,

	(state_update_func_ptr)st_audio_sfx_update,
	(state_draw_func_ptr)st_audio_sfx_draw,

	"st_audio_sfx",
	sizeof(st_audio_sfx_context_t)

};
