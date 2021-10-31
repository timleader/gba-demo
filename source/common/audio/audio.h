
#ifndef AUDIO_H
#define AUDIO_H

#include "common/types.h"
#include "common/math/vector.h"
#include "common/utils/ringbuffer.h"
#include "common/utils/timer.h"

/*
	Can eventually look at using a sound wave generator to really take advantage of the 
	GBA hardware and reduce space required on the cart. 
	
	http://www.belogic.com/gba/index.php

	https://github.com/gbdev/awesome-gbadev
	https://github.com/stuij/apex-audio-system

*/

#define AUDIO_FORMAT_PCM	0

#define AUDIO_SAMPLE_RATE_FULL	16384
#define AUDIO_SAMPLE_RATE_HALF	8162

typedef struct audioclip_s		
{
	uint8_t channels;
	uint8_t format;
	uint16_t sample_rate;	

	uint32_t chunk_count;
	uint32_t sample_count;

	uint8_t data[0];

} audioclip_t;

typedef audioclip_t* audioclip_ptr;



typedef uint32_t audio_handle_t;



//	SYS API:

void audio_initialize(void);	//	allocates buffers + sets up interrupts / DMA / Timers

void audio_shutdown(void);

void audio_update(void);	//	gives the system a chance to fill audio buffers




//	SFX API:

typedef audio_handle_t audio_sfx_handle_t;


audio_sfx_handle_t audio_sfx_play(audioclip_ptr clip);	//	maybe pass a property for 3D position, fall off curve 

audio_sfx_handle_t audio_sfx_play_ex(audioclip_ptr clip, uint8_t priority);			//	concept of default_priority 

audio_sfx_handle_t audio_sfx_play_3d(audioclip_ptr clip, vector3_t position, uint8_t priority);

void audio_sfx_panning(audio_sfx_handle_t sfx_handle, int8_t panning);

void audio_sfx_volume(audio_sfx_handle_t sfx_handle, uint8_t volume);

void audio_sfx_3d_position(audio_sfx_handle_t sfx_handle, vector3_t position);

void audio_sfx_cancel(audio_sfx_handle_t sfx_handle);

void audio_sfx_cancel_all(void);

void audio_sfx_set_listener_position(vector3_t position);




//	STREAMING API: 

#define AUDIO_STREAM_AMBIENT	0
#define AUDIO_STREAM_MUSIC		1

typedef enum audiostream_state_e
{
	IDLE,
	FADE_IN,
	PLAYING,
	FADE_OUT

} audiostream_state_t;
	
typedef int32_t(*audio_fill_stream_buffer_func)(void_ptr context, int8_t* dest, int32_t length);	//	pass context 

typedef struct audiostream_s
{
	audiostream_state_t state;

	uint8_t channel_idx;
	uint8_t loop;
	uint8_t reserved[2];

	audioclip_ptr audioclip;
	int32_t read_position;
	int32_t chunk_idx;

	game_timer_t timer;
	int16_t volume_source;
	int16_t volume_target;

	audio_fill_stream_buffer_func callback;		

} audiostream_t;			

typedef audio_handle_t audio_stream_handle_t;



audio_stream_handle_t audio_stream_open(audioclip_ptr audioclip, uint8_t layer);

audio_stream_handle_t audio_stream_open_custom(audio_fill_stream_buffer_func stream_fill_func, uint8_t layer);

void audio_stream_close(audio_stream_handle_t handle);

void audio_stream_volume(audio_stream_handle_t stream_handle, uint8_t volume);


#endif
