
#include "collision.h"

//-----------------------------------------------------------------------------
vector2_t collisionClosestPointLineSegment(vector2_t point, line_segment_t* line)
{
	vector2_t ap;
	mathVector2Substract(&ap, &point, &line->origin);

	fixed16_t magnitudeSqr = mathVector2LengthSqr(&line->vector);
	fixed16_t t = fixed16_mul(ap.x, line->vector.x) + fixed16_mul(ap.y, line->vector.y);

	t = fixed16_div(t, magnitudeSqr);
	t = fixed16_clamp(t, fixed16_zero, fixed16_one);

	vector2_t result;
	mathVector2ScalarMultiply(&result, &line->vector, t);

	mathVector2Add(&result, &result, &line->origin);

	return result;
}

//-----------------------------------------------------------------------------
int collisionCheckPointInsideCircle(vector2_t point, circle_t circle)
{
	fixed16_t xdiff = circle.center.x - point.x;
	fixed16_t ydiff = circle.center.y - point.y;

	fixed16_t distSq = fixed16_mul(xdiff, xdiff) + fixed16_mul(ydiff, ydiff);

	if (distSq < fixed16_mul(circle.radius, circle.radius))
		return 1;
	else
		return 0;
}

//-----------------------------------------------------------------------------
int collisionCheckPointInsideOBB(vector2_t point, obb_t obb)
{
	vector2_t d;
	fixed16_t dist;

	mathVector2Substract(&d, &point, &obb.center);

	for (int i = 0; i < 2; ++i)
	{
		dist = mathVector2DotProduct(&d, &obb.axis[i]);

		if (dist > obb.extents[i]) return 0;
		if (dist < -obb.extents[i]) return 0;

	}
	return 1;
}

//-----------------------------------------------------------------------------
vector2_t collisionClosestPointOBB(vector2_t point, obb_t obb)
{
	vector2_t result = { fixed16_zero, fixed16_zero };

	vector2_t d, t;
	fixed16_t dist;

	mathVector2Copy(&result, &obb.center);

	mathVector2Substract(&d, &point, &obb.center);

	for (int i = 0; i < 2; ++i)
	{
		dist = mathVector2DotProduct(&d, &obb.axis[i]);

		if (dist > obb.extents[i]) dist = obb.extents[i];
		if (dist < -obb.extents[i]) dist = -obb.extents[i];

		mathVector2ScalarMultiply(&t, &obb.axis[i], dist);
		mathVector2Add(&result, &result, &t);
	}

	return result;
}
