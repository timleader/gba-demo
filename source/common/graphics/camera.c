
#include "common/graphics/camera.h"

void camera_build(camera_t* camera)
{
	vector3_t rotation;
	matrix4x4_t T, R;

	mathMatrix4x4MakeTranslation(&T, &camera->position);

	mathVector3MakeFromElements(&rotation, camera->pitch, camera->yaw, fixed16_zero);
	mathMatrix4x4MakeRotationZYX(&R, &rotation);

	mathMatrix4x4Multiply(&camera->model, &T, &R);

	mathMatrix4x4MakePerspective(&camera->projection, camera->fov, F16(240.0 / 160.0), camera->nearPlane, camera->farPlane);		// this seems to checkout with sr
}


void cameraInitialize(camera_t * camera)
{
	const fixed16_t deg2rad = 0x00000478;		//	F16(PI / 180)

	mathVector3MakeFromElements(&camera->position, fixed16_zero, fixed16_zero, fixed16_zero);
	camera->pitch = fixed16_zero;
	camera->yaw = fixed16_zero;

	camera->fov = fixed16_mul(0x003C0000, deg2rad);	//	60 degrees

	camera->nearPlane = F16(0.1);
	camera->farPlane = F16(1000.0);

	camera_build(camera);
}

void camera_translate_local(camera_t * camera, vector3_t * local_position)
{
	mathVector3Copy(&camera->position, local_position);
}

void camera_translate_global(camera_t * camera, vector3_t * global_position)
{
	mathVector3Copy(&camera->position, global_position);
}

void cameraRotate(camera_t * camera, fixed16_t pitch, fixed16_t yaw)
{
	camera->pitch = pitch;
	camera->yaw = yaw;
}

