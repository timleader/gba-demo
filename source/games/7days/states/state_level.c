
#include "games/7days/states/states.h"

#include "common/math/matrix.h"
#include "common/math/trigonometry.h"
#include "common/math/point.h"

#include "common/graphics/graphics.h"
#include "common/graphics/image.h"
#include "common/graphics/camera.h"
#include "common/graphics/text.h"
#include "common/collision/collision.h"
#include "common/resources/resources.h"
#include "common/video/smacker.h"
#include "common/input/input.h"
#include "common/utils/timer.h"
#include "common/utils/profiler.h"
#include "common/memory.h"
#include "common/states/debug/states.h"

#include "games/7days/world.h"
#include "games/7days/widgets/widget_quickaction.h"

#include "package_7days.h"


//-----------------------------------------------------------------------------
typedef struct st_level_context_s	
{
	uint32_t time, now;

	widget_quickaction_state_t widget_state;
	interaction_point_t* cur_interaction;

} st_level_context_t;

typedef st_level_context_t* st_level_context_ptr;


//-----------------------------------------------------------------------------
void st_level_update(st_level_context_ptr context, fixed16_t dt)
{
	if (key_hit(KI_START))
	{
		state_push(&st_pause, 0);
		return;
	}

	if (key_hit(KI_SELECT))
	{
		state_push(&st_analysis, 0);
		return;
	}

	/// --------

	world_update(g_main_world);

	//	if we have pushed another state we don't want to do the rest 

	entity_ptr entity = &g_main_world->persistent.player.entity;
	vector2_t point = { entity->position.x, entity->position.z };

	//	world could track 
	interaction_point_t* interaction = interaction_point_check(g_main_world, point);	
	if (interaction)
	{
		context->cur_interaction = interaction;

		if (interaction->highlight_field_resource_id)
			g_main_world->ephermeral.active_highlight = resources_find_highlightfield(interaction->highlight_field_resource_id);

		widget_quickaction_schedule_interaction(&context->widget_state, interaction);

		if (key_hit(KI_A))	//	this needs to be player_controller input lock
		{
			audio_sfx_play(resources_find_audioclip(RES__SND_ACCEPT));

			state_push(&st_inventory, (uint32_t)interaction);	//	pass the interaction, can we do it by id? ??, could just pass a pointer .. .
		}
	}
	else
	{
		context->cur_interaction = 0;
		g_main_world->ephermeral.active_highlight = NULL;
		widget_quickaction_clear(&context->widget_state);
	}

	widget_quickaction_update(&context->widget_state);


	// if input(A) && widget_quickaction_ready()
	//		interaction process within world .. 


	//character_depthlayer = calculate_player_depthlayer();

	//	do something with depthlayer

}

//-----------------------------------------------------------------------------
void st_level_draw(st_level_context_ptr context, fixed16_t dt)
{
	//profiler_sample_handle_t phandle;

	//phandle = profiler_begin("w:draw");

	//	if first frame after resume don't render ... ??? 
	//	sequencer could be told to not render ... 
	//		in addition we will need to prevent palette write ... (where does this happen)


	world_draw(g_main_world);

	//profiler_end(phandle);

	graphics_pageflip();
}

//-----------------------------------------------------------------------------
void st_level_enter(st_level_context_ptr context, uint32_t parameter)
{
	uint8_t is_newgame = parameter & 0x01;
	uint16_t resource_id = (parameter >> 1) & 0xFFFF;

	uint8_t savegame_slot_id = ((parameter >> 1) & 0x03);

	graphics_set_resolution(graphics_screen_width, graphics_screen_height);
	graphics_set_vsync(3);

	world_initialize(g_main_world);

	if (is_newgame)
	{
		world_newgame(g_main_world, resource_id);
	}
	else
	{
		world_loadgame(g_main_world, &savegame_slots->slots[savegame_slot_id]);		
	}

	widget_quickaction_initialize(&context->widget_state);

	world_set_scanlines_dirty(g_main_world);

	//	pre-emptively process one step of the sequencer 
	sequence_update(&g_main_world->ephermeral.sequencer);		
}

//-----------------------------------------------------------------------------
void st_level_exit(st_level_context_ptr context)
{
	widget_quickaction_shutdown(&context->widget_state);

	world_delete(g_main_world);
}

//-----------------------------------------------------------------------------
void st_level_pause(st_level_context_ptr context)
{
	widget_quickaction_shutdown(&context->widget_state);

	//	unneeded allocation and copy have an easier way - fade might be nicer than a snap
	palette_ptr graphics_palette = palette_new(256, MEMORY_EWRAM);	//	can we just create this on the stack 
	
	graphics_read_palette(graphics_palette);
	palette_lerp(graphics_palette, graphics_palette, 0, fixed16_half);
	graphics_write_palette(graphics_palette);

	palette_delete(graphics_palette);
}

//-----------------------------------------------------------------------------
void st_level_resume(st_level_context_ptr context, uint32_t parameter)
{
	graphics_set_resolution(graphics_screen_width, graphics_screen_height);
	graphics_set_vsync(3);

	//		THIS IS WEIRD !!! 

	//	process result from previously pushed state, 
	uint8_t command = (parameter >> 24) & 0x000000FF;
	if (command == 1)	//	sequence, 
	{
		int16_t sequence_id = parameter & 0x0000FFFF;

		sequence_schedule(&g_main_world->ephermeral.sequencer, 0, sequence_id);
	}
	else if (command == 3)	//	load levels !!!		//	this could just be a sequence playback thing
	{
		//	can this just use the sequence schedule approach ...

		uint16_t resource_id = parameter & 0x000003FF;			// 10-bits
		uint8_t spawn_idx = (parameter >> 10) & 0x0000003F;		// 6-bits

		world_load_level(g_main_world, resource_id);

		uint8_t view_idx = g_main_world->ephermeral.spawns[spawn_idx].viewIndex;		

		g_main_world->persistent.view_idx = view_idx;
		world_load_view(g_main_world, view_idx);		

		//	spawn point
		mathVector3Copy(&g_main_world->persistent.player.entity.position, &g_main_world->ephermeral.spawns[spawn_idx].position);
		mathVector3MakeFromElements(&g_main_world->persistent.player.entity.rotation, fixed16_zero, g_main_world->ephermeral.spawns[spawn_idx].yaw, fixed16_zero);
		
		// give spawn_id instead of view_id
	}

	widget_quickaction_initialize(&context->widget_state);
	if (context->cur_interaction)
	{
		//	check it still exist
		widget_quickaction_schedule_interaction(&context->widget_state, context->cur_interaction);
		widget_quickaction_snap_to_ready(&context->widget_state);
	}

	graphics_write_palette(g_main_world->ephermeral.view_palette);		//	only wants to do this if needed ... 

	world_set_scanlines_dirty(g_main_world);

	//	pre-emptively process one step of the sequencer 
	sequence_update(&g_main_world->ephermeral.sequencer);	

	//	do we still require a world render ... 
}

//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_level =
{
	(state_enter_func_ptr)st_level_enter,
	(state_exit_func_ptr)st_level_exit,

	(state_pause_func_ptr)st_level_pause,
	(state_resume_func_ptr)st_level_resume,

	(state_update_func_ptr)st_level_update,
	(state_draw_func_ptr)st_level_draw,

	"st_level",
	sizeof(st_level_context_t)
};
