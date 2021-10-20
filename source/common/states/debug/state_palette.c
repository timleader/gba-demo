
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
#include "common/audio/audio.h"
#include "common/utils/bitstream.h"
#include "common/debug/debug.h"
#include "common/memory.h"

//-----------------------------------------------------------------------------
typedef struct st_palette_context_s
{
	uint32_t reserved;

} st_palette_context_t;

typedef st_palette_context_t* st_palette_context_ptr;

//-----------------------------------------------------------------------------
void st_palette_enter(st_palette_context_ptr context, uint32_t parameter)
{
	uint16_t resource_id = parameter & 0x0000FFFF;

	palette_ptr palette = resources_find_palette(resource_id);

	graphics_set_resolution(graphics_screen_width, graphics_screen_height);

	graphics_write_palette(palette);

	graphics_clear(255);

	uint8_t idx = 0;
	point2_t pos = { 56, 8 };
	point2_t size = { 8, 8 };

	while (idx < palette->size)
	{
		graphics_draw_color(idx + palette->offset, pos, size);

		pos.x += 8;
		if (pos.x >= 56 + 128)
		{
			pos.x -= 128;
			pos.y += 8;
		}

		++idx;
	}

	graphics_pageflip();
}

//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_palette =
{
	(state_enter_func_ptr)st_palette_enter,
	NULL,

	NULL,
	NULL,

	(state_update_func_ptr)st_common_update,
	NULL,

	"st_palette",
	sizeof(st_palette_context_t)

};
