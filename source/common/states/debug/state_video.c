
#include "states.h"

#include "common/math/matrix.h"

#include "common/graphics/graphics.h"
#include "common/graphics/overlay.h"
#include "common/graphics/image.h"
#include "common/graphics/camera.h"
#include "common/graphics/text.h"
#include "common/collision/collision.h"
#include "common/resources/resources.h"
#include "common/input/input.h"
#include "common/utils/profiler.h"
#include "common/memory.h"

#include "common/video/smacker.h"

#include "games/7days/sequence.h"
#include "package_7days.h"


//-----------------------------------------------------------------------------
typedef enum video_ui_layout_s
{
	CENTER			= 0,
	CENTER_TOP		= 1,
	BOTTOM_LEFT		= 2,

} video_ui_layout_t;


//-----------------------------------------------------------------------------
typedef enum video_sequence_event_type_s
{
	VIDEO_WAIT	= 0,
	VIDEO_TEXT	= 1,
	VIDEO_IMAGE = 2,

} video_sequence_event_type_t;


//-----------------------------------------------------------------------------
typedef struct st_video_context_s
{
	sequence_player_ptr video_sequence_player;
	uint8_t video_panel_id;

	smacker_handle_ptr smk_handle;
	audio_stream_handle_t audio_handle;

	uint8_t* frame_buffer;

	game_timer_t video_sequence_timer;
	int8_t video_sequence_timer_completion_flag;

	int8_t video_play_mode;
	int8_t video_progress_status;

} st_video_context_t;

typedef st_video_context_t* st_video_context_ptr;

//	current context doesn't need to be passed as parameter could just be gobally accessible 


//-----------------------------------------------------------------------------
void st_video_ui_set_layout(st_video_context_ptr context, video_ui_layout_t layout)
{
	point2_t panel_pos = { 0, 0 };
	switch (layout)
	{
	case CENTER:
		panel_pos.x = 24;
		panel_pos.y = 26;
		break;
	case CENTER_TOP:
		panel_pos.x = 24;
		panel_pos.y = 26;
		break;
	case BOTTOM_LEFT:
		panel_pos.x = 8;
		panel_pos.y = 136;
		break;
	}
	overlay_set_position(context->video_panel_id, panel_pos);
}


