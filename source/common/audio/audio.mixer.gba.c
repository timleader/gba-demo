
#include "audio.internal.h"

#include "common/debug/debug.h"
#include "common/memory.h"
#include "common/utils/bitstream.h"
#include "common/math/trigonometry.h"
#include "common/utils/profiler.h"

#include "common/platform/gba/gba.h"


//	eventually convert this to asm 


#define REG_SGCNT0_H   *(vuint16_t*)0x4000082		
#define REG_SOUNDCNT_H *(vuint16_t*)0x4000082		//Direct sound control

#define REG_SGCNT1     *(vuint16_t*)0x4000084		
#define REG_SOUNDCNT_X *(vuint16_t*)0x4000084	    //Extended sound control

#define REG_FIFO_A_ADDR	(0x40000A0)	//!< DSound A FIFO
#define REG_FIFO_B_ADDR	(0x40000A4)	//!< DSound B FIFO

#define REG_DMA1SAD     *(uint32_t*)0x40000BC	//DMA1 Source Address
#define REG_DMA1SAD_L   *(uint16_t*)0x40000BC	//DMA1 Source Address Low Value
#define REG_DMA1SAD_H   *(uint16_t*)0x40000BE	//DMA1 Source Address High Value
#define REG_DMA1DAD     *(uint32_t*)0x40000C0	//DMA1 Desination Address
#define REG_DMA1DAD_L   *(uint16_t*)0x40000C0	//DMA1 Destination Address Low Value
#define REG_DMA1DAD_H   *(uint16_t*)0x40000C2	//DMA1 Destination Address High Value
#define REG_DMA1CNT     *(uint32_t*)0x40000C4	//DMA1 Control (Amount)
#define REG_DMA1CNT_L   *(uint16_t*)0x40000C4	//DMA1 Control Low Value
#define REG_DMA1CNT_H   *(uint16_t*)0x40000C6	//DMA1 Control High Value

#define REG_DMA2SAD     *(uint32_t*)0x40000C8	//DMA2 Source Address
#define REG_DMA2SAD_L   *(uint16_t*)0x40000BC	//DMA2 Source Address Low Value
#define REG_DMA2SAD_H   *(uint16_t*)0x40000CA	//DMA2 Source Address High Value
#define REG_DMA2DAD     *(uint32_t*)0x40000CC	//DMA2 Desination Address
#define REG_DMA2DAD_L   *(uint16_t*)0x40000CC	//DMA2 Destination Address Low Value
#define REG_DMA2DAD_H   *(uint16_t*)0x40000CE	//DMA2 Destination Address High Value
#define REG_DMA2CNT     *(uint32_t*)0x40000D0	//DMA2 Control (Amount)
#define REG_DMA2CNT_L   *(uint16_t*)0x40000D0	//DMA2 Control Low Value
#define REG_DMA2CNT_H   *(uint16_t*)0x40000D2	//DMA2 Control High Value

#define REG_TM0CNT      *(uint32_t*)0x4000100	//Timer 0
#define REG_TM0CNT_L	*(uint16_t*)0x4000100	//Timer 0 count value
#define REG_TM0CNT_H    *(uint16_t*)0x4000102	//Timer 0 Control

#define REG_TM1CNT      *(uint32_t*)0x4000104	//Timer 1
#define REG_TM1CNT_L	*(uint16_t*)0x4000104	//Timer 1 count value
#define REG_TM1CNT_H    *(uint16_t*)0x4000106	//Timer 1 Control

#define REG_TM0D		*(vuint16_t*)(0x04000000+0x0100)	//!< Timer 0 data


const int32_t SAMPLE_RATE = 16384;				//	smacker is producing what is required for each correctly		// can we do 16,000
const int32_t AUDIO_BUFFER_LEN = 1638;			//	this sample length fucks things up	//	this won't be aligned 

//	819  samples will be demanded each frame ... 
//		smacker produces 820, 820, 820, 820, 816  --> which interestingly when averaged this is one sample off of 819 every frame, it is 819.2, this will eventually lead to audio going out of sync 

