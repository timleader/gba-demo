
#ifndef SCENE_H
#define SCENE_H


#include "common/graphics/camera.h"
#include "common/collision/collision.h"
#include "common/math/point.h"

#include "games/7days/entities/entity.h"

/*
	For authoring you need to be able to trigger an animation and start capturing the video
		then reset ready for the next captures ...
*/

//-----------------------------------------------------------------------------
typedef struct subview_s
{
	uint16_t resource_id;	//	{ video, image } 
	uint16_t attributes;	//	{ type }, { once, ping_pong, loop } 

	point2_t position;
	//	size - can be implied from the resource 

} subview_t;

//-----------------------------------------------------------------------------
typedef struct view_s
{
	//	need to know if the view requires depth checks ??? 

	int16_t image_id;
	int16_t depthmap_id;		//	2 bits per pixel

	//int16_t alt_palettes[2];

	//	have an attribute field to flag if this view requires depth evaluation 

	vector3_t view_position;
	vector3_t view_forwards;

	matrix4x4_t wvp;

	subview_t subviews[8];	//	4 might be enough

} view_t;

//-----------------------------------------------------------------------------
typedef struct trigger_s // prebaked is any part of these mutable ?? 
{
	//	filter could get compared with player bitmask to determine if it should be active ?? 

	//	filter as well !!! or should we just have an enable / disable flag

	uint16_t layer;	//	so if i'm in view x only check for view x filters	<<-------- how is this going to work
	uint16_t reserved;		//	attributes 

	int16_t on_enter;	//	<--- this should be sequence id !!! 
	int16_t on_exit;
	
	obb_t area;

} trigger_t;

//-----------------------------------------------------------------------------
typedef struct collider_s
{
	uint16_t layer;
	uint16_t reserved;

	uint32_t view_depth_values;	//	16 views x 2-bit depth value => ((view_depth_values >> (view_idx << 1)) & 0x03)

	obb_t box;

} collider_t;

//-----------------------------------------------------------------------------
typedef struct interaction_point_s
{
	uint16_t layer;	//	so if i'm in view x only check for view x filters	<<-------- how is this going to work

	int16_t highlight_field_resource_id;		//	this would imply interaction points are limited to one view ... 

	circle_t circle;

	uint16_t name_text_id;
	//	maybe some kind of filter
	uint16_t option_count;
	 
	struct {
		uint8_t item_id;
		uint8_t reserved;

		int16_t sequence_id;
	} options[0];

} interaction_point_t;

typedef interaction_point_t* interaction_point_ptr;

// interaction_point_next();

//-----------------------------------------------------------------------------
typedef struct spawn_s		//	probably not needed until we start transitioning between scenes
{
	vector3_t position;
	fixed16_t yaw;

	uint16_t viewIndex;
	uint16_t reserved;

} spawn_t;

//-----------------------------------------------------------------------------
typedef struct path_s
{
	uint16_t waypoint_count;
	vector2_t waypoints[32];

} path_t;

//-----------------------------------------------------------------------------
typedef struct navigation_link_data_s
{
	line_segment_t line_of_intersection;

} navigation_link_data_t;

//-----------------------------------------------------------------------------
typedef struct navigation_link_s
{
	uint16_t node_idx_a;
	uint16_t node_idx_b;

	uint16_t link_data_idx;
	uint16_t reserved;

} navigation_link_t;

//-----------------------------------------------------------------------------
typedef struct level_s	
{
	uint16_t level_idx;
	int16_t sequence_store_id;

	char name[12];
	
	int16_t on_load_sequence;
	uint16_t reserved_0;

	vector3_t origin;
	vector3_t scale;

	uint8_t numView;
	uint8_t numCollision;
	uint8_t numNavigationLink;
	uint8_t numNavigationLinkData;

	uint8_t numTrigger;
	uint8_t numInteraction;
	uint8_t numSpawn;

	uint8_t reserved_1;


	uint32_t viewPtrOffset;
	uint32_t collisionPtrOffset;
	uint32_t navigationLinkPtrOffset;
	uint32_t navigationLinkDataPtrOffset;
	uint32_t triggerPtrOffset;
	uint32_t interactionPtrOffset;
	uint32_t spawnPtrOffset;

} level_t;

typedef level_t* level_ptr;


#endif
