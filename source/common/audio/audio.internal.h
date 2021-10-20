
#ifndef AUDIO_INTERNAL_H
#define AUDIO_INTERNAL_H

#include "audio.h"

#include "common/math/vector.h"
#include "common/utils/timer.h"



#define AUDIO_SFX_CHANNEL_COUNT 6


typedef struct audio_channel_s
{
	int8_t* buffer;

	int32_t buffer_size;

	int32_t read_position;
	int32_t write_position;

	//------------------------

	int8_t active;

	//	attributes we don't really care about 
	int8_t type;	// normal = 0, looping = 1, ring = 2
	uint8_t volume;
	int8_t panning;
	//	etc... 

	audiostream_t* stream;	//	this is ugly have circular references here 

	//	3D
	vector3_t position;

} audio_channel_t;

typedef audio_channel_t* audio_channel_ptr;	//	these are internal right



typedef struct audio_context_s		
{

	//	MIXER
	int8_t* mixer_left_write_buffer;
	int8_t* mixer_right_write_buffer;

	int8_t* mixer_left_read_buffer;
	int8_t* mixer_right_read_buffer;

	int32_t mixer_buffer_size;
	int32_t mixer_aligned_buffer_size;
	int32_t mixer_write_buffer_full;

	uint32_t mixer_page_flip;

	int8_t* mixer_decode_buffer;		


	//	STREAMS { AMBIENT, MUSIC }
	int16_t ambient_current_id;
	int16_t music_current_id;
	audiostream_t ambient_streams[2];			//	2 streams for music, a primary and a secondary, fading out is always secondary (once fade out is complete disable the stream) 
	audiostream_t music_streams[2];


	//	SFX
	int32_t next_handle_id;
	audio_handle_t active_handles[AUDIO_SFX_CHANNEL_COUNT];
	audio_channel_t channels[AUDIO_SFX_CHANNEL_COUNT];		


	//	3D
	vector3_t listener_position;
	vector3_t listener_forwards;	

} audio_context_t;


extern audio_context_t g_audio_context;


#define inline __inline

static inline uint8_t _audio_get_channel_from_handle(audio_handle_t handle) { return (handle >> 24) & 0x07; }

audio_sfx_handle_t _create_audio_sfx_handle(uint8_t channel_idx, uint8_t priority);
audio_stream_handle_t _create_audio_stream_handle(uint8_t channel_idx, uint8_t priority);

int8_t _audio_find_free_channel(uint8_t priority);

void _audio_stream_logic(void);


#endif
