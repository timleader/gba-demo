
#include "audio.internal.h"

#include "common/memory.h"
#include "common/video/smacker.h"
#include "common/debug/debug.h"
#include "common/math/easing.h"


int32_t _audio_stream_fill_pcm_16khz(audiostream_t* stream, int8_t* dest, int32_t length)
{
	int8_t* samples = (int8_t*)stream->audioclip->data;
	uint32_t sample_count = stream->audioclip->sample_count;

	if (stream->read_position + length > sample_count)
		length = sample_count - stream->read_position;

	if (!length)
		return length;

	//debug_assert(((uint32_t)dest & 0x03) == 0, "length has to be a multiple of 2 for the below loop to work");
	//debug_assert(((uint32_t)&samples[stream->read_pos] & 0x03) == 0, "length has to be a multiple of 2 for the below loop to work");

	//memory_dma_copy32(dest, &samples[stream->read_pos], length);		
	memory_copy(dest, &samples[stream->read_position], length);
	stream->read_position += length;

	if (stream->loop)
	{
		if (stream->read_position >= sample_count)
			stream->read_position = 0;
	}

	return length;
}

int32_t _audio_stream_fill_pcm_8khz(audiostream_t* stream, int8_t* dest, int32_t length)	//	~75,000 cycles
{
	int8_t* samples = (int8_t*)stream->audioclip->data;
	uint32_t sample_count = stream->audioclip->sample_count << 1;

	if (stream->read_position + length > sample_count)
		length = sample_count - stream->read_position;

	if (!length)
		return length;

	debug_assert((length & 0x01) == 0, "length has to be a multiple of 2 for the below loop to work");

	int32_t cycles = length;
	int8_t* src = &samples[stream->read_position >> 1];
	while (cycles > 0)
	{
		int8_t sample0 = *src++;
		int8_t sample1 = *src;

		*dest++ = sample0;
		*dest++ = sample0 + ((sample1 - sample0) >> 1);

		cycles -= 2;
	}
	stream->read_position += length;

	debug_assert(dest <= g_audio_context.mixer_decode_buffer + g_audio_context.mixer_buffer_size, "stream_buffer overflow");

	if (stream->loop)
	{
		if (stream->read_position >= sample_count)
			stream->read_position = 0;
	}

	return length;
}


audio_stream_handle_t _audio_stream_open_ex(audioclip_ptr audioclip, audio_fill_stream_buffer_func stream_fill_func, uint8_t layer, uint8_t volume)	//	??? 
{
	int8_t channel_idx = _audio_find_free_channel(31);

	if (channel_idx < 0)
		return 0;

	audio_stream_handle_t handle = _create_audio_stream_handle(channel_idx, 31);

	g_audio_context.active_handles[channel_idx] = handle;


	audiostream_t* primary_stream = NULL;	
	audiostream_t* secondary_stream = NULL;
	switch (layer)
	{
	case AUDIO_STREAM_AMBIENT:
		secondary_stream = &g_audio_context.ambient_streams[g_audio_context.ambient_current_id];
		g_audio_context.ambient_current_id ^= 1;
		primary_stream = &g_audio_context.ambient_streams[g_audio_context.ambient_current_id];
		break;
	case AUDIO_STREAM_MUSIC:
		secondary_stream = &g_audio_context.music_streams[g_audio_context.music_current_id];
		g_audio_context.music_current_id ^= 1;
		primary_stream = &g_audio_context.music_streams[g_audio_context.music_current_id];
		break;
	default: 
		debug_assert(0, "audio::stream_open_ex layer is not supported");
		break;
	}

	//	--- STREAM LOGIC

	primary_stream->state = FADE_IN;
	primary_stream->timer = timer_start(60, TIMER_MODE_ONCE);
	primary_stream->volume_source = 0;
	primary_stream->volume_target = 255;
	primary_stream->channel_idx = channel_idx;

	primary_stream->read_position = 0;
	primary_stream->chunk_idx = 0;
	primary_stream->audioclip = audioclip;
	primary_stream->callback = stream_fill_func;
	primary_stream->loop = TRUE;

	if (secondary_stream->state != IDLE)
	{
		audio_channel_ptr channel = &g_audio_context.channels[secondary_stream->channel_idx];

		secondary_stream->state = FADE_OUT;
		secondary_stream->timer = timer_start(60, TIMER_MODE_ONCE);
		secondary_stream->volume_source = channel->volume;
		secondary_stream->volume_target = 0;
	}

	//	--- CHANNEL _LOGIC
	audio_channel_ptr channel = &g_audio_context.channels[channel_idx];

	channel->active = 1;
	channel->read_position = 0;
	channel->type = 1;
	channel->volume = volume;
	channel->stream = primary_stream;

	return handle;
}

