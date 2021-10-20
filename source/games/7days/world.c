
#include "world.h"

#include "common/graphics/graphics.h"
#include "common/utils/profiler.h"
#include "common/states/debug/states.h"

#include "games/7days/sequence.h"
#include "games/7days/states/states.h"

world_t g_world;	// put this on the heap

world_ptr g_main_world = &g_world;

//-----------------------------------------------------------------------------
void world_initialize(world_ptr world)
{
	debug_printf(DEBUG_LOG_DEBUG, "world::initialize");

	world->ephermeral.view_rendertarget = rendertarget_new(graphics_screen_width, graphics_screen_height, MEMORY_EWRAM);

	/* Sequencer Initialization */

	world_sequence_player_initialize(&world->ephermeral.sequencer, &world->persistent.sequencer_state);
	
	/* World Initialize fields  */

	world->persistent.level_resource_id = ~0;
	world->persistent.level_idx = 0;

	world->ephermeral.loaded_level_resource_id = ~0;
	world->ephermeral.loaded_view_idx = ~0;

	/* Build level idx to resource_id map */		//	do we need this 

	for (int16_t idx = 0; idx < WORLD_LEVEL_COUNT; ++idx)
		world->ephermeral.level_idx_to_resource_id[idx] = -1;

	for (int16_t idx = 0; idx < resource_count; ++idx)
	{
		if (resources[idx].type == RESOURCE_TYPE_LEVEL)
		{
			level_ptr level = resources_find_level(idx);
			debug_assert(level->level_idx < WORLD_LEVEL_COUNT, "world::initialize level_idx >= WORLD_LEVEL_COUNT");

			world->ephermeral.level_idx_to_resource_id[level->level_idx] = idx;
		}
	}

	world->ephermeral.highlight_timer = timer_start(20, TIMER_MODE_LOOP);
	world->ephermeral.active_highlight = NULL;
}

void world_delete(world_ptr world)
{
	rendertarget_delete(world->ephermeral.view_rendertarget);

	world_sequence_player_delete(&world->ephermeral.sequencer);
}

//-----------------------------------------------------------------------------
void world_load_level(world_ptr world, uint16_t resource_id)
{
	debug_printf(DEBUG_LOG_DEBUG, "world::load_level %u", resource_id);

	level_ptr level = resources_find_level(resource_id);
	uint8_t* ptr = (uint8_t*)level;

	//	if we have a map somewhere then we don't need to directly use resource_id !!! 
	world->persistent.level_resource_id = resource_id;	//	this shouldn't be here  ??
	world->persistent.level_idx = level->level_idx;	// source this from level --- this is critical to things not breaking !!!!! 

	debug_assert(level->numView <= 16, "level contains too many views");

	world->ephermeral.views = (view_t*)(ptr + level->viewPtrOffset);			//	might be able to resolve these pointers at build-time .. 
	world->ephermeral.numCollision = level->numCollision;
	world->ephermeral.collision = (collider_t*)(ptr + level->collisionPtrOffset);
	world->ephermeral.numNavLinks = level->numNavigationLink;
	world->ephermeral.nav_links = (navigation_link_t*)(ptr + level->navigationLinkPtrOffset);
	world->ephermeral.numNavLinkData = level->numNavigationLinkData;
	world->ephermeral.nav_link_data = (navigation_link_data_t*)(ptr + level->navigationLinkDataPtrOffset);
	world->ephermeral.trigger_count = level->numTrigger;
	world->ephermeral.triggers = (trigger_t*)(ptr + level->triggerPtrOffset);
	world->ephermeral.interaction_count = level->numInteraction;
	world->ephermeral.interactions = (interaction_point_t*)(ptr + level->interactionPtrOffset);
	world->ephermeral.spawn_count = level->numSpawn;
	world->ephermeral.spawns = (spawn_t*)(ptr + level->spawnPtrOffset);		//	spawn positions are broken 

	//	do we need the ephermeral one if we aren't auto switching, which we probably shouldn't as switching levels is more complex than switching views. 
	world->ephermeral.loaded_level_resource_id = resource_id;		
	
	if (level->sequence_store_id >= 0)
	{
		sequence_store_ptr store = resources_find_sequencestore(level->sequence_store_id);	//	this need to come from somewhere sensible
		world->ephermeral.sequencer.store = store;
	}

	//world_clear_entries(world);
}

