
#include "navigation.h"

#include "world.h"

#include "common/containers/list.h"


//-----------------------------------------------------------------------------
void navigation_generate_path(/*world_ptr world, */vector2_t origin, vector2_t destination, path_t* path)
{
	/*
		Reduce Memory Usage here
		Optimize
		Stablize against edge cases 
	 */

	//	need to know world !!! 

	//g_main_world->collision;

	int8_t origin_node_idx = -1, destination_node_idx = -1;

	for (uint8_t idx = 0; idx < g_main_world->ephermeral.numCollision; ++idx)
	{
		if (collisionCheckPointInsideOBB(origin, g_main_world->ephermeral.collision[idx].box))
		{
			origin_node_idx = idx;
			break;
		}
	}

	for (uint8_t idx = 0; idx < g_main_world->ephermeral.numCollision; ++idx)
	{
		if (collisionCheckPointInsideOBB(destination, g_main_world->ephermeral.collision[idx].box))
		{
			destination_node_idx = idx;
			break;
		}
	}


	/*
		AStar over the obb_t collision nodes
			This should be its own function 
	*/

	uint8_t* score_map = memory_allocate(sizeof(uint8_t) * g_main_world->ephermeral.numCollision, MEMORY_EWRAM);
	int16_t* came_from_map = memory_allocate(sizeof(int16_t) * g_main_world->ephermeral.numCollision, MEMORY_EWRAM);

	debug_assert(score_map != NULL, "navigation::generate_path score_map == NULL");
	debug_assert(came_from_map != NULL, "navigation::generate_path came_from_map == NULL");
	
	for (uint8_t idx = 0; idx < g_main_world->ephermeral.numCollision; ++idx)
		came_from_map[idx] = -1;

	int16_list_t open_list;
	int16_list_t closed_list;
	int16_list_t path_list;

	list_new(&open_list, 32, MEMORY_EWRAM);	//	fixed size 
	list_new(&closed_list, 32, MEMORY_EWRAM);
	list_new(&path_list, 32, MEMORY_EWRAM);

	list_append(&open_list, origin_node_idx);
	score_map[origin_node_idx] = 0;

	while (open_list.length) 
	{
		int8_t current_node_idx = open_list.data[0];
		list_remove_at_index(&open_list, 0);
		list_append(&closed_list, current_node_idx);

		if (current_node_idx == destination_node_idx)	
		{
			uint8_t temp_node_idx = current_node_idx;
			list_append(&path_list, temp_node_idx);

			while (came_from_map[temp_node_idx] != -1)
			{
				temp_node_idx = came_from_map[temp_node_idx];
				list_append(&path_list, temp_node_idx);
			}
			list_reverse(&path_list);
			break;
		}

		/* find first link node */
		uint8_t link_idx = 0;
		for (link_idx = 0; link_idx < g_main_world->ephermeral.numNavLinks; ++link_idx)
		{
			if (g_main_world->ephermeral.nav_links[link_idx].node_idx_a == current_node_idx)
				break;
		}

		for (; link_idx < g_main_world->ephermeral.numNavLinks; ++link_idx)
		{
			if (g_main_world->ephermeral.nav_links[link_idx].node_idx_a != current_node_idx)
				break;

			uint8_t neighbor_idx = g_main_world->ephermeral.nav_links[link_idx].node_idx_b;

			if (list_index_of(&closed_list, neighbor_idx) >= 0)
				continue;

			uint8_t score = score_map[current_node_idx] + 1;

			uint8_t exists_in_open_list = list_index_of(&open_list, neighbor_idx) >= 0;
			if (exists_in_open_list == FALSE ||
				score < score_map[neighbor_idx])
			{
				came_from_map[neighbor_idx] = current_node_idx;

				score_map[neighbor_idx] = score;

				if (exists_in_open_list == FALSE)
					list_append(&open_list, neighbor_idx);
			}
		}
	}

	list_delete(&open_list);
	list_delete(&closed_list);

	memory_free(score_map);
	memory_free(came_from_map);


	// convert to detailed path 
	//path_t path; 
	if (path_list.length >= 31)
	{
		debug_printf(DEBUG_LOG_FATAL, "navigation path_list is greater than 32 point");
		path->waypoint_count = 0;
		return;
	}

	vector2_t* detailed_path = &path->waypoints[0];
	uint8_t detailed_path_idx = 0;

	mathVector2Copy(&detailed_path[detailed_path_idx++], &origin);
	for (uint8_t idx = 0; idx < path_list.length - 1; ++idx)
	{
		vector2_t target_point;
		if (idx + 2 == path_list.length)
		{
			mathVector2Copy(&target_point, &destination);
		}
		else
		{
			mathVector2Copy(&target_point, &g_main_world->ephermeral.collision[path_list.data[idx + 2]].box.center);
		}

		uint16_t current_node_idx = path_list.data[idx];
		uint16_t next_node_idx = path_list.data[idx + 1];

		uint8_t link_idx = 0;
		line_segment_t* line_segment = NULL;
		for (link_idx = 0; link_idx < g_main_world->ephermeral.numNavLinks; ++link_idx)
		{
			navigation_link_t* link = &g_main_world->ephermeral.nav_links[link_idx];
			if (link->node_idx_a == current_node_idx &&
				link->node_idx_b == next_node_idx)
			{
				uint8_t link_data_idx = link->link_data_idx;
				line_segment = &g_main_world->ephermeral.nav_link_data[link_data_idx].line_of_intersection;
				break;
			}
		}

		vector2_t waypoint = collisionClosestPointLineSegment(target_point, line_segment);
		mathVector2Copy(&detailed_path[detailed_path_idx++], &waypoint);
	}
	mathVector2Copy(&detailed_path[detailed_path_idx++], &destination);
	//	probably need to validate that end point is within collision bounds

	path->waypoint_count = detailed_path_idx;

	list_delete(&path_list);
}

//-----------------------------------------------------------------------------
void navigation_clear_path(navigation_agent_ptr agent)
{

}

//-----------------------------------------------------------------------------
void navigation_agent_update(navigation_agent_ptr agent, entity_ptr entity)	//	will need access to world when dynamic generated path
{
	entity->position.x += agent->step_vector.x;
	entity->position.z += agent->step_vector.y;

	//	should be making sure they are animating !!! 

	//	target rotation only needs to be done when switching waypoint ... 

	entity->rotation.x = fixed16_zero;
	entity->rotation.y = fixed16_arctangent2(agent->step_vector.x, agent->step_vector.y);	//	smooth transition for changes, maybe slerp 
	entity->rotation.z = fixed16_zero;


	// consistant velocity, don't equally subdivide, mybe don't take the subdivide approach


	if (++agent->step_idx >= agent->step_count)	
	{
		//	move to next waypoint ? 
		if (++agent->waypoint_idx >= agent->path.waypoint_count)
		{
			agent->entity_id = -1;
			agent->completion_flag = 1;
			//	path complete
			agent->step_idx = 0;
			agent->step_count = 32;

			return;
		}

		vector2_t origin = agent->path.waypoints[agent->waypoint_idx - 1];
		vector2_t destination = agent->path.waypoints[agent->waypoint_idx];
		vector2_t delta;

		mathVector2Substract(&delta, &destination, &origin);
		//	find magnitude of delta  / divide it by the step size
		//		this will require sqr :-( 

		mathVector2ScalarMultiply(&agent->step_vector, &delta, fixed16_one >> 5);
	
		agent->step_idx = 0;
		agent->step_count = 32;
	}
}

