
#ifndef ENEMY_AI_H
#define ENEMY_AI_H

#include "level.h"

//	perhaps have this fit some kind of  entity_ai interface
//		throw all of this in an `ai` folder

typedef struct entity_ai_s	//	enemy, player, follow, dummy, etc...
{
	//	entity_id

	//	ai_state  
	//  40 byte limit 

	//	ai_type => function pointer to update function	//	needs to be persistent

} entity_ai_t;


//-----------------------------------------------------------------------------
typedef struct enemy_ai_state_s
{
	//	state
	
	//	last known player position
	//	time since last seen player

	//	patrol waypoints ?

} enemy_ai_state_t;

typedef enemy_ai_state_t* enemy_ai_state_ptr;


//-----------------------------------------------------------------------------
void enemy_ai_update(entity_ptr entity);

/*
	hidden ...
		not yet in the level

	patrol ...
		follow predefined waypoints  
		check for line of sight to player - perhaps use navigation data to determine this. 
		
	pursue the player ... (hunting)
		check for losing sight of player
		if in range attack player
		if out of sight for a certain amount of time, return to patrol

	flee ...

	should all reset between levels
	    

	generate sounds ... 
	
*/

#endif 