//-----------------------------------------------------------------------------
/*void world_clear_entries(world_ptr world)
{
	for (uint16_t idx = 0; idx < world->entity_count; ++idx)
		world->entities[idx].id = -1;
}*/

//	it's messy having to be called externally in order to make things work 
void world_set_scanlines_dirty(world_ptr world)
{
	world_ephemeral_t* ephermeral = &world->ephermeral;

	ephermeral->dirty_scanline_span[0].start = 0;
	ephermeral->dirty_scanline_span[0].end = g_graphics_context.height;
	ephermeral->dirty_scanline_span[1].start = 0;
	ephermeral->dirty_scanline_span[1].end = g_graphics_context.height;
}

//-----------------------------------------------------------------------------
void world_load_view(world_ptr world, uint16_t view_idx)	//	internal only !!! 
{
	debug_printf(DEBUG_LOG_DEBUG, "world::load_view %u", view_idx);

	world_ephemeral_t* ephermeral = &world->ephermeral;

	ephermeral->loaded_view_idx = view_idx;
	image_ptr image_cart = resources_find_image(ephermeral->views[view_idx].image_id);

	graphics_bind_rendertarget(ephermeral->view_rendertarget);
	graphics_draw_image(image_cart, 0, 0, 0);
	graphics_bind_rendertarget(NULL);

	ephermeral->view_palette = resources_find_palette(image_cart->palette_id);
	ephermeral->view_depth = resources_find_depthmap(ephermeral->views[view_idx].depthmap_id);

	graphics_bind_depthmap(ephermeral->view_depth);
	graphics_set_depth_layer(0);

	graphics_write_palette(ephermeral->view_palette);

	/*
		Graphics Pal 
			0 - 127		= backgrounds
			128 - 191	= subviews
			192 - 255	= entities
	*/
	level_persistent_ptr current_level = world_current_level(world);
	current_level->layer = (1 << view_idx);


	//	apply persistent subviews here 

	/*
		SUBVIEW use-cases
			- Doors										- temporal, proxmity triggered 
			- Opeing a vent								- persistent, interaction triggered ( this could be a 3d entity ) 
			- ambient, eg. smoke, moving star field		- permenant-ish, maybe persistent but 
	*/

	world_set_scanlines_dirty(world);

}

//-----------------------------------------------------------------------------
navigation_agent_ptr world_acquire_nav_agent(world_ptr world)
{
	level_persistent_ptr level = world_current_level(world);

	navigation_agent_ptr nav_agent = NULL;
	for (uint8_t idx = 0; idx < WORLD_NAVIGATION_AGENT_COUNT; ++idx)
	{
		if (level->nav_agents[idx].entity_id == -1)
		{
			nav_agent = &level->nav_agents[idx];
			break;
		}
	}
	return nav_agent;
}

//	refactor away from path 
//-----------------------------------------------------------------------------
vector2_t world_closestpointonpath(vector2_t point, uint32_t mask)	
{
	vector2_t currentClosestPoint = { 0, 0 }, tmpDist = { 0, 0 };
	fixed16_t currentClosestDist = fixed16_maximum;

	for (uint8_t i = 0; i < g_main_world->ephermeral.numCollision; ++i)
	{
		//	use collision mask 

		vector2_t closestPoint = collisionClosestPointOBB(point, g_main_world->ephermeral.collision[i].box);
		if (collisionCheckPointInsideOBB(point, g_main_world->ephermeral.collision[i].box))
			return point;

		//	can get depth value here ... 

		mathVector2Substract(&tmpDist, &point, &closestPoint);
		fixed16_t dist = mathVector2Length(&tmpDist);
		if (dist < currentClosestDist)
		{
			currentClosestDist = dist;
			mathVector2Copy(&currentClosestPoint, &closestPoint);
		}
	}

	return currentClosestPoint;
}

