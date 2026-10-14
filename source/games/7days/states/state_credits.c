
#include "games/7days/states/states.h"

#include "common/graphics/overlay.h"
#include "common/utils/timer.h"
#include "common/math/easing.h"
#include "common/input/input.h"


//-----------------------------------------------------------------------------
typedef struct st_credits_context_s
{
	uint32_t credits_text_offset;

	uint16_t scroll_position_y;

	uint8_t panel_credits_view;

} st_credits_context_t;

typedef st_credits_context_t* st_credits_context_ptr;

//-----------------------------------------------------------------------------
void st_credits_draw_view(st_credits_context_ptr context)
{
	overlay_clear(context->panel_credits_view, 0);

}

//-----------------------------------------------------------------------------
void st_credits_enter(st_credits_context_ptr context, uint32_t parameter)
{
	context->scroll_position_y = 0;
	context->panel_credits_view = overlay_create_panel(6, 18);

	point2_t pos = { 22, 8 };
	overlay_set_position(context->panel_credits_view, pos);

	st_credits_draw_view(context);

	palette_ptr ui_palette = resources_find_palette_from_name("pal/ui/default");
	overlay_write_palette(ui_palette);
}

//-----------------------------------------------------------------------------
void st_credits_exit(st_credits_context_ptr context)
{
	overlay_destroy_panel(context->panel_credits_view);
}

//-----------------------------------------------------------------------------
void st_credits_update(st_credits_context_ptr context, fixed16_t t)
{
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