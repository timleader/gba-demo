
#include "games/7days/states/states.h"

#include "common/graphics/overlay.h"
#include "common/utils/timer.h"
#include "common/math/easing.h"
#include "common/input/input.h"
#include "common/string.h"
#include "common/resources/resources.h"

//-----------------------------------------------------------------------------
const char* credits_text =
	"This is the credits screen.\n"
	"Tim\n"
	"Bob\n";

//-----------------------------------------------------------------------------
typedef struct st_credits_context_s
{
	uint32_t credits_text_offset;

	uint8_t panel_credits_pageview[2];
	point2_t panel_credits_pageview_pos[2];
	
	game_timer_t scroll_timer;

} st_credits_context_t;

typedef st_credits_context_t* st_credits_context_ptr;

//-----------------------------------------------------------------------------
void st_credits_draw_view(st_credits_context_ptr context)
{
	char text_line_buffer[128];
	char text_wrapped_buffer[128];

	overlay_clear(context->panel_credits_pageview[0], 0);
	overlay_clear(context->panel_credits_pageview[1], 0);

	int32_t newline_index = string_index_of(credits_text, '\n');
	if (newline_index != -1)
	{
		memory_copy8(&text_line_buffer[0], credits_text, newline_index);
		text_line_buffer[newline_index] = '\0';
	}
	else
	{
		uint32_t text_length = string_length(credits_text);
		if (text_length > 127)
			text_length = 127;
		memory_copy8(&text_line_buffer[0], credits_text, text_length);
		text_line_buffer[text_length] = '\0';
	}

	//string_wrap(&text_buffer[0], &text_buffer[0], 128);
	//	wrap 
	//	center as we write ... 

	point2_t text_pos = { 0, 0 };
	overlay_draw_string(context->panel_credits_pageview[0], &text_line_buffer[0], 1, text_pos);
}

//-----------------------------------------------------------------------------
void st_credits_enter(st_credits_context_ptr context, uint32_t parameter)
{
	context->scroll_timer = timer_start(4, TIMER_MODE_UNBOUNDED);

	context->panel_credits_pageview_pos[0] = (point2_t){ 22, 8 };
	context->panel_credits_pageview_pos[1] = (point2_t){ 22, 160 };
	context->panel_credits_pageview[0] = overlay_create_panel(6, 6);
	context->panel_credits_pageview[1] = overlay_create_panel(6, 6);

	point2_t pos = { 22, 8 };
	overlay_set_position(context->panel_credits_pageview[0], context->panel_credits_pageview_pos[0]);
	overlay_set_position(context->panel_credits_pageview[1], context->panel_credits_pageview_pos[1]);

	st_credits_draw_view(context);

	palette_ptr ui_palette = resources_find_palette_from_name("pal/ui/default");
	overlay_write_palette(ui_palette);
}

//-----------------------------------------------------------------------------
void st_credits_exit(st_credits_context_ptr context)
{
	overlay_destroy_panel(context->panel_credits_pageview[0]);
	overlay_destroy_panel(context->panel_credits_pageview[1]);
}

//-----------------------------------------------------------------------------
void st_credits_update(st_credits_context_ptr context, fixed16_t t)
{
	context->panel_credits_pageview_pos[0].y = fixed16_to_int(timer_progress(context->scroll_timer));
	overlay_set_position(context->panel_credits_pageview[0], context->panel_credits_pageview_pos[0]);

	//	page view swap .. 

	if (key_hit(KI_B))
	{
		state_pop(0);
	}
}

//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_credits =
{
	(state_enter_func_ptr)st_credits_enter,
	(state_exit_func_ptr)st_credits_exit,

	NULL,
	NULL,

	(state_update_func_ptr)st_credits_update,
	NULL,

	"st_credits",
	0
};