//	interaction points should be handled differently than trigger volumes
//	on_enter vs on_exit <--- this is only needed for interaction points
	//	is it an interaction point of prox trigger ??
		//	if so, set this as active interaction point on 

//-----------------------------------------------------------------------------
interaction_point_ptr interaction_point_check(world_ptr world, vector2_t point)
{
	//	interaction mask !! 

	//	callbacks for on enter and on exit ... 

	interaction_point_ptr interaction = world->ephermeral.interactions;
	level_persistent_t* level = world_current_level(world);

	for (uint8_t i = 0; i < world->ephermeral.interaction_count; ++i)
	{
		if ((interaction->layer & level->layer) > 0)
		{
			if (collisionCheckPointInsideCircle(point, interaction->circle))
			{
				return interaction;
			}
		}

		//	formalize in some kind of iterator
		uint8_t interaction_size_in_bytes = sizeof(interaction_point_t) + (4 * interaction->option_count);
		uint8_t* ptr = ((uint8_t*)interaction) + interaction_size_in_bytes;
		interaction = (interaction_point_ptr)ptr;
	}

	return NULL;
}


//-----------------------------------------------------------------------------
void trigger_check(world_ptr world, entity_ptr entity)	
{
	//	need to also check all entities, as we may want doors to open for them too 
	//		consider this for accelerating 

	uint32_t trigger_state = world->persistent.trigger_check_state;		

	level_persistent_t* level = world_current_level(world);

	vector2_t point = { entity->position.x, entity->position.z };	//{ world->player->position.x, world->player->position.z };	//	pull this from world player

	for (int32_t i = 0; i < world->ephermeral.trigger_count; ++i)
	{
		trigger_t* trigger = &world->ephermeral.triggers[i];

		uint8_t check = 0;

		//	also apply a filter here so we can do stuff like, if has key, enable bit in filter, 
		//		which will enable trigger group ... 

		if ((trigger->layer & level->layer) > 0)
		{
			check = collisionCheckPointInsideOBB(point, trigger->area);
			if (check)
			{
				if ((trigger_state & (1 << i)) == 0)
				{
					trigger_state |= (1 << i);
					debug_printf(DEBUG_LOG_DEBUG, "world::on_enter %u", i);

					sequence_schedule(&world->ephermeral.sequencer, trigger->on_enter);
				}
			}
		}

		if (!check)
		{
			if ((trigger_state & (1 << i)) > 0)
			{
				trigger_state &= ~(1 << i);
				debug_printf(DEBUG_LOG_DEBUG, "world::on_exit %u", i);

				sequence_schedule(&world->ephermeral.sequencer, trigger->on_exit);
			}
		}
	}

	world->persistent.trigger_check_state = trigger_state;
}

