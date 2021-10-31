
#ifndef WORLD_H
#define WORLD_H

#include "common/types.h"
#include "common/memory.h"
#include "common/resources/resources.h"
#include "common/graphics/graphics.h"
#include "common/graphics/highlight_field.h"
#include "common/collision/collision.h"
#include "common/savegame/savegame.h"
#include "common/video/smacker.h"

#include "games/7days/navigation.h"
#include "games/7days/sequence.h"
#include "games/7days/entities/entity.h"


//-----------------------------------------------------------------------------
#define WORLD_MODEL_ENTITY_COUNT 4
#define WORLD_IMAGE_ENTITY_COUNT 8
#define WORLD_NAVIGATION_AGENT_COUNT 4

#define WORLD_LEVEL_COUNT 8

#define WORLD_MAX_VIEWS_PER_LEVEL 8


//-----------------------------------------------------------------------------
typedef struct level_persistent_s
{
	uint16_t layer;
	uint16_t trigger_mask;
	uint16_t collision_mask;
	uint16_t reserved;

	entity_t entities[WORLD_MODEL_ENTITY_COUNT];		//	these are just render entities			//	maybe have a pointer to the active levels's entity 

	uint8_t view_state[WORLD_MAX_VIEWS_PER_LEVEL];		//	door open / door closed 
		//	bit mask to indicate the state of the view and what subview things should be active ... 
	//	playing progress of subviews will be in the ephermal struct

	//	persistent state 
		//	on / off


	//	off not visible
	//	on visible at last frame 
	//  on visible continuous looping 
	//	on visible continuous flip floping 


	//	ephermal representation 
		//	

	//interaction_points[8]; triggers[8]; -- leave these where they are for now

	navigation_agent_t nav_agents[WORLD_NAVIGATION_AGENT_COUNT];		//	should this be per levels ... if they are not are they just ephermal 

} level_persistent_t;

typedef level_persistent_t* level_persistent_ptr;

/*
	Keeping the persistent data separate so we can save and load with just a memory_copy 
	ensure ephermeral data uses persistent as target, so it will switch view etc based on 
	persistent data. 
*/

//-----------------------------------------------------------------------------
typedef struct world_persistent_s
{
	//	dialogue persistence 


	int16_t ambient_resource_id;
	int16_t music_resource_id;


	// worth copying triggers & collision here so we access them from EWRAM and can alter them...

	//	view_mask, more generic approach maybe   ?? 

	//	interaction mask
	//	entity mask

	inventory_t inventory;			//	this is more of a play_context than world

	uint32_t sequencer_state[4];	
	sequence_channel_persistent_t sequencer_channel;	//	potentially 2 of these

	uint16_t level_resource_id;		//	if this changes the game should too

	uint8_t level_idx;				//	take index from level_t
	uint8_t view_idx;				//	should be a limit of 16 views per scene 

	uint32_t trigger_check_state;	//	multiple of these ?? 	

	player_entity_t player;			//	this should control an entity ... as then it is easier for sequencer to take control


	level_persistent_t levels[WORLD_LEVEL_COUNT];
		//	how do we know the levels resource_id from the levels idx used here 

	/*

	//	This does feel right !!!

	this would keep the subview handling much more inline with the entity handling,
		means we aren't solving the same problem twice ...

	consider levels persistence vs world persistence,
		eg. once we open a vent on levels X it should stay open... - how do we store this,

	make interaction points dynamic and store them in persistent layer ... (same as triggers)


	how do we have persistence of dialogue state ... ???

	*/

} world_persistent_t;

