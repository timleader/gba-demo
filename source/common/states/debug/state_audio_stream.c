
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
typedef struct st_audio_stream_context_s
{
	audioclip_ptr audioclip;	

	audio_stream_handle_t stream_handle;

} st_audio_stream_context_t;

typedef st_audio_stream_context_t* st_audio_stream_context_ptr;



//-----------------------------------------------------------------------------
void st_audio_stream_enter(st_audio_stream_context_ptr context, uint32_t parameter)
{
	uint16_t resource_id = parameter & 0x0000FFFF;

	context->audioclip = resources_find_audioclip(resource_id);

	context->stream_handle = audio_stream_open(context->audioclip, AUDIO_STREAM_MUSIC);
}

//-----------------------------------------------------------------------------
void st_audio_stream_exit(st_audio_stream_context_ptr context)
{
	audio_stream_close(context->stream_handle);
}


//-----------------------------------------------------------------------------
void st_audio_stream_update(st_audio_stream_context_ptr context, fixed16_t dt)
{
	if (key_hit(KI_B))
	{
		state_pop(0);
	}
	else if (key_hit(KI_SELECT))
	{
		state_push(&st_analysis, 0);
	}

	if (key_hit(KI_A))
	{
		context->stream_handle = audio_stream_open(context->audioclip, AUDIO_STREAM_MUSIC);
	}

	//	maybe output playback timer 
}

//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_audio_stream =
{
	(state_enter_func_ptr)st_audio_stream_enter,
	(state_exit_func_ptr)st_audio_stream_exit,

	NULL,
	NULL,

	(state_update_func_ptr)st_audio_stream_update,
	NULL,

	"st_audio_str",
	sizeof(st_audio_stream_context_t)
};