//-----------------------------------------------------------------------------
void world_update(world_ptr world)
{

//--------------------------------------------------------------------------
//		ENTITY UPDATE
//--------------------------------------------------------------------------

	if (world->persistent.player.active)
	{
		player_entity_update(&world->persistent.player);
	}

	level_persistent_ptr level = world_current_level(world);
	entity_ptr entites = level->entities;

	for (uint16_t idx = 0; idx < WORLD_MODEL_ENTITY_COUNT; ++idx)
	{
		model_entity_update(&entites[idx]);		//	this is more of a model / renderer update
	}

	/*
		will need to move player some how
	*/


	for (uint16_t idx = 0; idx < WORLD_NAVIGATION_AGENT_COUNT; ++idx)
	{
		navigation_agent_ptr agent = &level->nav_agents[idx];
		if (agent->entity_id >= 0)
		{
			entity_ptr entity = world_find_entity(world, agent->entity_id);
			navigation_agent_update(agent, entity);
		}
	}

	trigger_check(world, &world->persistent.player.entity);
	for (uint16_t idx = 0; idx < WORLD_MODEL_ENTITY_COUNT; ++idx)
	{
		//trigger_check(world, &entites[idx]);
	}
	
//--------------------------------------------------------------------------
//		SEQUENCER UPDATE
//--------------------------------------------------------------------------

	if (timer_expired(world->ephermeral.sequencer_timer))
	{
		world->ephermeral.sequencer_timer_completion_flag = 1;
	}

	sequence_update(&world->ephermeral.sequencer);


	//	apply loaded levels 

//--------------------------------------------------------------------------
//		AUDIO
//--------------------------------------------------------------------------

	if (world->ephermeral.ambient_resource_id != world->persistent.ambient_resource_id)
	{
		world->ephermeral.ambient_resource_id = world->persistent.ambient_resource_id;
		world->ephermeral.ambient_stream = audio_stream_open(resources_find_audioclip(world->ephermeral.ambient_resource_id), AUDIO_STREAM_AMBIENT);
	}

	if (world->ephermeral.music_resource_id != world->persistent.music_resource_id)
	{
		world->ephermeral.music_resource_id = world->persistent.music_resource_id;
		world->ephermeral.music_stream = audio_stream_open(resources_find_audioclip(world->ephermeral.music_resource_id), AUDIO_STREAM_MUSIC);
	}

//--------------------------------------------------------------------------
//		VIEW UPDATE START 
//--------------------------------------------------------------------------

	uint8_t view_dirty_flags = 0;
	uint8_t target_view_state = level->view_state[world->persistent.view_idx];

	if (world->ephermeral.loaded_view_idx != world->persistent.view_idx) 	//	this is potentially lagging by one frame ?? 
	{
		world_load_view(world, world->persistent.view_idx);

		//	put this in the above func 

		world->ephermeral.view_state = target_view_state;	

		for (uint8_t idx = 0; idx < 8; ++idx)
			world->ephermeral.subview_frame_indices[idx] = 0;

		view_dirty_flags = target_view_state;
	}

	uint8_t current_view_state = world->ephermeral.view_state;

	//	update subviews
		//	how do I know it is animating out or in 

	uint16_t subview_idx = 0;
	subview_t* subview = &world->ephermeral.views[world->persistent.view_idx].subviews[0];

	while (target_view_state | current_view_state)
	{
		if ((target_view_state & 0x01) && !(current_view_state & 0x01))	//	in 
		{
			world->ephermeral.view_state |= (1 << subview_idx);
			current_view_state |= 0x01;
		}

		//	may need to tick a video ??? 

		//	apply video frame to ewram view image 	  --- // vid/test 
		/*if (world->ephermeral.video_handle)
		{
			smk_next(world->ephermeral.video_handle, world->ephermeral.view_image->data, world->ephermeral.view_image->width);
		}*/

		if (current_view_state & 0x01)
		{
			if (target_view_state & 0x01)	//	on
			{
				world->ephermeral.subview_frame_indices[subview_idx]++;
				if (world->ephermeral.subview_frame_indices[subview_idx] > 8)
				{
					world->ephermeral.subview_frame_indices[subview_idx] = 8;
				}
				else
				{
					view_dirty_flags |= (1 << subview_idx);
				}
			}
			else //	out 
			{
				world->ephermeral.subview_frame_indices[subview_idx]--;
				if (world->ephermeral.subview_frame_indices[subview_idx] <= 0)
				{
					world->ephermeral.view_state &= ~(1 << subview_idx);
				}
				else
				{
					view_dirty_flags |= (1 << subview_idx);
				}
			}
		}

		target_view_state >>= 1;
		current_view_state >>= 1;

		subview++; subview_idx++;
	}


	//	apply subview palette

	//palette_ptr pal = resources_find_palette(image->palette_id);
	//graphics_write_palette(pal);		//	this needs to happen on resume, this shouldn't happen every frame ... 


	//	apply subview graphics

	if (view_dirty_flags)
	{
		world_ephemeral_t* ephermeral = &world->ephermeral;

		//	apply dirty subviews 
		subview_idx = 0;
		subview = &world->ephermeral.views[world->persistent.view_idx].subviews[0];
		rendertarget_ptr ewram_background_rendertarget = world->ephermeral.view_rendertarget;

		graphics_bind_rendertarget(ewram_background_rendertarget);

		do
		{
			if (view_dirty_flags & 0x01)
			{
				image_ptr image = resources_find_image(subview->resource_id);
				uint16_t frame_idx = world->ephermeral.subview_frame_indices[subview_idx];
				
				palette_ptr pal = resources_find_palette(image->palette_id);
				graphics_write_palette(pal);		//	this needs to happen on resume, this shouldn't happen every frame ... 

				graphics_draw_image(image, frame_idx, subview->position.x, subview->position.y);

				//	eventually add video support here... 
				
				//	mark as dirty - to world span 
				int16_t start_y = subview->position.y;
				int16_t end_y = subview->position.y + image->height;

				span_encapsulate(&ephermeral->dirty_scanline_span[0], start_y, end_y);
				span_encapsulate(&ephermeral->dirty_scanline_span[1], start_y, end_y);
			}

			view_dirty_flags >>= 1;

			subview++; subview_idx++;

		} while (view_dirty_flags);

		graphics_bind_rendertarget(NULL);
	}

//--------------------------------------------------------------------------
//		VIEW UPDATE END
//--------------------------------------------------------------------------

}

