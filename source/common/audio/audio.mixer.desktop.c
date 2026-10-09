
#include "audio.internal.h"

#include "common/debug/debug.h"
#include "common/memory.h"
#include "common/utils/bitstream.h"
#include "common/math/trigonometry.h"
#include "common/math/easing.h"

#include <SDL.h>
#include <SDL_audio.h>


const int32_t SAMPLE_RATE = 16384;
const int32_t AUDIO_BUFFER_LEN = 1638;// 16384;	// this can be alot smaller  2 frames worth of audio at 20 fps   
//	audio buffer size is going to result in 


audio_context_t g_audio_context;

SDL_mutex* g_audio_mutex;
int32_t g_read_position;		//	this is only needed on windows 


//	need update cycle to run somewhat in sync with audio ... 
//		how do we bring it back into sync ... this is just a windows problem ... as gba will run in sync with VBlanks. .. 


void flip_audio_buffers()
{
	if (g_audio_context.mixer_write_buffer_full)
	{
		int8_t* tmp = g_audio_context.mixer_left_read_buffer;
		g_audio_context.mixer_left_read_buffer = g_audio_context.mixer_left_write_buffer;
		g_audio_context.mixer_left_write_buffer = tmp;

		tmp = g_audio_context.mixer_right_read_buffer;
		g_audio_context.mixer_right_read_buffer = g_audio_context.mixer_right_write_buffer;
		g_audio_context.mixer_right_write_buffer = tmp;

		g_audio_context.mixer_write_buffer_full = 0;
	}
	else
	{
		debug_printf(DEBUG_LOG_WARN, "audio:flip_audio_buffers, buffers aren't full and ready to be flipped");
	}
}

static void sdl_audio_callback(void* user_data, Uint8* raw_buffer, int bytes)			//	use this to set the pace for the update cycle ??? 
{
	//	for windows this is an audio thread ? do we need thread protection ... 

	Sint16* buffer = (Sint16*)raw_buffer;
	int32_t length = bytes >> 2;	//	this shift takes into account, 8 -> 16 bits and stereo output 

	if (SDL_LockMutex(g_audio_mutex) == 0)
	{
		//	needs to already be split for stereo
		int8_t* mixer_left_output_buffer = g_audio_context.mixer_left_read_buffer + g_read_position;
		int8_t* mixer_right_output_buffer = g_audio_context.mixer_right_read_buffer + g_read_position;

		while (length > 0)
		{
			//	converting int8_t samples in g_samples to SInt16 for SDL playback
			int16_t sample_left = *mixer_left_output_buffer++;
			int16_t sample_right = *mixer_right_output_buffer++;

			sample_left <<= 8;	//	increase scale to take int8 -> int16 
			sample_right <<= 8;	//	increase scale to take int8 -> int16 

			*buffer++ = sample_left;		//	left
			*buffer++ = sample_right;		//	right 

			length--;

			++g_read_position;
			if (g_read_position >= g_audio_context.mixer_buffer_size)
			{
				g_read_position = 0;
				flip_audio_buffers();		//	should we assert if we aren't actually ready to flip 
				mixer_left_output_buffer = g_audio_context.mixer_left_read_buffer;
				mixer_right_output_buffer = g_audio_context.mixer_right_read_buffer;
			}
		}

		SDL_UnlockMutex(g_audio_mutex);
	}
	else
	{
		debug_assert(0, "audio:sdl_audio_callback, SDL_LockMutex failed");
	}
}

