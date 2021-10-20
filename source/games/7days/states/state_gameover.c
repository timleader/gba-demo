
#include "games/7days/states/states.h"

#include "common/math/matrix.h"
#include "common/math/point.h"

#include "common/graphics/graphics.h"
#include "common/graphics/overlay.h"
#include "common/graphics/image.h"
#include "common/graphics/camera.h"
#include "common/graphics/text.h"
#include "common/collision/collision.h"
#include "common/resources/resources.h"
#include "common/input/input.h"
#include "common/memory.h"

#include "package_7days.h"


//-----------------------------------------------------------------------------
typedef struct st_gameover_context_s
{
	uint8_t panel_gameover;

	uint8_t reserved[3];

} st_gameover_context_t;

typedef st_gameover_context_t* st_gameover_context_ptr;

//-----------------------------------------------------------------------------
void st_gameover_enter(st_gameover_context_ptr context, uint32_t parameter)
{
	palette_ptr palette = resources_find_palette(RES__PAL_UI_DEFAULT);
	overlay_write_palette(palette);

	context->panel_gameover = overlay_create_panel(3, 8);

	point2_t panel_position = { 80, 160 - 64 };
	overlay_set_position(context->panel_gameover, panel_position);

	overlay_clear(context->panel_gameover, 0);

	point2_t string_position = { 1, 0 };
	overlay_draw_string(context->panel_gameover, "game over", 1, string_position);
}

//-----------------------------------------------------------------------------
void st_gameover_exit(st_gameover_context_ptr context)
{
	overlay_destroy_panel(context->panel_gameover);
}

//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_gameover =
{
	(state_enter_func_ptr)st_gameover_enter,
	(state_exit_func_ptr)st_gameover_exit,

	NULL,
	NULL,

	NULL,
	NULL,

	"st_gameover",
	sizeof(st_gameover_context_t)

};