//-----------------------------------------------------------------------------
void world_reload_palettes(world_ptr world)		//	maybe a mark as dirty would be better 
{

}

// implement a graphics Draw Wireframe Circle

// move this somewhere better
//-----------------------------------------------------------------------------
void graphicsDrawWireframeOBB(obb_t* obb, fixed16_t y, matrix4x4_t* wvp)	//	put this code somewhere better
{
	vector2_t offsetX, offsetY;
	mathVector2ScalarMultiply(&offsetX, &obb->axis[0], obb->extents[0]);
	mathVector2ScalarMultiply(&offsetY, &obb->axis[1], obb->extents[1]);

	vector3_t pts[5];

	vector2_t point;

	mathVector2Add(&point, &obb->center, &offsetX);
	mathVector2Add(&point, &point, &offsetY);
	pts[0].x = point.x;
	pts[0].y = y;
	pts[0].z = point.y;

	pts[4].x = point.x;
	pts[4].y = y;
	pts[4].z = point.y;

	mathVector2Add(&point, &obb->center, &offsetX);
	mathVector2Substract(&point, &point, &offsetY);
	pts[1].x = point.x;
	pts[1].y = y;
	pts[1].z = point.y;

	mathVector2Substract(&point, &obb->center, &offsetX);
	mathVector2Substract(&point, &point, &offsetY);
	pts[2].x = point.x;
	pts[2].y = y;
	pts[2].z = point.y;

	mathVector2Substract(&point, &obb->center, &offsetX);
	mathVector2Add(&point, &point, &offsetY);
	pts[3].x = point.x;
	pts[3].y = y;
	pts[3].z = point.y;

	//	more protection in draw line against off screen out of bounds 
	graphics_draw_line(pts, 5, 0, wvp);
}