audio_context_t g_audio_context;


IWRAM_CODE void audio_timer1_interrupt(void)	
{
	REG_DMA1CNT = 0;
	REG_DMA2CNT = 0;
	REG_TM0CNT_H = 0;

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
		debug_printf(DEBUG_LOG_WARN, "audio interrupt, buffers are not full yet");
	}

	//	Left Channel
	REG_DMA1SAD = (uint32_t)g_audio_context.mixer_left_read_buffer; //dma1 source
	//REG_DMA1DAD = REG_FIFO_A_ADDR; //write to FIFO A address
	REG_DMA1CNT_H = 0xb600; //dma control: DMA active+ start on FIFO+32bit+repeat+increment source&dest

	//	Right Channel
	REG_DMA2SAD = (uint32_t)g_audio_context.mixer_right_read_buffer;	//	just right side blipping, when switching to left buffer no blipping 
	//REG_DMA2DAD = REG_FIFO_B_ADDR;
	REG_DMA2CNT_H = 0xb600;


	//	other systems do this in a vysnc hook instead 
	REG_TM1CNT_L = 0xffff - (g_audio_context.mixer_buffer_size - 8); //0xffff-the number of samples to play	//	CONSIDER CARRY OVER HERE 
	REG_TM1CNT_H = 0xC4; //enable timer1 + irq and cascade from timer 0

	//Formula for playback frequency is: 0xFFFF-round(cpuFreq/playbackFreq)
	REG_TM0CNT_L = 0xFBFF;//	=> 16,384 Hz ---- 0xFBE8; => 16,000 Hz                 //16khz playback freq 0xFFFF - (2^24 / 16384);
	REG_TM0CNT_H = 0x0080; //enable timer0
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
	fixed16_t distance_attenuation = fixed16_one - fixed16_div(distance, F16(4));	//	max_distance should be set somewhere 

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
	// 
	//	run this through a falloff function 

	//	sample is already at full volume, 
	//	volume modifier would reduce the sample amplitude 

	//	do we do this in fixed point math ... 


	//	apply master volume !!!			//	channel->volume 
	//		apply to left and right 


	// ----- 3D EFX ------

}


int8_t* _audio_left_buffer, * _audio_right_buffer;
volatile int16_t _audio_left_volume, _audio_right_volume;
int8_t* _audio_source_buffer;
int16_t _audio_sample_count;

void audio_mix_samples(void);