void _apply_3d_effect(audio_channel_ptr channel, int16_t* left, int16_t* right)
{

	// ----- 3D EFX ------


	//	angle between listener and emitter 

	vector2_t listener_forwards = { g_audio_context.listener_forwards.x, g_audio_context.listener_forwards.z };

	vector3_t tmp;
	mathVector3Substract(&tmp, &g_audio_context.listener_position, &channel->position);

	vector2_t listener_to_emitter = { tmp.x, tmp.z };


	fixed16_t dot = mathVector2DotProduct(&listener_forwards, &listener_to_emitter);
	fixed16_t det = fixed16_mul(listener_forwards.x, listener_to_emitter.y) - fixed16_mul(listener_forwards.y, listener_to_emitter.x);

	fixed16_t angle = fixed16_arctangent2(det, dot);

	angle = fixed16_mul(angle, F16(57.2958));	//	angle will be in radians

	angle = fixed16_to_int(angle);

	/*
	* 2D case
	dot = x1*x2 + y1*y2      # dot product between [x1, y1] and [x2, y2]
	det = x1*y2 - y1*x2      # determinant
	angle = atan2(det, dot)  # atan2(y, x) or atan2(sin, cos)
	*/

	/*
	* Plane embedded in 3D
	dot = x1*x2 + y1*y2 + z1*z2
	det = x1*y2*zn + x2*yn*z1 + xn*y1*z2 - z1*y2*xn - z2*yn*x1 - zn*y1*x2
	angle = atan2(det, dot)
	*/

	if (angle < 0)
		angle += 360;


	if (angle < 90)
	{
		*left = 255 - (int16_t)(255.0f * ((float)angle / 89.0f));
	}
	else if (angle < 180)
	{
		*left = (int16_t)(255.0f * ((float)(angle - 90) / 89.0f));
	}
	else if (angle < 270)
	{
		*right = 255 - (int16_t)(255.0f * ((float)(angle - 180) / 89.0f));
	}
	else
	{
		*right = (int16_t)(255.0f * ((float)(angle - 270) / 89.0f));
	}


	fixed16_t distance = mathVector3Distance(&channel->position, &g_audio_context.listener_position);
	fixed16_t distance_attenuation = fixed16_one - fixed16_div(distance, F16(4));

	if (distance_attenuation < fixed16_zero)
		distance_attenuation = fixed16_zero;

	fixed16_t left_f = fixed16_mul(distance_attenuation, fixed16_from_int(*left));
	fixed16_t right_f = fixed16_mul(distance_attenuation, fixed16_from_int(*right));

	*left = fixed16_to_int(left_f);
	*right = fixed16_to_int(right_f);

	//		this would be a linear falloff 
	// 5 meter falloff 
	//    1 / 5 
	//	  d / 5 
	//	  1.0 - (d / 5) 
	//	     d=0 -> 1
	//		 d=2.5 -> 0.5
	//		 d=5 -> 0 
	//		 d=10 -> -1
	// 
	//		this would be quadratic falloff 
	//	  1 / 25
	//	  d^2 / 25
	//	  1.0 - (d^2 / 25)
	//		d^2=0 -> 1
	//		d^2=6.25 -> 0.25 (d=2.5)
	//		d^2=25 -> 0 (d=5)
	// 
	// 
	// 	   could run the distance falloff through an easing function to get sine_in_out 
	// 
	// 
	// 
	//	run this through a falloff function 

	//	sample is already at full volume, 
	//	volume modifier would reduce the sample amplitude 

	//	do we do this in fixed point math ... 


	//	apply master volume !!!			//	channel->volume 
	//		apply to left and right 


	// ----- 3D EFX ------

}

void fill_audio_buffer()	
{
	//	how much should we consume from the staged chunks ?? 
	//		it can only be exact if we have exact frame rate (too much of an ask, I think) 

	//	consider a 'nop' fast path ... 

	/*
		Important lesson: need to have the ability to fill faster than we consume to be more resiliant to frame spikes as it gives us the ability to recover. 
	*/

	if (SDL_LockMutex(g_audio_mutex) == 0)
	{
		if (g_audio_context.mixer_write_buffer_full)
		{
			SDL_UnlockMutex(g_audio_mutex);
			return;
		}

		int16_t sample = 0;

		memory_set32(g_audio_context.mixer_left_write_buffer, g_audio_context.mixer_aligned_buffer_size >> 2, 0x00);	
		memory_set32(g_audio_context.mixer_right_write_buffer, g_audio_context.mixer_aligned_buffer_size >> 2, 0x00);	

		for (uint8_t channel_idx = 0; channel_idx < AUDIO_SFX_CHANNEL_COUNT; ++channel_idx)
		{
			audio_channel_ptr channel = &g_audio_context.channels[channel_idx];
			if (channel->active)
			{
				//	if stream, call callback here
				if (channel->type)
				{			
					int8_t* stream_buffer = g_audio_context.mixer_decode_buffer;
					audiostream_t* stream = channel->stream;

					channel->buffer_size = stream->callback(stream, stream_buffer, g_audio_context.mixer_buffer_size);
					channel->buffer = stream_buffer;	
					channel->read_position = 0;
				}

				int8_t* mixer_left_write_buffer = g_audio_context.mixer_left_write_buffer;
				int8_t* mixer_right_write_buffer = g_audio_context.mixer_right_write_buffer;


				int16_t left = 255;
				int16_t right = 255;

				//	should only be applied if channel is 3d 
				//_apply_3d_effect(channel, &left, &right);

				//	apply channel & master volume 
				left = (left * channel->volume) >> 8;
				right = (right * channel->volume) >> 8;

				//debug_printf(DEBUG_LOG_INFO, "audio:mixer - left=%i, right=%i", left, right);

				int16_t length = g_audio_context.mixer_buffer_size;
				if (length > (channel->buffer_size - channel->read_position))
				{
					length = channel->buffer_size - channel->read_position;

					if (channel->type == 0)
						channel->active = 0;
				}

				int8_t* tmp_left_buffer = mixer_left_write_buffer;
				int8_t* tmp_right_buffer = mixer_right_write_buffer;

				while (length > 0)		//	~819 samples per frame !!! 
				{
					debug_assert(channel->read_position < channel->buffer_size, "channel buffer overflow");

					sample = channel->buffer[channel->read_position];
					++channel->read_position;		//	should buffer_size 

					debug_assert(mixer_left_write_buffer < g_audio_context.mixer_left_write_buffer + g_audio_context.mixer_buffer_size, "mixer buffer overflow");
					debug_assert(mixer_right_write_buffer < g_audio_context.mixer_right_write_buffer + g_audio_context.mixer_buffer_size, "mixer buffer overflow");

					*mixer_left_write_buffer++ += (int8_t)((sample * left) >> 8);		//	output stereo here, 
					*mixer_right_write_buffer++ += (int8_t)((sample * right) >> 8);		//	output stereo here, 

					length--;
				}
			}
		}

		g_audio_context.mixer_write_buffer_full = 1;

		SDL_UnlockMutex(g_audio_mutex);
	}
	else
	{
		debug_assert(0, "audio:sdl_audio_callback, SDL_LockMutex failed");
	}
}