//-----------------------------------------------------------------------------
void world_draw(world_ptr world)
{
	profiler_sample_handle_t phandle;

	phandle = profiler_begin("w:p_entites");

	//	entity generate draw list 
	entity_ptr entity;
	uint16_t idx = 0;
	uint16_t draw_list_count = 0;

	entity_ptr entity_draw_list[WORLD_MODEL_ENTITY_COUNT];
	fixed16_t entity_distance_list[WORLD_MODEL_ENTITY_COUNT];

	level_persistent_ptr level = world_current_level(world);
	entity_ptr entites = level->entities;

	vector3_t view_position;
	mathVector3Copy(&view_position, &world->ephermeral.views[world->persistent.view_idx].view_position);
	 
	entity_draw_list[draw_list_count++] = &world->persistent.player.entity;
	for (idx = 0; idx < WORLD_MODEL_ENTITY_COUNT; ++idx)
	{
		entity = &entites[idx];

		if (entity->id >= 0 && (entity->layer & level->layer) > 0)
		{
			entity_draw_list[draw_list_count] = entity;
			entity_distance_list[draw_list_count] = mathVector3DistanceSqr(&view_position, &entity->position);

			draw_list_count++;
		}
	}

	//	the sorting here can be the same as the triangle sort 

	/*
		I don't like this code for entity draw order sorting, it feels messy 
	*/

	for (idx = 1; idx < draw_list_count; ++idx)
	{
		if (entity_distance_list[idx - 1] < entity_distance_list[idx])
		{
			fixed16_t tmp = entity_distance_list[idx - 1];
			entity_distance_list[idx - 1] = entity_distance_list[idx];
			entity_distance_list[idx] = tmp;

			entity = entity_draw_list[idx - 1];
			entity_draw_list[idx - 1] = entity_draw_list[idx];
			entity_draw_list[idx] = entity;

			--idx;
		}
	}

	profiler_end(phandle);


	phandle = profiler_begin("w:background");

	//	use a world dirty span here 

	uint32_t start = world->ephermeral.dirty_scanline_span[g_graphics_context.page_flip].start;
	uint32_t count = world->ephermeral.dirty_scanline_span[g_graphics_context.page_flip].end - start;
	uint32_t stride = g_graphics_context.width;

	memory_dma_copy32(
		g_graphics_context.frame_buffer + (start * stride),
		world->ephermeral.view_rendertarget->frame_buffer + (start * stride),
		(count * stride) >> 2);

	graphics_reset_dirty_scanline();

	if (world->ephermeral.active_highlight != NULL)
	{
		fixed16_t scroll_progress = timer_progress(world->ephermeral.highlight_timer);

		int32_t start = fixed16_to_int(fixed16_mul(scroll_progress, fixed16_from_int(world->ephermeral.active_highlight->row_count))) + world->ephermeral.active_highlight->row_start;
		int32_t end = start + 4;

		graphics_draw_highlightfield(world->ephermeral.active_highlight, 64, start, end);	
	}

	profiler_end(phandle);


	phandle = profiler_begin("w:d_entites");

	//	model_entity_draw
	for (idx = 0; idx < draw_list_count; ++idx)
	{
		entity_draw(
			entity_draw_list[idx],
			&world->ephermeral.views[world->persistent.view_idx].wvp);
	}

	if (g_graphics_context.frame_dirty_scanline_span.start < g_graphics_context.frame_dirty_scanline_span.end)
		world->ephermeral.dirty_scanline_span[g_graphics_context.page_flip] = g_graphics_context.frame_dirty_scanline_span;

	profiler_end(phandle);


#if WORLD_DEBUG_DRAW

	fixed16_t ground_y = entity_draw_list[0]->position.y;

	/*	Debug Wireframe drawing of the collision */
	for (uint16_t idx = 0; idx < world->ephermeral.numCollision; ++idx)
	{
		graphicsDrawWireframeOBB(
			&world->ephermeral.collision[idx].box,
			ground_y,
			&world->ephermeral.views[world->persistent.view_idx].wvp);
	}

	/*	Debug Wireframe drawing of the trigger areas */
	level_persistent_ptr current_level = world_current_level(world);
	for (uint16_t idx = 0; idx < world->ephermeral.trigger_count; ++idx)
	{
		trigger_t* trigger = &world->ephermeral.triggers[idx];

		if ((trigger->layer & current_level->layer) > 0)
		{
			graphicsDrawWireframeOBB(
				&trigger->area, 
				ground_y,
				&world->ephermeral.views[world->persistent.view_idx].wvp);
		}
	}

#endif

}

