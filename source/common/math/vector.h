
#ifndef VECTOR_H
#define VECTOR_H


#include "fixed16.h"


typedef struct vector2_s		//	f16vector2_s / f8vector2_s
{
	fixed16_t x, y;
} vector2_t;

typedef struct vector3_s
{
	fixed16_t x, y, z;
} vector3_t;

typedef struct vector4_s
{
	fixed16_t x, y, z, w;
} vector4_t;

/**

	Why doesn't id tech 2 use function with (ptr, ptr) for vector stuff
		instead it uses #defines or clear function  'id_shared.h' :163
	
 */

static inline IWRAM_CODE void mathVector2Copy(vector2_t* result, const vector2_t* vec)
{
	result->x = vec->x;
	result->y = vec->y;
}

static inline IWRAM_CODE void mathVector2MakeFromElements(vector2_t* result, fixed16_t x, fixed16_t y)
{
	result->x = x;
	result->y = y;
}

static inline IWRAM_CODE void mathVector2Substract(vector2_t* result, const vector2_t* a, const vector2_t* b)
{
	result->x = a->x - b->x;
	result->y = a->y - b->y;
}

static inline IWRAM_CODE void mathVector2Add(vector2_t* result, const vector2_t* a, const vector2_t* b)
{
	result->x = a->x + b->x;
	result->y = a->y + b->y;
}

static inline IWRAM_CODE fixed16_t mathVector2DotProduct(const vector2_t* a, const vector2_t* b)
{
	fixed16_t result = fixed16_zero;
	result += fixed16_mul(a->x, b->x);
	result += fixed16_mul(a->y, b->y);
	return result;
}

static inline IWRAM_CODE void mathVector2ScalarMultiply(vector2_t* result, const vector2_t* vec, fixed16_t scalar)
{
	result->x = fixed16_mul(vec->x, scalar);
	result->y = fixed16_mul(vec->y, scalar);
}

static inline IWRAM_CODE fixed16_t mathVector2LengthSqr(const vector2_t* vec)
{
	fixed16_t result = fixed16_zero;
	result += fixed16_mul(vec->x, vec->x);
	result += fixed16_mul(vec->y, vec->y);
	return result;
}

static inline IWRAM_CODE fixed16_t mathVector2Length(const vector2_t* vec)
{
	fixed16_t result = mathVector2LengthSqr(vec);
	result = fixed16_sqrt(result);
	return result;
}

static inline IWRAM_CODE void mathVector3Copy(vector3_t* result, const vector3_t* vec)
{
	result->x = vec->x;
	result->y = vec->y;
	result->z = vec->z;
}

static inline IWRAM_CODE void mathVector3MakeFromElements(vector3_t* result, fixed16_t x, fixed16_t y, fixed16_t z)
{
	result->x = x;
	result->y = y;
	result->z = z;
}

static inline IWRAM_CODE void mathVector3Add(vector3_t* result, const vector3_t* a, const vector3_t* b)
{
	result->x = a->x + b->x;
	result->y = a->y + b->y;
	result->z = a->z + b->z;
}

static inline IWRAM_CODE void mathVector3Substract(vector3_t* result, const vector3_t* a, const vector3_t* b)
{
	result->x = a->x - b->x;
	result->y = a->y - b->y;
	result->z = a->z - b->z;
}

static inline IWRAM_CODE fixed16_t mathVector3LengthSqr(const vector3_t* vec)
{
	fixed16_t result = fixed16_zero;
	result += fixed16_mul(vec->x, vec->x);
	result += fixed16_mul(vec->y, vec->y);
	result += fixed16_mul(vec->z, vec->z);
	return result;
}

static inline IWRAM_CODE fixed16_t mathVector3Length(const vector3_t* vec)
{
	fixed16_t distance = mathVector3LengthSqr(vec);
	return fixed16_sqrt(distance);
}

static inline IWRAM_CODE fixed16_t mathVector3DistanceSqr(const vector3_t* a, const vector3_t* b)
{
	vector3_t delta;
	mathVector3Substract(&delta, a, b);
	return mathVector3LengthSqr(&delta);
}

static inline IWRAM_CODE fixed16_t mathVector3Distance(const vector3_t* a, const vector3_t* b)
{
	vector3_t delta;
	mathVector3Substract(&delta, a, b);
	fixed16_t distance = mathVector3LengthSqr(&delta);
	return fixed16_sqrt(distance);
}

static inline void mathVector3ScalarMultiply(vector3_t* result, const vector3_t* vec, fixed16_t scalar)
{
	result->x = fixed16_mul(vec->x, scalar);
	result->y = fixed16_mul(vec->y, scalar);
	result->z = fixed16_mul(vec->z, scalar);
}

static inline IWRAM_CODE void mathVector4Copy(vector4_t* result, const vector4_t* vec)
{
	result->x = vec->x;
	result->y = vec->y;
	result->z = vec->z;
	result->w = vec->w;
}

static inline IWRAM_CODE void mathVector4MakeFromElements(vector4_t* result, fixed16_t x, fixed16_t y, fixed16_t z, fixed16_t w)
{
	result->x = x;
	result->y = y;
	result->z = z;
	result->w = w;
}

static inline IWRAM_CODE fixed16_t mathVector4LengthSqr(const vector4_t *vec)
{
	fixed16_t result = fixed16_zero;
	result += fixed16_mul(vec->x, vec->x);
	result += fixed16_mul(vec->y, vec->y);
	result += fixed16_mul(vec->z, vec->z);
	result += fixed16_mul(vec->w, vec->w);
	return result;
}

static inline void mathVector4ScalarMultiply(vector4_t* result, const vector4_t* vec, fixed16_t scalar)
{
	result->x = fixed16_mul(vec->x, scalar);
	result->y = fixed16_mul(vec->y, scalar);
	result->z = fixed16_mul(vec->z, scalar);
	result->w = fixed16_mul(vec->w, scalar);
}

/*
static inline void mathVector4Normalize(vector4_t *result, const vector4_t *vec)
{
	fixed16_t lenSqr, lenInv;
	lenSqr = mathVector4LengthSqr(vec);
	lenInv = fixed16_div(fixed16_one, sqrtf(lenSqr));	
	result->x = fixed16_mul(vec->x, lenInv);
	result->y = fixed16_mul(vec->y, lenInv);
	result->z = fixed16_mul(vec->z, lenInv);
	result->w = fixed16_mul(vec->w, lenInv);
}
*/

#endif
