
#ifndef COLLISION_H
#define COLLISION_H


#include "common/math/vector.h"

//	collision world, //	collision world can just be world
//		path
//		triggers
//		
//		structure for accelerated collision detection

//		entities should exist in this collision world, so upon detection we can tie it back to something

typedef struct obb_s
{
	vector2_t center;
	vector2_t axis[2];
	fixed16_t extents[2];

	//	height[2]	// only along one axis	//	this shouldn't be part of the collision sys

} obb_t;

typedef struct circle_s		//	scrap circles and just us obb for triggers ??? or create a collider obj that supports either 
{
	vector2_t center;
	fixed16_t radius;

} circle_t;

typedef struct line_segment_s
{
	vector2_t origin;
	vector2_t vector;

} line_segment_t;

vector2_t collisionClosestPointLineSegment(vector2_t point, line_segment_t* line);

int collisionCheckPointInsideCircle(vector2_t point, circle_t circle);

int collisionCheckPointInsideOBB(vector2_t point, obb_t obb);

vector2_t collisionClosestPointOBB(vector2_t point, obb_t obb);


//	collisionWorldClosestPointOnPath(vector2_t point)


//	this could contains a collision world and have functions with world wide checks

//	Create Debug Visualizer view - generate meshes / models from obb 

//	Check all OBB in scene for now
//	
//	player wants to move to x run it through collisionClosestPointOBB and use the result as the player's new position.
/*
	//	could chain obb together and just check current and next obbs ( not all )


	foreach obb in world
		closest, inside = closestPointOBB(point, obb)
		if (inside)
			return closest
		if (closest < current_closest)
			current_closest = closest
	return current_closest

*/




#endif // !COLLISION_H