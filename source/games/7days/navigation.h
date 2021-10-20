
#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "level.h"

typedef struct navigation_agent_s
{
	int16_t entity_id;
	uint16_t waypoint_idx;

	path_t path;


	vector2_t step_vector;

	uint16_t step_idx;
	uint16_t step_count;


	fixed16_t velocity;

	int8_t completion_flag;

} navigation_agent_t;

typedef navigation_agent_t* navigation_agent_ptr;


/*
	input:
		destination 
	output: 
		path
*/
void navigation_generate_path(/*world_ptr world, */vector2_t origin, vector2_t destination, path_t* path);

void navigation_clear_path(navigation_agent_ptr agent);

void navigation_agent_update(navigation_agent_ptr agent, entity_ptr entity);	//	will need access to world when dynamic generated path


/*
	pre-baked paths or runtime generated ??? 

	have a mesh of which collision ground obb are connected, 

	lets start with pre-baked for cutscenes ... 

	might need runtime generated for main boss.. 

*/


#endif