audio_stream_handle_t audio_stream_open(audioclip_ptr audioclip, uint8_t layer)
{
	audio_fill_stream_buffer_func callback = NULL;

	switch (audioclip->format)
	{
		case AUDIO_FORMAT_PCM:
		{
			if (audioclip->sample_rate == AUDIO_SAMPLE_RATE_FULL)
				callback = (audio_fill_stream_buffer_func)_audio_stream_fill_pcm_16khz;
			else if (audioclip->sample_rate == AUDIO_SAMPLE_RATE_HALF)
				callback = (audio_fill_stream_buffer_func)_audio_stream_fill_pcm_8khz;
			else
				debug_assert(0, "audio:stream_open, audioclip sample_rate isn't supported");

			break;
		}
		default:
		{
			debug_assert(0, "audio:stream_open, audioclip format isn't supported");
			break;
		}
	}

	return _audio_stream_open_ex(audioclip, callback, layer, 0);
}

audio_stream_handle_t audio_stream_open_custom(audio_fill_stream_buffer_func stream_fill_func, uint8_t layer)
{
	return _audio_stream_open_ex(NULL, stream_fill_func, layer, 0);
}

void audio_stream_close(audio_stream_handle_t stream_handle)
{
	int8_t channel_idx = _audio_get_channel_from_handle(stream_handle);

	if (g_audio_context.active_handles[channel_idx] != stream_handle)
		return;

	//	add support for immediate stop / fade out 

	audio_channel_ptr channel = &g_audio_context.channels[channel_idx];
	audiostream_t* stream = channel->stream;	
	
	if (stream->state == FADE_IN ||
		stream->state == PLAYING)
	{
		stream->state = FADE_OUT;
		stream->timer = timer_start(60, TIMER_MODE_ONCE);
		stream->volume_source = channel->volume;
		stream->volume_target = 0;
	}
}

void audio_stream_volume(audio_stream_handle_t stream_handle, uint8_t volume)
{
	int8_t channel_idx = _audio_get_channel_from_handle(stream_handle);

	if (g_audio_context.active_handles[channel_idx] != stream_handle)
		return;

	audio_channel_ptr channel = &g_audio_context.channels[channel_idx];
	audiostream_t* stream = channel->stream;

	if (stream->state == PLAYING)
	{
		g_audio_context.channels[channel_idx].volume = volume;
	}
	else
	{
		stream->volume_target = volume;
	}
}


void _audio_stream_logic(void)
{
	const audiostream_t* streams[4] = 
	{ 
		&g_audio_context.music_streams[0], 
		&g_audio_context.music_streams[1],
		&g_audio_context.ambient_streams[0],
		&g_audio_context.ambient_streams[1]
	};

	for (int32_t idx = 0; idx < 4; ++idx)
	{
		audiostream_t* stream = streams[idx];
		audio_channel_t* channel = &g_audio_context.channels[stream->channel_idx];

		switch (stream->state)
		{
			case FADE_IN:
			{
				fixed16_t t = timer_progress(stream->timer);
				t = math_easeoutquad(t);
				fixed16_t vf = fixed16_lerp(fixed16_from_int(stream->volume_source), fixed16_from_int(stream->volume_target), t);
				int16_t v = fixed16_to_int(vf);
				channel->volume = v;

				if (timer_expired(stream->timer))		//	update loop is too fast on gba 
					stream->state = PLAYING;

				break;
			}
			case FADE_OUT:
			{
				fixed16_t t = timer_progress(stream->timer);
				t = math_easeoutquad(t);
				fixed16_t vf = fixed16_lerp(fixed16_from_int(stream->volume_source), fixed16_from_int(stream->volume_target), t);
				int16_t v = fixed16_to_int(vf);
				channel->volume = v;

				if (timer_expired(stream->timer))
				{
					stream->state = IDLE;
					channel->active = 0;
				}

				break;
			}
			case IDLE:
			case PLAYING:
			{
				//	do nothing 
				break;
			}
		}
	}
}



