
#ifndef ENTITY_H
#define ENTITY_H


#include "common/math/fixed16.h"
#include "common/math/matrix.h"
#include "common/math/vector.h"
#include "common/math/point.h"

#include "common/graphics/model.h"
#include "common/collision/collision.h"

//	maybe this is common ??? 

//	this is definitely a render_ent, for the render world

//	actor entity, this could contain a render entity ?? 
//		player, npc

//	entity for trigger ?? or just trigger obj ?? 



/*
	entities are things that can be created in the world
	 
		- player_entity 
		- model_entity

		- navigation_agent (is its own thing)
		- ground (is it's own thing, alot of baking goes into this)

		- inventory

*/


typedef struct entity_s		
{
	uint16_t id;	//	is this needed 

	//	ent type
	//		is renderable, is interaction point

	uint8_t layer;		// depth layer
	 
	vector3_t position;		//	further data for lerping
	vector3_t rotation;

	vector3_t previous_position;
	vector3_t target_rotation;	//	so we can lerp towards the desired rotation

	//	fixed16_t velocity, as we move we tween upto appropriate velocity (non-directional, so we don't drift?)

	uint16_t model_resource_id;
	int16_t animation_id;
	fixed16_t frame_precise;		//	allows for fractional frame progression 

	int16_t frame_count;
	
	uint8_t update_func_id;	//	this whole serialize... --- no wait, it will 
	//	maybe this should be an id 
		//	player_update
		//	npc_update 

	//	other render parameters

} entity_t;

typedef entity_t* entity_ptr;


typedef void (*entity_update_func_ptr)(entity_ptr entity);


void model_entity_update(entity_ptr entity);

void entity_draw(entity_ptr entity, matrix4x4_t* wvp);


typedef struct player_entity_s
{
	entity_t entity;

	uint8_t active;
	uint8_t reserved[3];

	vector3_t forwards;

} player_entity_t;

typedef player_entity_t* player_entity_ptr;


void player_entity_update(player_entity_ptr player);

void animation_update(entity_ptr entity);

//	enemy brain 
//		enters certain areas, and pursues main person 


#endif
