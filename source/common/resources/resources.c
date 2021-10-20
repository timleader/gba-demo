
#include "resources.h"

const char* resource_type_names[] =	
{
	"PAL",
	"IMG",
	"TMG",
	"DMP",
	"SND",
	"VID",
	"MDL",
	"STR",

	"LVL",
	"DAG",
	"SEQ",
	"ITS",

	"HIF",

	"UNK"
};

/*

input system, is input going to fill laggy if we don't poll this at 60Hz

	up
	down
	left
	right

	a,
	b,

	L,
	R,

	select,
	start

 */


model_t loadedModels[8];	// this needs sorting 

model_t* resources_find_model(uint16_t id)	//	put these functions within the appropriate files
{
	debug_assert(resources[id].type == RESOURCE_TYPE_MODEL, "resources::find_model type != RESOURCE_TYPE_MODEL");

	packed_model_t* pModel = (packed_model_t*)(resource_base + resources[id].offset);
	uint8_t * ptr = (uint8_t*)pModel;

	//	load packed model into model
	loadedModels[0].flags = pModel->flags;
	loadedModels[0].image_id = pModel->imageID;

	mathVector3Copy(&loadedModels[0].origin, &pModel->origin);
	mathVector3Copy(&loadedModels[0].scale, &pModel->scale);

	loadedModels[0].animation_count = pModel->animation_count;
	loadedModels[0].animations = (animation_t*)(ptr + pModel->animationPtrOffset);

	loadedModels[0].vertices_count = pModel->vertex_count;
	loadedModels[0].vertices = (packed_vertex_t*)(ptr + pModel->verticesPtrOffset);

	loadedModels[0].polygon_count = pModel->polygon_count;
	loadedModels[0].polygons = (packed_polygon_t*)(ptr + pModel->trianglePtrOffset);

	return &loadedModels[0];
}

level_t * resources_find_level(uint16_t id)
{
	debug_assert(resources[id].type == RESOURCE_TYPE_LEVEL, "resources::find_level type != RESOURCE_TYPE_LEVEL");

	level_t* pScene = (level_t*)(resource_base + resources[id].offset);
	return pScene;
}