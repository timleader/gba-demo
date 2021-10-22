
#include "world.h"

#include "common/graphics/graphics.h"
#include "common/utils/profiler.h"
#include "common/states/debug/states.h"

#include "games/7days/sequence.h"
#include "games/7days/states/states.h"


//-----------------------------------------------------------------------------
typedef enum sequence_event_type_s
{
	WORLD_WAIT = 0,		//	Frame Count

	WORLD_DIALOGUE = 1,		//	Dialogue id								<<	Pushes State

	WORLD_GAMEOVER = 2,

	WORLD_ENTITY_PATH = 3,		//	be able to define a path that should be used

	WORLD_SOUND = 4,		//	Sound id

	WORLD_VIDEO = 5,		//	Video id, Frame X to Frame Y			<<	Pushes State

	//	DISPLAY IMAGE --- eg. zoom in on map

	//	Lift Menu ?? 

	//	switch player model ?? 

	//	DIE


	WORLD_COLLISION_CHANGE = 8,	//	

	WORLD_SCENE_CHANGE = 9,			//	 rename: LEVEL_CHANGE
	WORLD_VIEW_CHANGE = 10,			// VIEW ID

	WORLD_TRIGGER_MASK_CHANGE = 11,	//	TRIGGER MASK
	WORLD_COLLISION_MASK_CHANGE = 12,

	WORLD_INVENTORY_ADD = 13,
	WORLD_INVENTORY_REMOVE = 14,

	WORLD_TERMINAL = 15,

	WORLD_INPUT_LOCK = 16,

	WORLD_ENTITY_SPAWN = 17,

	WORLD_ELEVATOR = 18,

	WORLD_IMAGE = 19,

	WORLD_ENTITY_ANIMATION = 20,		//
	WORLD_ENTITY_DESTROY = 21,

	WORLD_VIEW_STATE_CHANGE = 22,

	WORLD_SFX = 23,


	//	Manipulate - Ambient, Background 


	//	Block Input ??? 

} sequence_event_type_t;

