
#include "games/7days/states/states.h"

#include "common/resources/resources.h"
#include "common/graphics/graphics.h"
#include "common/graphics/overlay.h"
#include "common/graphics/image.h"
#include "common/input/input.h"
#include "common/memory.h"
#include "common/string.h"

#include "games/7days/sequence.h"

#include "package_7days.h"

//	handle user input - for later point in the game !!! 

//-----------------------------------------------------------------------------
#define TERMINAL_LINE_WIDTH 20
#define TERMINAL_LINE_COUNT 10

//-----------------------------------------------------------------------------
typedef struct terminal_line_s
{
	char line[TERMINAL_LINE_WIDTH];

} terminal_line_t;

//-----------------------------------------------------------------------------
typedef enum terminal_sequence_event_type_s
{
	TERMINAL_WAIT			= 0,
	TERMINAL_CLEAR			= 1,
	TERMINAL_OUTPUT			= 2,
	TERMINAL_NEWLINE		= 3,
	TERMINAL_COMMAND		= 4,
	TERMINAL_EXIT			= 5,

} terminal_sequence_event_type_t;


//-----------------------------------------------------------------------------
typedef struct st_terminal_context_s
{
	sequence_player_ptr terminal_sequence_player;
	palette_ptr terminal_ui_palette;
	uint8_t terminal_panel_id;
	uint8_t reserved[3];

	terminal_line_t terminal_output[TERMINAL_LINE_COUNT];

	char terminal_working_buffer[256];

	uint16_t terminal_current_line_idx;
	uint16_t reserved2;

	point2_t terminal_cursor_position;
	game_timer_t terminal_cursor_blink_timer;

	game_timer_t terminal_sequence_timer;


	uint8_t terminal_readout_idx;
	uint8_t terminal_cursor_blink_state;
	int8_t terminal_sequence_timer_completion_flag;
	uint8_t terminal_state;

	audio_stream_handle_t stream_handle;

} st_terminal_context_t;

typedef st_terminal_context_t* st_terminal_context_ptr;


//-----------------------------------------------------------------------------
void st_terminal_draw_output(st_terminal_context_ptr context)
{
	overlay_clear(context->terminal_panel_id, 0);

	for (uint8_t idx = 0; idx < TERMINAL_LINE_COUNT; ++idx)
	{
		point2_t t_pos = { 0, idx };

		overlay_draw_string(
			context->terminal_panel_id,
			context->terminal_output[idx].line,
			2,
			t_pos);
	}
}

