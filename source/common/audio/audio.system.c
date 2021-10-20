
#include "audio.internal.h"


//	can sort / prioritize using 0 -> 23 bits of the handle 

//	[0  ->  7]		=> channel_idx
//	[8  -> 23]		=> uid
//	[24 -> 30]		=> priority
//	[ 31 ]			=> sfx / stream 

audio_sfx_handle_t _create_audio_sfx_handle(uint8_t channel_idx, uint8_t priority)
{
	audio_sfx_handle_t handle = 0;
	handle |= (++g_audio_context.next_handle_id & 0xffff);
	handle |= (priority & 0x7f) << 16;
	handle |= channel_idx << 24;
	return handle;
}

audio_stream_handle_t _create_audio_stream_handle(uint8_t channel_idx, uint8_t priority)
{
	audio_sfx_handle_t handle = 0;
	handle |= (++g_audio_context.next_handle_id & 0xffff);
	handle |= (priority & 0x7f) << 16;
	handle |= channel_idx << 24;
	handle |= 1 << 31;
	return handle;
}

int8_t _audio_find_free_channel(uint8_t priority)		
{
	int8_t channel_idx = 0;

	//	check for unused channels first
	for (; channel_idx < AUDIO_SFX_CHANNEL_COUNT; ++channel_idx)
	{
		if (!g_audio_context.channels[channel_idx].active)
			break;
	}

	if (channel_idx == AUDIO_SFX_CHANNEL_COUNT)
	{
		audio_handle_t handle = ~0;
		channel_idx = 0;
		for (; channel_idx < AUDIO_SFX_CHANNEL_COUNT; ++channel_idx)
		{
			if (((g_audio_context.active_handles[channel_idx] >> 16) & 0x0f) <= priority)
			{
				if (handle == ~0)
				{
					handle = g_audio_context.active_handles[channel_idx];
					break;
				}
			}
		}
		for (; channel_idx < AUDIO_SFX_CHANNEL_COUNT; ++channel_idx)
		{
			if ((g_audio_context.active_handles[channel_idx] & 0x00ffffff) < (handle & 0x00ffffff))
				handle = g_audio_context.active_handles[channel_idx];
		}

		if (handle != ~0)
		{
			//	if sfx handle then cancel sfx
			if (handle >> 31 == 0)
			{
				audio_sfx_cancel(handle);
			}

			channel_idx = _audio_get_channel_from_handle(handle);
		}
		else
		{
			channel_idx = -1;
		}
	}

	return channel_idx;
}