//-----------------------------------------------------------------------------
typedef struct world_ephemeral_s		//	need to add appropriate padding  to struct for Align4
{
	//	LEVEL

	int16_t ambient_resource_id;
	int16_t music_resource_id;
	audio_stream_handle_t ambient_stream;
	audio_stream_handle_t music_stream;

	/*
		LIMITATIONS:
			max views per scene = 16
			max triggers per scene = 32
	*/ 

	uint16_t loaded_level_resource_id;		//	not needed

	view_t* views;		

	uint8_t numCollision;
	collider_t* collision;		//	ground -> this isn't per view 
		//	how do we augment the collision, maybe the ability to enable / disable collision entity
	uint8_t numNavLinks;
	navigation_link_t* nav_links;

	uint8_t numNavLinkData;
	navigation_link_data_t* nav_link_data;

	//	links

//****	ground	- can ths have a gradient 

	//	quadtree accelerated OBB collision
	//	I'm at position (X,Z) use quadtree to look up OBB to check collision against  
	//		use additional data from OBB to determine Y positioning of agent


	uint8_t trigger_count;
	trigger_t* triggers;	//	baked into level - do we need to dynamically create and destroy 

	//	need to be able to slide along the walk with this

	uint8_t interaction_count;
	interaction_point_t* interactions;

	uint8_t spawn_count;
	spawn_t* spawns;

	vector3_t origin;
	vector3_t scale;


	//	VIEW	CACHED

	uint8_t loaded_view_idx;

	span_t dirty_scanline_span[2]; 

	palette_ptr view_palette;
	rendertarget_ptr view_rendertarget;		
	depthmap_ptr view_depth;

	uint8_t view_state;					//	state of what is actually being displayed 
	int16_t subview_frame_indices[8];
	uint16_t loaded_video_resource_id;
	smacker_handle_ptr subview_video_handle;		//	maybe 2 of these ???


	highlight_field_ptr active_highlight;
	game_timer_t highlight_timer;


	//	SEQUENCER

	sequence_player_t sequencer;
	sequence_channel_ephermeral_t sequencer_channel;

	game_timer_t sequencer_timer;
	int8_t sequencer_timer_completion_flag;


	//	LEVEL MAPPING

	int16_t level_idx_to_resource_id[WORLD_LEVEL_COUNT];

} world_ephemeral_t;

//-----------------------------------------------------------------------------
typedef struct world_s
{
	world_persistent_t persistent;

	world_ephemeral_t ephermeral;

} world_t;

typedef world_t* world_ptr;

//-----------------------------------------------------------------------------
extern world_ptr g_main_world;

//-----------------------------------------------------------------------------
inline static level_persistent_ptr world_current_level(world_ptr world)
{
	uint16_t level_idx = world->persistent.level_idx;
	return &world->persistent.levels[level_idx];
}

//-----------------------------------------------------------------------------
inline static entity_ptr world_find_entity(world_ptr world, int8_t idx)
{
	if (idx == 0x1F)
	{
		return &world->persistent.player.entity;
	}
	else
	{
		debug_assert(idx < WORLD_MODEL_ENTITY_COUNT, "world::find_entity idx >= WORLD_MODEL_ENTITY_COUNT");

		return &world_current_level(world)->entities[idx];
	}
}


//-----------------------------------------------------------------------------
void world_initialize(world_ptr world);

void world_sequence_player_assign_event_handlers(sequence_player_ptr player);

void world_load_level(world_ptr world, uint16_t resource_id);

void world_load_view(world_ptr world, uint16_t view_idx);

void world_reload_palettes(world_ptr world);

void world_update(world_ptr world);

void world_draw(world_ptr world);

void world_delete(world_ptr world);

void world_set_scanlines_dirty(world_ptr world);

//	savegame i/o

void world_newgame(world_ptr world, uint16_t resource_id);

void world_savegame(world_ptr world, tiledimage_ptr preview_image, savegame_ptr slot);

void world_loadgame(world_ptr world, savegame_ptr slot);

void world_savegame_load_preview_image(savegame_ptr slot, tiledimage_ptr dest);


navigation_agent_ptr world_acquire_nav_agent(world_ptr world);

vector2_t world_closestpointonpath(vector2_t point, uint32_t mask);

interaction_point_ptr interaction_point_check(world_ptr world, vector2_t point);


#endif 
