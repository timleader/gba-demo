
#ifndef MODEL_H
#define MODEL_H


#include "common/math/vector.h"

//---------------------------------

//	denormalize texture coordinates eg, 0 -> 1 => 0 -> 128

//	triangle list would reduce vertex projection cost

typedef struct packed_vector2_s		//	unpack within the draw -- frame select
{
	uint8_t x, y;
} packed_vector2_t;

typedef struct packed_vector3_s
{
	uint8_t x, y, z;

	uint8_t reserved;

} packed_vector3_t;

typedef struct packed_vertex_s
{
	packed_vector2_t uv;

} packed_vertex_t;

typedef struct packed_polygon_s		//	polygons not polygons 
{
	uint16_t v[4];				//	look at reducing to uint8_t, this is an index 
	uint8_t color_idx;			//		- can probably remove this !!! 
	uint8_t vertex_count;		//		- quantizized face normal, for culling purposes ??? - no we should have a submodel construct

} packed_polygon_t;


typedef struct animation_s		//	using this will allow us to store named animations in here ... 
{
	char name[12];
	uint32_t frame_count;

	packed_vector3_t frames[0];

} animation_t;


typedef struct animation_event_s
{
	uint16_t frame_idx;
	//	event id

} animation_event_t;


//	refactor to preload_model_s
typedef struct packed_model_s	//	I don't think gba can deal with 1 byte alignment, use 2 byte aligment
{ 
	uint16_t flags;		//	eg. pre-transformed
	uint16_t imageID;

	vector3_t origin;
	vector3_t scale;

	uint16_t animation_count;
	uint16_t vertex_count;
	uint16_t polygon_count;
	uint16_t reserved;

	uint32_t animationPtrOffset;

	uint32_t verticesPtrOffset;

	uint32_t trianglePtrOffset;	//	this is pointer but to where

} packed_model_t;



typedef struct model_s
{
	//	id
	uint8_t flags;			//	eg. pre-transformed
	uint16_t image_id;		//	this removes need to bind image... 

	vector3_t origin;
	vector3_t scale;

	uint16_t animation_count;
	animation_t* animations;		//	vertex positions for each animation frame, 	CART | EWRAM    -- CART has the precache so this might actually be more efficient

	uint16_t vertices_count;
	packed_vertex_t* vertices;		//	stores the UVs as there don't change during an animation

	uint16_t polygon_count;
	packed_polygon_t* polygons;	//	index buffer 

} model_t;

typedef model_t* model_ptr;


#define inline __inline

inline animation_t* model_find_animation(model_ptr model, uint16_t id)
{
	uint16_t animation_clip_size = sizeof(animation_t) + (model->vertices_count * sizeof(packed_vector3_t));

	uint8_t* animations_byte_ptr = (uint8_t*)model->animations;
	return (animation_t*)(animations_byte_ptr + (animation_clip_size * id));
}

animation_t* model_find_animation_from_name(model_ptr model, const char* name);

int16_t model_find_animation_index_of(model_ptr model, const char* name);

#endif // !MODEL_H

