
#include "games/7days/entities/entity.h"

#include "common/debug/debug.h"


//	character animation update 
void animation_update(entity_ptr entity)		//	apply this to player and npc
{
	vector3_t displacement;
	mathVector3Substract(&displacement, &entity->position, &entity->previous_position);
	
	fixed16_t displacement_length = mathVector3Length(&displacement);

	//	dot_product , forwards vs displacement 

	//	this needs to be in an animation_controller func set 

	// Blending between animations ??? this would be a fair bit of cpu overhead, that might be needed elsewhere ... 


	//	State machine would make sense here, as then we can trigger a idle twitch if in idle, etc.. 
	//		could do states via just having a update func ptr that we switch ... 

	//	how do we factor in sitting / interacting 

	if (entity->model_resource_id == 0)
		return;


	model_ptr model = resources_find_model(entity->model_resource_id);
	int16_t animation_id = entity->animation_id;
	if (displacement_length > F16(0.1))
	{
		animation_id = model_find_animation_index_of(model, "walk");	//"run"	don't use strings 
	}
	else if (displacement_length > F16(0.001))
	{
		animation_id = model_find_animation_index_of(model, "walk");	//	don't use strings 

		//	displacement controls the progression of the frames ... 

		//	should we use creep ... ??? 
	}
	else
	{
		animation_id = model_find_animation_index_of(model, "idle");

		//	idle - twitch, 

		//	check for rotational displacement ---> "turning"
	}

	// check for animation_events, this might want us to play a sfx 

	if (animation_id != entity->animation_id)
	{
		entity->animation_id = animation_id;

		animation_t* animation = model_find_animation(model, entity->animation_id);

		debug_assert(animation, "animation could not be found");

		entity->frame = 0;
		entity->frame_count = animation->frame_count;
	}
	else
	{
		//	forwards or backwards frame progression, depending on the dot of the forwards vs displacement
		if (++entity->frame >= entity->frame_count)
			entity->frame = 0;
	}

	mathVector3Copy(&entity->previous_position, &entity->position);
}
//  animation update ( entity, displacement ) 
//		character animation controller 

