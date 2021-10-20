
#ifndef POINT_H
#define POINT_H

#include "fixed16.h"

typedef struct point2_s
{
	int16_t x, y;
} point2_t;

static inline IWRAM_CODE void mathPoint2Copy(point2_t* result, const point2_t* vec)
{
	result->x = vec->x;
	result->y = vec->y;
}

static inline IWRAM_CODE void mathPoint2MakeFromElements(point2_t* result, int16_t x, int16_t y)
{
	result->x = x;
	result->y = y;
}

#endif