//-----------------------------------------------------------------------------
void world_newgame(world_ptr world, uint16_t resource_id)
{
	/*
		might be good for this to be data-driven eventually 
	*/

	//------------------
	level_persistent_ptr level_data = world_current_level(world);

	level_data->entities[0].id = -1;
	level_data->entities[1].id = -1;

	for (uint16_t idx = 0; idx < WORLD_NAVIGATION_AGENT_COUNT; ++idx)
		level_data->nav_agents[idx].entity_id = -1;


	inventory_reset(&world->persistent.inventory);

	world_load_level(g_main_world, resource_id);


	g_main_world->persistent.view_idx = 1;
	world_load_view(g_main_world, 1);		//	spawn view


	level_ptr level = resources_find_level(resource_id);

	mathVector3Copy(&world->ephermeral.origin, &level->origin);
	mathVector3Copy(&world->ephermeral.scale, &level->scale);

	//-------------------
	player_entity_ptr player = &world->persistent.player;
	player->entity.id = 0;
	player->entity.layer = ~0;
	player->entity.model_resource_id = resources_find_index_of("mdl/test");
	player->entity.frame = 0;
	mathVector3MakeFromElements(&world->persistent.player.forwards, F16(0), F16(0), F16(0));
	player->active = 1;
	//	some function to apply spawn would be useful 

	//	this should be a different function !!! 
	mathVector3Copy(&player->entity.position, &world->ephermeral.spawns[0].position);
	mathVector3MakeFromElements(&player->entity.rotation, fixed16_zero, world->ephermeral.spawns[0].yaw, fixed16_zero);
	//--------------------

	//	this is the problem !!! 
	if (level->sequence_store_id >= 0)
	{
		sequence_store_ptr store = resources_find_sequencestore(level->sequence_store_id);	//	this need to come from somewhere sensible
		world->ephermeral.sequencer.store = store;
		sequence_schedule(&world->ephermeral.sequencer, level->on_load_sequence);	//	should this be based on spawn point ? 
	}
}

//-----------------------------------------------------------------------------
void world_savegame(world_ptr world, tiledimage_ptr preview_image, savegame_ptr slot)
{
	savegame_set_open(slot);
	{
		level_ptr level = resources_find_level(world->persistent.level_resource_id);
		savegame_write_header(slot, level->name);

		uint8_t* dest = slot->data;
		memory_copy8(dest, &world->persistent, sizeof(world_persistent_t));

		dest = slot->data + sizeof(world_persistent_t);

		uint32_t preview_image_size_in_bytes = tiledimage_size_in_bytes(preview_image);

		memory_copy8(dest, preview_image, preview_image_size_in_bytes);
	}
	savegame_set_close(slot);
}

//-----------------------------------------------------------------------------
void world_loadgame(world_ptr world, savegame_ptr slot)
{
	memory_copy8(&world->persistent, slot->data, sizeof(world_persistent_t));

	world_load_level(g_main_world, world ->persistent.level_resource_id);
}

//-----------------------------------------------------------------------------
void world_savegame_load_preview_image(savegame_ptr slot, tiledimage_ptr preview_image)
{
	debug_assert(preview_image != NULL, "");

	uint32_t preview_image_size_in_bytes = tiledimage_size_in_bytes(preview_image);

	uint8_t* src = slot->data + sizeof(world_persistent_t);
	memory_copy8(preview_image, src, preview_image_size_in_bytes);
}
