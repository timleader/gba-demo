
#include "games/7days/entities/entity.h"

#include "common/debug/debug.h"

//	movement has already been applied, this is just a presentation layer 

//	character animation update 
void animation_update(entity_ptr entity)		//	apply this to player and npc
{
	vector3_t displacement;
	mathVector3Substract(&displacement, &entity->position, &entity->previous_position);
	
	fixed16_t displacement_length = mathVector3Length(&displacement);

	//	dot_product , forwards vs displacement 

	//	rotational displacement 

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


	//	int16_t animation_controller(entity_ptr entity, vector3_t displacement)  // returns animation_id 

	///	above is specific to the model type, eg. characters 





	//	below is common 

	// check for animation_events, this might want us to play a sfx 

	if (animation_id != entity->animation_id)
	{
		entity->animation_id = animation_id;

		animation_t* animation = model_find_animation(model, entity->animation_id);

		debug_assert(animation, "animation could not be found");

		entity->frame_precise = fixed16_zero;
		entity->frame_count = animation->frame_count;
	}
	else
	{
		//	some animations might want to progress over time, instead of over displacement !!! 
		//		animation should probably define if they are time or displacement based update 
		//		animation should state it's update_rate (for both time and displacement) 

		uint8_t vblank_count = graphics_get_vsync();
		fixed16_t frames_per_second = fixed16_div(fixed16_from_int(60), (fixed16_from_int(vblank_count)));
		fixed16_t frame_rate = fixed16_div(fixed16_one, frames_per_second);

		//	frame per meter -> should be pull from animation selection and stored in entity  
		fixed16_t displacement_per_second = fixed16_one;		//	source this from somewhere sensible 
		fixed16_t displacement_per_frame = fixed16_mul(displacement_per_second, frame_rate);


		fixed16_t frame_delta = fixed16_div(displacement_length, displacement_per_frame);		//	need to factor in frame rate 

		entity->frame_precise += frame_delta;

		//	forwards or backwards frame progression, depending on the dot of the forwards vs displacement
		if (fixed16_to_int(entity->frame_precise) >= entity->frame_count)
			entity->frame_precise = fixed16_zero;
	}

	mathVector3Copy(&entity->previous_position, &entity->position);
}
