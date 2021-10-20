
#ifndef CAMERA_H
#define CAMERA_H


#include "common/math/matrix.h"

typedef struct cameraSetup_s	//	
{
	vector3_t position;
	fixed16_t pitch, yaw;				//	surely only need 2 degrees of freedom !! 
	fixed16_t fov;
} cameraSetup_t;

typedef struct camera_s
{
	vector3_t position;
	fixed16_t pitch, yaw;				//	surely only need 2 degrees of freedom !! 
	fixed16_t fov;						
	fixed16_t nearPlane, farPlane;

	matrix4x4_t model;
	matrix4x4_t projection;

} camera_t;


void camera_build(camera_t* camera);

void cameraInitialize(camera_t* camera);

void camera_translate_local(camera_t* camera, vector3_t* local_position);

void camera_translate_global(camera_t* camera, vector3_t* global_position);

void cameraRotate(camera_t* camera, fixed16_t pitch, fixed16_t yaw);


#endif // !CAMERA_H