//-----------------------------------------------------------------------------
sequence_completion_mode_t sequence_event_handler_wait(sequence_player_ptr player, sequence_event_t event)
{
	uint32_t waitFrameCount = SEQUENCE_EVENT_EXTRA(event);

	g_main_world->ephermeral.sequencer_timer = timer_start((uint16_t)waitFrameCount, TIMER_MODE_ONCE);
	g_main_world->ephermeral.sequencer_timer_completion_flag = 0;

	player->ephermeral_channels[0].completion_flag = &g_main_world->ephermeral.sequencer_timer_completion_flag;

	return SEQUENCE_COMPLETION_WAIT_ON_FLAG;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t sequence_event_handler_dialogue(sequence_player_ptr player, sequence_event_t event)		//	how do we know if is complete >??
{
	state_push(&st_dialogue, SEQUENCE_EVENT_EXTRA(event));

	return SEQUENCE_COMPLETION_NEXT_TICK;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t sequence_event_handler_video(sequence_player_ptr player, sequence_event_t event)		//	how do we know if is complete >??
{
	state_push(&st_video, SEQUENCE_EVENT_EXTRA(event));	//	need to pass over start and finish frame

	return SEQUENCE_COMPLETION_NEXT_TICK;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t sequence_event_handler_inventory_add(sequence_player_ptr player, sequence_event_t event)
{
	uint16_t item_id = SEQUENCE_EVENT_EXTRA(event) & 0x0000FFFF;

	inventory_additem(&g_main_world->persistent.inventory, item_id);

	//	show some awarding UI 

	return SEQUENCE_COMPLETION_IMMEDIATE;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t sequence_event_handler_inventory_remove(sequence_player_ptr player, sequence_event_t event)
{
	uint16_t item_id = SEQUENCE_EVENT_EXTRA(event) & 0x0000FFFF;

	inventory_removeitem(&g_main_world->persistent.inventory, item_id);

	//	show some consuming UI 

	return SEQUENCE_COMPLETION_IMMEDIATE;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t sequence_event_handler_terminal(sequence_player_ptr player, sequence_event_t event)		//	how do we know if is complete >??
{
	state_push(&st_terminal, SEQUENCE_EVENT_EXTRA(event));	//	need to pass over start and finish frame

	return SEQUENCE_COMPLETION_NEXT_TICK;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t sequence_event_handler_elevator(sequence_player_ptr player, sequence_event_t event)		//	how do we know if is complete >??
{
	state_push(&st_elevator, SEQUENCE_EVENT_EXTRA(event));	//	need to pass over start and finish frame

	return SEQUENCE_COMPLETION_NEXT_TICK;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t sequence_event_handler_image(sequence_player_ptr player, sequence_event_t event)		//	how do we know if is complete >??
{
	state_push(&st_image, SEQUENCE_EVENT_EXTRA(event));	//	need to pass over start and finish frame

	return SEQUENCE_COMPLETION_NEXT_TICK;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t sequence_event_handler_world_change_level(sequence_player_ptr player, sequence_event_t event)
{
	uint16_t resource_id = SEQUENCE_EVENT_EXTRA(event) & 0x0000FFFF;

	//	some kind of world reset to clear un-needed state 

	//	aaa

	world_load_level(g_main_world, resource_id);

	return SEQUENCE_COMPLETION_IMMEDIATE;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t sequence_event_handler_world_change_view(sequence_player_ptr player, sequence_event_t event)
{
	uint16_t view_idx = SEQUENCE_EVENT_EXTRA(event) & 0x0000FFFF;

	g_main_world->persistent.view_idx = view_idx;

	return SEQUENCE_COMPLETION_IMMEDIATE;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t sequence_event_handler_world_change_trigger_mask(sequence_player_ptr player, sequence_event_t event)
{
	//uint32_t extra_data = SEQUENCE_EVENT_EXTRA(event);
	//uint32_t trigger_mask = extra_data & 0xFF;
	//uint8_t mask_combine_op = (extra_data >> 16) & 0x03;
	/*
	if (mask_combine_op == 0)	// Add
	{
		filter_mask |= trigger_mask;
	}
	else if (mask_combine_op == 1) // Remove
	{
		filter_mask &= ~trigger_mask;
	}
	else if (mask_combine_op == 2) // replace
	{
		filter_mask = trigger_mask;
	}
	*/
	return SEQUENCE_COMPLETION_IMMEDIATE;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t sequence_event_handler_world_change_collision_mask(sequence_player_ptr player, sequence_event_t event)
{
	//uint32_t extra_data = SEQUENCE_EVENT_EXTRA(event);
	//uint32_t trigger_mask = extra_data & 0x00FF;
	//uint8_t mask_combine_op = (extra_data >> 16) & 0x03;


	return SEQUENCE_COMPLETION_IMMEDIATE;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t sequence_event_handler_gameover(sequence_player_ptr player, sequence_event_t event)
{
	state_goto(&st_gameover, 0);

	return SEQUENCE_COMPLETION_NEXT_TICK;
}

//	tween handler is required for things like doors

//-----------------------------------------------------------------------------
sequence_completion_mode_t sequence_event_handler_entity_path(sequence_player_ptr player, sequence_event_t event)
{
	packed_vector2_t packed_destination;
	int8_t entity_id;

	uint32_t extra_data = SEQUENCE_EVENT_EXTRA(event);

	packed_destination.x = (extra_data) & 0x000000FF;
	packed_destination.y = (extra_data >> 8) & 0x000000FF;

	entity_id = (extra_data >> 16) & 0x0000001F;

	//	8-bits for other parameters, eg. speed 

	vector2_t unpack_origin = { g_main_world->ephermeral.origin.x, g_main_world->ephermeral.origin.z };
	vector2_t unpack_scale = { g_main_world->ephermeral.scale.x, g_main_world->ephermeral.scale.z };

	entity_ptr entity = world_find_entity(g_main_world, entity_id);		//	need to be able to select player entity too 

	vector2_t origin, destination;

	origin.x = entity->position.x;
	origin.y = entity->position.z;

	destination.x = ((unpack_scale.x * packed_destination.x) >> 8) + unpack_origin.x;
	destination.y = ((unpack_scale.y * packed_destination.y) >> 8) + unpack_origin.y;

	navigation_agent_ptr nav_agent = world_acquire_nav_agent(g_main_world);
	nav_agent->entity_id = entity_id;		//	have a better reset than this 
	nav_agent->waypoint_idx = 0;
	nav_agent->completion_flag = 0;
	nav_agent->step_count = 0;
	nav_agent->step_vector.x = 0;
	nav_agent->step_vector.y = 0;

	navigation_generate_path(origin, destination, &nav_agent->path);

	player->ephermeral_channels[0].completion_flag = &nav_agent->completion_flag;	//	this should be optional !!! 

	return SEQUENCE_COMPLETION_WAIT_ON_FLAG;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t sequence_event_handler_input_lock(sequence_player_ptr player, sequence_event_t event)
{
	uint8_t lock = SEQUENCE_EVENT_EXTRA(event) & 0x00000001;

	g_main_world->persistent.player.active = lock;

	return SEQUENCE_COMPLETION_IMMEDIATE;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t sequence_event_handler_model_entity_spawn(sequence_player_ptr player, sequence_event_t event)
{
	uint32_t extra_data = SEQUENCE_EVENT_EXTRA(event);

	//	NOT QUITE ENOUGH DATA 
	uint16_t model_resource_id = (extra_data) & 0x000003FF;
	uint8_t entity_id = (extra_data >> 10) & 0x0000001F;
	uint8_t spawn_id = (extra_data >> 15) & 0x0000001F;		//	maybe spawn nodes should be locator points, can be used for various things 

	//	how do we determine the idx ???

	entity_ptr entities = world_current_level(g_main_world)->entities;

	entities[entity_id].id = entity_id;
	entities[entity_id].layer = ~0;	//	should eventually be 0 

	mathVector3Copy(&entities[entity_id].position, &g_main_world->ephermeral.spawns[spawn_id].position);
	mathVector3MakeFromElements(&entities[entity_id].rotation, fixed16_zero, g_main_world->ephermeral.spawns[spawn_id].yaw, fixed16_zero);

	entities[entity_id].model_resource_id = model_resource_id;
	entities[entity_id].frame = 0;

	return SEQUENCE_COMPLETION_IMMEDIATE;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t sequence_event_handler_entity_destroy(sequence_player_ptr player, sequence_event_t event)
{
	uint32_t extra_data = SEQUENCE_EVENT_EXTRA(event);

	uint8_t entity_id = (extra_data >> 0) & 0x000000FF;

	entity_ptr entities = world_current_level(g_main_world)->entities;

	entities[entity_id].id = -1;
	entities[entity_id].layer = 0;	//	should eventually be 0 

	return SEQUENCE_COMPLETION_IMMEDIATE;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t sequence_event_handler_entity_layer(sequence_player_ptr player, sequence_event_t event)
{
	return SEQUENCE_COMPLETION_IMMEDIATE;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t sequence_event_handler_view_state_change(sequence_player_ptr player, sequence_event_t event)
{
	uint32_t extra_data = SEQUENCE_EVENT_EXTRA(event);

	// could have a seperate command to position it

	uint16_t view_idx = (extra_data >> 0) & 0x0000000F;
	uint8_t subview_idx = (extra_data >> 5) & 0x00000007;
	uint8_t target_state = (extra_data >> 9) & 0x00000001;

	level_persistent_ptr level = world_current_level(g_main_world);

	if (target_state)
	{
		level->view_state[view_idx] |= (1 << subview_idx);
	}
	else
	{
		level->view_state[view_idx] &= ~(1 << subview_idx);
	}

	return SEQUENCE_COMPLETION_IMMEDIATE;
}

//-----------------------------------------------------------------------------
sequence_completion_mode_t seuqence_event_handler_audio_sfx(sequence_player_ptr player, sequence_event_t event)
{
	uint32_t extra_data = SEQUENCE_EVENT_EXTRA(event);
	uint16_t sfx_resource_id = (extra_data) & 0x000003FF;

	audioclip_ptr clip = resources_find_audioclip(sfx_resource_id);
	audio_sfx_play(clip);

	//	3d, use spawn point maybe ... 

	return SEQUENCE_COMPLETION_IMMEDIATE;
}


//-----------------------------------------------------------------------------
sequence_event_handler_t g_world_sequence_event_handlers[] = 
{
	sequence_event_handler_wait,
	sequence_event_handler_dialogue,
	sequence_event_handler_gameover, 
	sequence_event_handler_entity_path,
	NULL,
	sequence_event_handler_video,
	NULL,
	NULL,
	NULL,
	sequence_event_handler_world_change_level,
	sequence_event_handler_world_change_view,
	sequence_event_handler_world_change_trigger_mask,
	NULL,
	sequence_event_handler_inventory_add,
	sequence_event_handler_inventory_remove,
	sequence_event_handler_terminal,
	sequence_event_handler_input_lock,
	sequence_event_handler_model_entity_spawn,
	sequence_event_handler_elevator,
	NULL,
	NULL,
	sequence_event_handler_entity_destroy,
	sequence_event_handler_view_state_change,
	seuqence_event_handler_audio_sfx
};

//-----------------------------------------------------------------------------
void world_sequence_player_initialize(sequence_player_ptr player, uint32_t* state)
{
	player->persistent_state = state;


	player->event_handlers = &g_world_sequence_event_handlers;


	player->channel_count = 1;


	/*
	player->ephermeral_channels = 

	player->persistent->current_event = -1;
	player->persistent->scheduled_event = -1;

	player->state = PLAYER_STATE_IDLE;

	player->channel_count = 1;*/
	//player->ephermeral_channels = //	

}

//-----------------------------------------------------------------------------
void world_sequence_player_delete(sequence_player_ptr player)
{
}