//-----------------------------------------------------------------------------
void st_terminal_write_character(st_terminal_context_ptr context, char character)
{
	point2_t size = { 8, 8 };
	overlay_draw_color(context->terminal_panel_id, 0, context->terminal_cursor_position, size);

	context->terminal_output[context->terminal_current_line_idx].line[context->terminal_cursor_position.x] = character;
	overlay_draw_character(context->terminal_panel_id, character, 2, context->terminal_cursor_position);

	context->terminal_cursor_position.x++;

	audioclip_ptr clip = resources_find_audioclip(RES__SND_TERMINAL);
	audio_sfx_play_ex(clip, 0);
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t st_terminal_sequence_event_handler_wait(sequence_player_ptr player, sequence_event_t event)
{
	st_terminal_context_ptr context = (st_terminal_context_ptr)player->context;

	uint32_t waitFrameCount = SEQUENCE_EVENT_EXTRA(event);

	context->terminal_sequence_timer = timer_start((uint16_t)waitFrameCount, TIMER_MODE_ONCE);
	player->completion_flag = &context->terminal_sequence_timer_completion_flag;
	context->terminal_sequence_timer_completion_flag = 0;

	return SEQUENCE_COMPLETION_WAIT_ON_FLAG;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t st_terminal_sequence_event_handler_clear(sequence_player_ptr player, sequence_event_t event)
{
	st_terminal_context_ptr context = (st_terminal_context_ptr)player->context;

	overlay_clear(context->terminal_panel_id, 0);
	context->terminal_current_line_idx = 0;

	for (uint8_t idx = 0; idx < TERMINAL_LINE_COUNT; ++idx)
	{
		for (uint8_t j = 0; j < TERMINAL_LINE_WIDTH; ++j)
			context->terminal_output[idx].line[j] = 0;
	}

	const char* command_prompt = ">";	//	perhaps this should be optional ...?
	memory_copy(&context->terminal_output[9], (const void_ptr)command_prompt, string_length(command_prompt));

	context->terminal_cursor_position.x = 1;
	context->terminal_cursor_position.y = 9;

	audioclip_ptr clip = resources_find_audioclip(RES__SND_TERMINAL);
	audio_sfx_play_ex(clip, 0);

	return SEQUENCE_COMPLETION_IMMEDIATE;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t st_terminal_sequence_event_handler_write_output(sequence_player_ptr player, sequence_event_t event)
{
	st_terminal_context_ptr context = (st_terminal_context_ptr)player->context;

	uint16_t string_id = SEQUENCE_EVENT_EXTRA(event) & 0x0000FFFF;

	const char* text = stringstore_get(string_id);
	string_wrap(text, context->terminal_working_buffer, TERMINAL_LINE_WIDTH);

	char* output_buf = context->terminal_working_buffer;

	uint16_t line_count = string_character_count(output_buf, '\n') + 1;

	//	I don't think this is needed as everything gets too overwhelming on screen -- maybe keep this thought just in case s
	int8_t lines_to_shift = (context->terminal_current_line_idx + line_count) - TERMINAL_LINE_COUNT;
	if (lines_to_shift > 0)
	{
		for (uint8_t idx = 0; idx < TERMINAL_LINE_COUNT - lines_to_shift; ++idx)
		{
			//	OPTIMIZATION: an extra layer of indirection here would remove the need for a copy
			memory_copy(&context->terminal_output[idx], &context->terminal_output[idx + lines_to_shift], TERMINAL_LINE_WIDTH);
		}

		context->terminal_current_line_idx -= lines_to_shift;
	}

	const char delimeters[] = { '\n', 0 };
	for (uint8_t idx = 0; idx < line_count; ++idx)
	{
		int32_t newline_idx = string_index_of_any(output_buf, delimeters, 2);
		memory_copy(&context->terminal_output[context->terminal_current_line_idx], output_buf, newline_idx);
		context->terminal_output[context->terminal_current_line_idx].line[newline_idx] = 0;
		output_buf += newline_idx + 1;

		context->terminal_current_line_idx++;
	}

	st_terminal_draw_output(context);

	return SEQUENCE_COMPLETION_IMMEDIATE;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t st_terminal_sequence_event_handler_newline(sequence_player_ptr player, sequence_event_t event)
{
	st_terminal_context_ptr context = (st_terminal_context_ptr)player->context;

	uint16_t line_count = SEQUENCE_EVENT_EXTRA(event) & 0x0000FFFF;

	//	I don't think this is needed as everything gets too overwhelming on screen -- maybe keep this thought just in case s
	int8_t lines_to_shift = (context->terminal_current_line_idx + line_count) - TERMINAL_LINE_COUNT;
	if (lines_to_shift > 0)
	{
		for (uint8_t idx = 0; idx < TERMINAL_LINE_COUNT - lines_to_shift; ++idx)
		{
			//	OPTIMIZATION: an extra layer of indirection here would remove the need for a copy
			memory_copy(&context->terminal_output[idx], &context->terminal_output[idx + lines_to_shift], TERMINAL_LINE_WIDTH);
		}

		context->terminal_current_line_idx -= lines_to_shift;

		st_terminal_draw_output(context);
	}

	context->terminal_current_line_idx += line_count;

	return SEQUENCE_COMPLETION_IMMEDIATE;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t st_terminal_sequence_event_handler_exit(sequence_player_ptr player, sequence_event_t event)
{
	state_pop(0);
	return SEQUENCE_COMPLETION_IMMEDIATE;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t st_terminal_sequence_event_handler_write_command(sequence_player_ptr player, sequence_event_t event)
{
	st_terminal_context_ptr context = (st_terminal_context_ptr)player->context;

	uint16_t string_id = SEQUENCE_EVENT_EXTRA(event) & 0x0000FFFF;
	char* output = context->terminal_working_buffer;

	const char* text = stringstore_get(string_id);
	memory_copy(output, (const void_ptr)text, string_length(text) + 1);

	context->terminal_state = 1;

	return SEQUENCE_COMPLETION_NEXT_TICK;
}

//-----------------------------------------------------------------------------
void st_terminal_enter(st_terminal_context_ptr context, uint32_t parameter)
{
	int16_t sequence_id = parameter & 0x0000FFFF;	//	this should probably be the resource id

	sequence_player_ptr player = (sequence_player_ptr)memory_allocate(sizeof(sequence_player_t), MEMORY_EWRAM);

	player->state = PLAYER_STATE_IDLE;
	player->event_handler_count = 6;
	player->event_handlers = (sequence_event_handler_t*)memory_allocate(sizeof(sequence_event_handler_t*) * 6, MEMORY_EWRAM);
	player->event_handlers[0] = st_terminal_sequence_event_handler_wait;
	player->event_handlers[1] = st_terminal_sequence_event_handler_clear;
	player->event_handlers[2] = st_terminal_sequence_event_handler_write_output;
	player->event_handlers[3] = st_terminal_sequence_event_handler_newline;
	player->event_handlers[4] = st_terminal_sequence_event_handler_write_command;
	player->event_handlers[5] = st_terminal_sequence_event_handler_exit;

	player->persistent = (sequence_player_state_persistent_t*)memory_allocate(sizeof(sequence_player_state_persistent_t), MEMORY_EWRAM);
	player->persistent->current_event = -1;
	player->persistent->scheduled_event = -1;

	player->context = context;
	
	sequence_store_ptr store = resources_find_sequencestore(sequence_id);
	player->store = store;
	sequence_schedule(player, 0);

	context->terminal_sequence_player = player;

	/*
		load background img
	*/
	image_ptr image = resources_find_image(RES__IMG_TERMINAL);
	palette_ptr palette = resources_find_palette(image->palette_id);

	graphics_write_palette(palette);

	graphics_draw_image(image, 0, 0, 0);

	graphics_pageflip();

	graphics_set_vsync(2);	

	/*
		create ui palette
	*/
	context->terminal_ui_palette = palette_new(4, MEMORY_EWRAM);

	context->terminal_ui_palette->color555[0] = RGB555(0, 0, 0);
	context->terminal_ui_palette->color555[1] = RGB555(64, 64, 255);
	context->terminal_ui_palette->color555[2] = RGB555(255, 255, 255);
	context->terminal_ui_palette->color555[3] = RGB555(64, 255, 64);

	overlay_write_palette(context->terminal_ui_palette);

	/*
		create terminal ui
	*/

	context->terminal_panel_id = overlay_create_panel(5, 10);

	point2_t panel_position = { 40, 26 };
	overlay_set_position(context->terminal_panel_id, panel_position);

	overlay_clear(context->terminal_panel_id, 0);

	/*
		create working buffer
	*/
	context->terminal_current_line_idx = 0;

	for (uint8_t idx = 0; idx < TERMINAL_LINE_COUNT; ++idx)
		context->terminal_output[idx].line[0] = 0;

	/*
		command prompt
	*/
	const char* command_prompt = ">";	//	maybe this should be optional ... 
	memory_copy(&context->terminal_output[9], (const void_ptr)command_prompt, string_length(command_prompt));

	context->terminal_cursor_position.x = 1;
	context->terminal_cursor_position.y = 9;


	//audio_sfx_play(resources_find_audioclip(RES__SND_TERMINAL_ON));	//	can we have a startup sequence ?? 

	context->stream_handle = audio_stream_open(resources_find_audioclip(RES__SND_TERMINAL_BG), AUDIO_STREAM_MUSIC);
	audio_stream_volume(context->stream_handle, 32);
}

//-----------------------------------------------------------------------------
void st_terminal_exit(st_terminal_context_ptr context)
{
	audio_stream_close(context->stream_handle);

	overlay_destroy_panel(context->terminal_panel_id);

	sequence_player_ptr player = context->terminal_sequence_player;
	if (player != NULL)
	{
		if (player->persistent != NULL)
		{
			memory_free(player->persistent);
		}
		if (player->event_handlers != NULL)
		{
			memory_free(player->event_handlers);
		}
		memory_free(player);
	}

	if (context->terminal_ui_palette != NULL)
	{
		memory_free(context->terminal_ui_palette);
		context->terminal_ui_palette = NULL;
	}
}

//-----------------------------------------------------------------------------
void st_terminal_update(st_terminal_context_ptr context, fixed16_t dt)
{
	if (key_hit(KI_B))
	{
		state_pop(0);
		return;
	}

	if (context->terminal_state == 0)	//	sequence player
	{
		if (timer_expired(context->terminal_sequence_timer))
		{
			context->terminal_sequence_timer_completion_flag = 1;
		}

		sequence_update(context->terminal_sequence_player);
	}
	else if (context->terminal_state == 1)	//	entering command
	{
		if (context->terminal_working_buffer[context->terminal_readout_idx])
		{
			//	don't do this every frame !!! 
			st_terminal_write_character(context, context->terminal_working_buffer[context->terminal_readout_idx]);
			context->terminal_readout_idx++;
		}
		else
		{
			context->terminal_state = 0;
			context->terminal_readout_idx = 0;
			context->terminal_current_line_idx++;
		}
	}

	/*
	blinking cursor
	*/
	if (timer_expired(context->terminal_cursor_blink_timer))
	{
		context->terminal_cursor_blink_timer = timer_start(10, TIMER_MODE_ONCE);

		context->terminal_cursor_blink_state ^= 1;
		if (context->terminal_cursor_blink_state)
		{
			point2_t size = { 8, 8 };
			overlay_draw_color(context->terminal_panel_id, 0, context->terminal_cursor_position, size);
		}
		else
		{
			overlay_draw_character(context->terminal_panel_id, '_', 2, context->terminal_cursor_position);
		}
	}
}


//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_terminal =
{
	(state_enter_func_ptr)st_terminal_enter,
	(state_exit_func_ptr)st_terminal_exit,

	NULL,
	NULL,

	(state_update_func_ptr)st_terminal_update,
	NULL,

	"st_terminal",
	sizeof(st_terminal_context_t)

};
