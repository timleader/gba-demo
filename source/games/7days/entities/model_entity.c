
#include "games/7days/entities/entity.h"

#include "common/graphics/graphics.h"
#include "common/resources/resources.h"
#include "common/memory.h"

//-----------------------------------------------------------------------------
void model_entity_update(entity_ptr entity)
{
	animation_update(entity);		// only appropriate for avatars 
}

//-----------------------------------------------------------------------------
void entity_draw(entity_ptr entity, matrix4x4_t* wvp)
{
	matrix4x4_t M, R;

	mathMatrix4x4MakeTranslation(&M, &entity->position);
	mathMatrix4x4MakeRotationZYX(&R, &entity->rotation);
	mathMatrix4x4Multiply(&M, &M, &R);

	mathMatrix4x4Multiply(&M, wvp, &M);

	model_ptr model = resources_find_model(entity->model_resource_id);

	image_ptr img = resources_find_image(model->image_id);
	graphics_bind_image(img);

	//	move this palette write to only happen once 
	palette_ptr pal = resources_find_palette(img->palette_id);
	graphics_write_palette(pal);	//	this can't happen here, due to potential conflicts 

	graphics_draw_model(model, entity->animation_id, fixed16_to_int(entity->frame_precise), &M);
}