//	SYS

void audio_initialize(void)
{
	g_audio_context.mixer_buffer_size = AUDIO_BUFFER_LEN;
	g_audio_context.mixer_aligned_buffer_size = 1640;

	int8_t* mixer_buffer = (int8_t*)memory_allocate(g_audio_context.mixer_aligned_buffer_size * 5, MEMORY_IWRAM);	//	~ 2 to 4 KB 
	memory_set32(mixer_buffer, (g_audio_context.mixer_aligned_buffer_size * 5) >> 2, 0);

	g_audio_context.mixer_left_write_buffer = mixer_buffer + (g_audio_context.mixer_aligned_buffer_size * 0);
	g_audio_context.mixer_left_read_buffer = mixer_buffer + (g_audio_context.mixer_aligned_buffer_size * 1);

	g_audio_context.mixer_right_write_buffer = mixer_buffer + (g_audio_context.mixer_aligned_buffer_size * 2);
	g_audio_context.mixer_right_read_buffer = mixer_buffer + (g_audio_context.mixer_aligned_buffer_size * 3);

	g_audio_context.mixer_decode_buffer = mixer_buffer + (g_audio_context.mixer_aligned_buffer_size * 4);
	memory_set32(g_audio_context.mixer_decode_buffer, g_audio_context.mixer_aligned_buffer_size >> 2, 0x0C0C0C0C);

	g_audio_context.mixer_write_buffer_full = 1;


	g_read_position = 0;

	g_audio_mutex = SDL_CreateMutex();
	debug_assert(g_audio_mutex != NULL, "audio::initialize, SDL_CreateMutex failed");


	if (SDL_Init(SDL_INIT_AUDIO) != 0)
		debug_printf(DEBUG_LOG_ERROR, "Failed to initialize SDL: %s", SDL_GetError());	//	consider this fatal

	SDL_AudioSpec want;
	want.freq = SAMPLE_RATE; // number of samples per second
	want.format = AUDIO_S16; // sample type (here: signed short i.e. 16 bit)
	want.channels = 2; // only one channel
	want.samples = 512; // buffer-size
	want.callback = sdl_audio_callback; // function SDL calls periodically to refill the buffer

	SDL_AudioSpec have;
	if (SDL_OpenAudio(&want, &have) != 0)
		debug_printf(DEBUG_LOG_ERROR, "Failed to open audio: %s", SDL_GetError());		//	consider this fatal

	if (want.format != have.format)
		debug_printf(DEBUG_LOG_ERROR, "Failed to get the desired AudioSpec");		//	consider this fatal

	SDL_PauseAudio(0); // start playing sound

	debug_printf(DEBUG_LOG_INFO, "sound::initialized");
}

void audio_shutdown(void)
{
	SDL_CloseAudio();

	SDL_DestroyMutex(g_audio_mutex);
}

void audio_update(void)
{
	_audio_stream_logic();

	fill_audio_buffer();
}