IWRAM_CODE void fill_audio_buffer()	//	fill ring buffer	//	make this assmebly eventually 
{
	if (g_audio_context.mixer_write_buffer_full)
		return;

	profiler_sample_handle_t profiler_handle;

	profiler_handle = profiler_begin("a:fill_buf:ms");

	memory_set32(g_audio_context.mixer_left_write_buffer, g_audio_context.mixer_aligned_buffer_size >> 2, 0x00);	// clear mixer output buffer
	memory_set32(g_audio_context.mixer_right_write_buffer, g_audio_context.mixer_aligned_buffer_size >> 2, 0x00);	// clear mixer output buffer

	profiler_end(profiler_handle);

	for (uint8_t channel_idx = 0; channel_idx < AUDIO_SFX_CHANNEL_COUNT; ++channel_idx)
	{
		audio_channel_ptr channel = &g_audio_context.channels[channel_idx];
		if (channel->active)
		{

			profiler_handle = profiler_begin("a:fb:ch0");

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

			profiler_end(profiler_handle);

			profiler_handle = profiler_begin("a:fb:ch1");

			//	should only be applied if channel is 3d 
			//_apply_3d_effect(channel, &left, &right);

			//	apply channel & master volume 
			left = (left * channel->volume) >> 8;
			right = (right * channel->volume) >> 8;

			int16_t length = g_audio_context.mixer_buffer_size;
			if (length > (channel->buffer_size - channel->read_position))
			{
				length = channel->buffer_size - channel->read_position;

				if (channel->type == 0)
					channel->active = 0;
			}

			profiler_end(profiler_handle);

			profiler_handle = profiler_begin("a:fb:ch2");

			int8_t* source_buffer = &channel->buffer[channel->read_position];
			channel->read_position += length;


			_audio_left_buffer = mixer_left_write_buffer;
			_audio_right_buffer = mixer_right_write_buffer;
			_audio_left_volume = left;
			_audio_right_volume = right;
			_audio_source_buffer = source_buffer;
			_audio_sample_count = length;

			audio_mix_samples();

			profiler_end(profiler_handle);
		}
	}

	g_audio_context.mixer_write_buffer_full = 1;
}

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

	debug_printf(DEBUG_LOG_DEBUG, "audio::initialize - mixer_left_write_buffer=0x%x", g_audio_context.mixer_left_write_buffer);
	debug_printf(DEBUG_LOG_DEBUG, "audio::initialize - mixer_left_read_buffer=0x%x", g_audio_context.mixer_left_read_buffer);
	debug_printf(DEBUG_LOG_DEBUG, "audio::initialize - mixer_right_write_buffer=0x%x", g_audio_context.mixer_right_write_buffer);
	debug_printf(DEBUG_LOG_DEBUG, "audio::initialize - mixer_right_read_buffer=0x%x", g_audio_context.mixer_right_read_buffer);
	debug_printf(DEBUG_LOG_DEBUG, "audio::initialize - mixer_stream_buffer=0x%x", g_audio_context.mixer_decode_buffer);


	debug_printf(DEBUG_LOG_DEBUG, "g_audio_ring_buffer allocated");

	irqSet(IRQ_TIMER1, audio_timer1_interrupt);
	irqEnable(IRQ_TIMER1);

	//	right channel is outputting , left channel is struggling 
	//	Channel A is shit

	//	here we can set A => Left, B => Right 
	REG_SOUNDCNT_H = 0x9A0C; //	enable DS A&B + fifo reset + use timer0 + max volume to L and R
	REG_SOUNDCNT_X = 0x0080; //	turn sound chip on

	debug_printf(DEBUG_LOG_DEBUG, "DirectSound Enabled");

	//	Left Channel
	REG_DMA1CNT = 0;
	REG_DMA1SAD = (uint32_t)g_audio_context.mixer_left_read_buffer; //dma1 source
	REG_DMA1DAD = REG_FIFO_A_ADDR; //	write to FIFO A address
	REG_DMA1CNT_L = 0x0000;
	REG_DMA1CNT_H = 0xb600; //	dma control: DMA active+ start on FIFO+32bit+repeat+increment source&dest

	//	Right Channel
	REG_DMA2CNT = 0;
	REG_DMA2SAD = (uint32_t)g_audio_context.mixer_right_read_buffer;
	REG_DMA2DAD = REG_FIFO_B_ADDR;
	REG_DMA2CNT_L = 0x0000;
	REG_DMA2CNT_H = 0xb600;

	debug_printf(DEBUG_LOG_DEBUG, "DMA Setup");

	//	-8 gets rid of the noise blit during silence, though causes a different overrun or skip as frontend music not blits while playing ... 
	REG_TM1CNT_L = 0xffff - (g_audio_context.mixer_buffer_size - 8); //	0xffff-the number of samples to play
	REG_TM1CNT_H = 0xC4; //enable timer1 + irq and cascade from timer 0

	//Formula for playback frequency is: 0xFFFF-round(cpuFreq/playbackFreq)

	REG_TM0CNT_L = 0xFBE8; //16khz playback freq
	REG_TM0CNT_H = 0x0080; //enable timer0

	debug_printf(DEBUG_LOG_DEBUG, "Audio Frequency Set");
}

void audio_shutdown(void)
{
}

void audio_update(void)
{
	_audio_stream_logic();

	fill_audio_buffer();
}