//-----------------------------------------------------------------------------
sequence_completion_mode_t st_video_sequence_event_handler_wait(sequence_player_ptr player, sequence_event_t event)
{
	st_video_context_ptr context = (st_video_context_ptr)player->context;

	uint32_t waitFrameCount = SEQUENCE_EVENT_EXTRA(event);

	context->video_sequence_timer = timer_start((uint16_t)waitFrameCount, TIMER_MODE_ONCE);
	context->video_sequence_timer_completion_flag = 0;

	player->completion_flag = &context->video_sequence_timer_completion_flag;

	return SEQUENCE_COMPLETION_WAIT_ON_FLAG;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t st_video_sequence_event_handler_text(sequence_player_ptr player, sequence_event_t event)
{
	st_video_context_ptr context = (st_video_context_ptr)player->context;

	uint32_t extra_data = SEQUENCE_EVENT_EXTRA(event);

	uint16_t string_id = extra_data & 0x0000FFFF;
	uint8_t duration = (extra_data >> 16) & 0x000000FF;
	uint8_t layout = (extra_data >> 24) & 0x0000000F;

	overlay_clear(context->video_panel_id, 0);

	st_video_ui_set_layout(context, layout);

	const char* text = stringstore_get(string_id);

	char output_text[128];
	string_wrap(text, output_text, 6 * 4);

	point2_t text_pos = { 0, 0 };
	overlay_draw_string(context->video_panel_id, output_text, 2, text_pos);

	//	reset_panel 

	context->video_sequence_timer = timer_start(duration, TIMER_MODE_ONCE);
	context->video_sequence_timer_completion_flag = 0;

	player->completion_flag = &context->video_sequence_timer_completion_flag;

	return SEQUENCE_COMPLETION_NEXT_TICK;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t st_video_sequence_event_handler_image(sequence_player_ptr player, sequence_event_t event)
{
	st_video_context_ptr context = (st_video_context_ptr)player->context;

	uint32_t extra_data = SEQUENCE_EVENT_EXTRA(event);

	uint16_t resource_id = extra_data & 0x0000FFFF;
	uint8_t duration = (extra_data >> 16) & 0x000000FF;
	uint8_t layout = (extra_data >> 24) & 0x0000000F;

	overlay_clear(context->video_panel_id, 0);

	st_video_ui_set_layout(context, layout);

	tiledimage_ptr image = resources_find_tiledimage(resource_id);
	palette_ptr pal = resources_find_palette(image->palette_id);

	overlay_write_palette(pal);

	point2_t image_pos = { 0, 0 };
	overlay_draw_tiledimage(context->video_panel_id, image, image_pos);

	//	reset_panel

	context->video_sequence_timer = timer_start(duration, TIMER_MODE_ONCE);

	return SEQUENCE_COMPLETION_NEXT_TICK;
}


//-----------------------------------------------------------------------------
int32_t st_video_fill_buffer(audiostream_t* stream, int8_t* dest, int32_t length)	//	do we need this ?? 
{
	st_video_context_ptr context = (st_video_context_ptr)state_context();

	ringbuffer_ptr buffer = smk_get_audio(context->smk_handle, 0);

	//	should return bytes written... 
	ringbuffer_copy_from(buffer, dest, length);

	return length;
}

//-----------------------------------------------------------------------------
void st_video_update(st_video_context_ptr context, fixed16_t dt)
{
	if (key_hit(KI_B))
	{
		state_pop(0);
		return;
	}

	if (!context->video_play_mode &&
		context->video_progress_status == SMK_LAST)
	{
		state_pop(0);
		return;
	}

	if (key_hit(KI_START))
	{
		state_goto(&st_analysis, 0);
		return;
	}

	if (timer_expired(context->video_sequence_timer))
	{
		context->video_sequence_timer_completion_flag = 1;
	}

	sequence_update(context->video_sequence_player);
}

//-----------------------------------------------------------------------------
void st_video_draw(st_video_context_ptr context, fixed16_t dt)
{
	/*
		Optimizations Pending:
				
			- SMK_TREE_TYPE huff16_tree to be in IWRAM fo 1 cycle access vs current 5 cycle access
			- 16-bit/32-bit wide writes to the framebuffer
			- remove all smk_malloc 
		
		Feature Pending:
				
			- Audio 
			- Page Flipping / Double Buffering 
	*/


	switch (context->video_progress_status)
	{
		case SMK_MORE:
		{
			profiler_sample_handle_t phandle = profiler_begin("video:smk_next");
			context->video_progress_status = smk_next(context->smk_handle, context->frame_buffer, g_graphics_context.width);		
			profiler_end(phandle);
			break;
		}
		case SMK_LAST:
		{
			if (context->video_play_mode)
				context->video_progress_status = smk_first(context->smk_handle, context->frame_buffer, g_graphics_context.width);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
void st_video_enter(st_video_context_ptr context, uint32_t parameter)
{
	uint16_t resource_id = parameter & 0x0000FFFF;

	uint8_t options = (parameter >> 16) & 0x000000FF;
	//	options eg. debug controls (play, pause, etc..) 

	context->video_play_mode = options & 0x01;

	videoclip_ptr video = resource_find_videoclip(resource_id);
	if (video == NULL)
	{
		state_pop(~0);
		return;
	}

	//	cant run twice !! guess cleanup isn't right 

	context->smk_handle = smk_open_memory(video->data, video->size);
	if (context->smk_handle == NULL)
	{
		state_pop(~0);
		return;
	}

	unsigned long w, h;

	unsigned char   a_trackmask, a_channels[7], a_depth[7];
	unsigned long   a_rate[7];

	/* get some info about the file */
	smk_info_video(context->smk_handle, &w, &h, NULL);
	smk_info_audio(context->smk_handle, &a_trackmask, a_channels, a_depth, a_rate);

	smk_enable_all(context->smk_handle, SMK_VIDEO_TRACK);

	if (a_channels[0])
	{
		smk_enable_audio(context->smk_handle, 0, SMK_AUDIO_TRACK_0);
	}

	//	push sounds straight to sound system buffers 

	debug_assert(w <= 240, "st_video::enter video width > 240");
	debug_assert(h <= 160, "st_video::enter video height > 160");

	graphics_set_resolution(w, h);

	//graphics_set_vsync(3);	//	set this based on source video .. 

	//	first audio grab is about a second worth of audio,   19 frames, and video is 20fps, so yeah 

	context->frame_buffer = g_graphics_context.frame_buffer;
	context->video_progress_status = smk_first(context->smk_handle, context->frame_buffer, g_graphics_context.width);		//	this will happen on the same frame as the first smk_next 

	if (a_channels[0])
	{
		audio_stream_open_custom(st_video_fill_buffer, 0);
	}

	const uint16_t* pal = smk_get_palette(context->smk_handle);		//	work with palette_ptr
	memory_copy16(g_graphics_context.palette, pal, 256);
	g_graphics_context.dirty_flags |= 0x01;


	graphics_pageflip();


	/*
		create video sequencer
	*/
	context->video_sequence_player = (sequence_player_ptr)memory_allocate(sizeof(sequence_player_t), MEMORY_EWRAM);

	context->video_sequence_player->state = PLAYER_STATE_IDLE;
	context->video_sequence_player->event_handler_count = 3;
	context->video_sequence_player->event_handlers = (sequence_event_handler_t*)memory_allocate(sizeof(sequence_event_handler_t*) * 3, MEMORY_EWRAM);
	context->video_sequence_player->event_handlers[VIDEO_WAIT] = st_video_sequence_event_handler_wait;
	context->video_sequence_player->event_handlers[VIDEO_TEXT] = st_video_sequence_event_handler_text;
	context->video_sequence_player->event_handlers[VIDEO_IMAGE] = st_video_sequence_event_handler_image;

	context->video_sequence_player->persistent = (sequence_player_state_persistent_t*)memory_allocate(sizeof(sequence_player_state_persistent_t), MEMORY_EWRAM);
	context->video_sequence_player->persistent->current_event = -1;
	context->video_sequence_player->persistent->scheduled_event = -1;

	context->video_sequence_player->context = context;

	if (video->sequeunce_collection_id >= 0) 
	{
		sequence_store_ptr store = resources_find_sequencestore(video->sequeunce_collection_id);
		context->video_sequence_player->store = store;
		sequence_schedule(context->video_sequence_player, 0);
	}

	/*
		load ui palette
	*/
	palette_ptr palette = resources_find_palette(RES__PAL_UI_DEFAULT);
	overlay_write_palette(palette);

	/*
		create terminal ui
	*/
	context->video_panel_id = overlay_create_panel(6, 10);

	point2_t panel_position = { 24, 26 };
	overlay_set_position(context->video_panel_id, panel_position);

	overlay_clear(context->video_panel_id, 0);
}

//-----------------------------------------------------------------------------
void st_video_exit(st_video_context_ptr context)
{
	audio_stream_close(0);

	smk_close(context->smk_handle);

	if (context->video_sequence_player != NULL)
	{
		if (context->video_sequence_player->event_handlers != NULL)
		{
			memory_free(context->video_sequence_player->event_handlers);
		}
		if (context->video_sequence_player->persistent != NULL)
		{
			memory_free(context->video_sequence_player->persistent);
		}
		memory_free(context->video_sequence_player);
		context->video_sequence_player = NULL;
	}

	overlay_destroy_panel(context->video_panel_id);

	//	need clean up of smacker_handle_ptr !!! 
}

//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_video =
{
	(state_enter_func_ptr)st_video_enter,
	(state_exit_func_ptr)st_video_exit,

	NULL,
	NULL,

	(state_update_func_ptr)st_video_update,
	(state_draw_func_ptr)st_video_draw,

	"st_video",
	sizeof(st_video_context_t)

};
