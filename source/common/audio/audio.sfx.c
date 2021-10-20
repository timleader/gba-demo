
#include "audio.internal.h"

#include "common/debug/debug.h"

//	SFX	

//	maybe pass a property for 3D position, fall off curve 


audio_sfx_handle_t audio_sfx_play_ex(audioclip_ptr clip, uint8_t priority)
{
	debug_assert(clip->format == AUDIO_FORMAT_PCM, "audio::sfx doesn't support compressed audio clips");
	debug_assert(clip->sample_rate == AUDIO_SAMPLE_RATE_FULL, "audio::sfx doesn't support compressed audio clips");

	int8_t channel_idx = _audio_find_free_channel(0);
	if (channel_idx < 0)
		return 0;

	audio_sfx_handle_t handle = _create_audio_sfx_handle(channel_idx, 0);	//	encode priority in handle 

	g_audio_context.active_handles[channel_idx] = handle;

	audio_channel_ptr channel = &g_audio_context.channels[channel_idx];

	int8_t* samples = (int8_t*)clip->data;
	uint32_t sample_count = clip->sample_count;

	channel->active = 1; 
	channel->buffer = samples;
	channel->buffer_size = sample_count;
	channel->read_position = 0;
	channel->type = 0;
	channel->volume = 255;

	return handle;
}

audio_sfx_handle_t audio_sfx_play_3d(audioclip_ptr clip, vector3_t position, uint8_t priority)
{
	debug_assert(clip->format == AUDIO_FORMAT_PCM, "audio::sfx doesn't support compressed audio clips");
	debug_assert(clip->sample_rate == AUDIO_SAMPLE_RATE_FULL, "audio::sfx doesn't support compressed audio clips");

	int8_t channel_idx = _audio_find_free_channel(0);
	if (channel_idx < 0)
		return 0;

	audio_sfx_handle_t handle = _create_audio_sfx_handle(channel_idx, 0);	//	encode priority in handle 

	g_audio_context.active_handles[channel_idx] = handle;

	audio_channel_ptr channel = &g_audio_context.channels[channel_idx];

	int8_t* samples = (int8_t*)clip->data;
	uint32_t sample_count = clip->sample_count;

	channel->active = 1;
	channel->buffer = samples;
	channel->buffer_size = sample_count;
	channel->read_position = 0;
	channel->type = 0;
	channel->position = position;
	channel->volume = 255;

	return handle;
}

audio_sfx_handle_t audio_sfx_play(audioclip_ptr clip)	
{
	return audio_sfx_play_ex(clip, 0);
}


void audio_sfx_volume(audio_sfx_handle_t sfx_handle, uint8_t volume)
{
	uint8_t channel_idx = _audio_get_channel_from_handle(sfx_handle);

	if (g_audio_context.active_handles[channel_idx] != sfx_handle)
		return;

	g_audio_context.channels[channel_idx].volume = volume;
}

void audio_sfx_panning(audio_sfx_handle_t sfx_handle, int8_t panning)
{
	uint8_t channel_idx = _audio_get_channel_from_handle(sfx_handle);

	if (g_audio_context.active_handles[channel_idx] != sfx_handle)
		return;

	g_audio_context.channels[channel_idx].panning = panning;
}

void audio_sfx_3d_position(audio_sfx_handle_t sfx_handle, vector3_t position)
{
	uint8_t channel_idx = _audio_get_channel_from_handle(sfx_handle);

	if (g_audio_context.active_handles[channel_idx] != sfx_handle)
		return;

	g_audio_context.channels[channel_idx].position = position;
}

void audio_sfx_cancel(audio_sfx_handle_t sfx_handle)
{
	uint8_t channel_idx = _audio_get_channel_from_handle(sfx_handle);

	if (g_audio_context.active_handles[channel_idx] != sfx_handle)
		return;

	g_audio_context.channels[channel_idx].active = 0;
}

void audio_sfx_cancel_all(void)
{
	for (uint32_t channel_idx = 0; channel_idx < AUDIO_SFX_CHANNEL_COUNT; ++channel_idx)
	{
		if ((g_audio_context.active_handles[channel_idx] >> 31) == 0)
		{
			g_audio_context.channels[channel_idx].active = 0;
			g_audio_context.active_handles[channel_idx] = 0;
		}
	}
}

void audio_sfx_set_listener_position(vector3_t position)
{
	g_audio_context.listener_position = position;
	mathVector3MakeFromElements(&g_audio_context.listener_forwards, fixed16_zero, fixed16_zero, fixed16_one);
}
