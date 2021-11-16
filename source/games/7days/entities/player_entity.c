
#include "games/7days/entities/entity.h"
#include "games/7days/world.h"

#include "common/input/input.h"


/*

	idle

	walk

	run

	sit

	interact 



	- Need to know the bounds of each animation, so we can cycle them correctly 

	- Blending between animations ??? 

*/


//-----------------------------------------------------------------------------
void player_entity_update(player_entity_ptr player)
{
	/* Where should input be processed???  */

	entity_ptr entity = &player->entity;

	const fixed16_t deg2rad = 0x00000478;		//	F16(PI / 180)
	const fixed16_t speed = fixed16_one << 3;

	//	movement velocity..
	vector3_t delta;
	mathVector3ScalarMultiply(&delta, &player->forwards, fixed16_one >> 4);

	vector3_t displacement = { fixed16_zero, fixed16_zero, fixed16_zero };

	if (key_is_down(KI_B))
	{
		mathVector3ScalarMultiply(&delta, &delta, fixed16_one << 1);
	}


	if (key_is_down(KI_UP))
	{
		mathVector3Add(&displacement, &displacement, &delta);
	}

	if (key_is_down(KI_DOWN))
	{
		mathVector3Substract(&displacement, &displacement, &delta);
	}




	if (key_is_down(KI_LEFT))
	{
		entity->rotation.y -= fixed16_mul(speed, deg2rad);
	}

	if (key_is_down(KI_RIGHT))
	{
		entity->rotation.y += fixed16_mul(speed, deg2rad);
	}

	player->forwards.x = fixed16_sine(entity->rotation.y);
	player->forwards.z = fixed16_cosine(entity->rotation.y);	//	I don't think this makes sense I think it should be -

	mathVector3Add(&entity->position, &entity->position, &displacement);
	vector2_t point = { entity->position.x, entity->position.z };

	vector2_t currentClosestPoint = world_closestpointonpath(point, 0);

	// actual displacement can only really be figured out here !!! 

	entity->position.x = currentClosestPoint.x;
	entity->position.z = currentClosestPoint.y;



	animation_update(entity);
}
