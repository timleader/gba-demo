
#ifndef MATRIX_H
#define MATRIX_H


#include "fixed16.h"
#include "vector.h"
#include "trigonometry.h"

//	 column major !!!

typedef struct matrix4x4_s		//	f16matrix4x4_s
{
	vector4_t col0;
	vector4_t col1;
	vector4_t col2;
	vector4_t col3;
} matrix4x4_t;

static inline IWRAM_CODE void mathMatrix4x4Copy(matrix4x4_t* result, const matrix4x4_t* mat)
{
	mathVector4Copy(&result->col0, &mat->col0);
	mathVector4Copy(&result->col1, &mat->col1);
	mathVector4Copy(&result->col2, &mat->col2);
	mathVector4Copy(&result->col3, &mat->col3);
}

static inline IWRAM_CODE void mathMatrix4x4MakeIdentity(matrix4x4_t* result)
{
	mathVector4MakeFromElements(&result->col0, fixed16_one, fixed16_zero, fixed16_zero, fixed16_zero);
	mathVector4MakeFromElements(&result->col1, fixed16_zero, fixed16_one, fixed16_zero, fixed16_zero);
	mathVector4MakeFromElements(&result->col2, fixed16_zero, fixed16_zero, fixed16_one, fixed16_zero);
	mathVector4MakeFromElements(&result->col3, fixed16_zero, fixed16_zero, fixed16_zero, fixed16_one);
}

static inline IWRAM_CODE void mathMatrix4x4MakeTranslation(matrix4x4_t* result, vector3_t* translation)
{
	mathVector4MakeFromElements(&result->col0, fixed16_one, fixed16_zero, fixed16_zero, fixed16_zero);
	mathVector4MakeFromElements(&result->col1, fixed16_zero, fixed16_one, fixed16_zero, fixed16_zero);
	mathVector4MakeFromElements(&result->col2, fixed16_zero, fixed16_zero, fixed16_one, fixed16_zero);
	mathVector4MakeFromElements(&result->col3, translation->x, translation->y, translation->z, fixed16_one);
}

static inline IWRAM_CODE void mathMatrix4x4MakeRotationZYX(matrix4x4_t *result, const vector3_t *radiansXYZ)
{
	fixed16_t sX, cX, sY, cY, sZ, cZ, tmp0, tmp1;
	sX = fixed16_sine(radiansXYZ->x);
	cX = fixed16_cosine(radiansXYZ->x);
	sY = fixed16_sine(radiansXYZ->y);
	cY = fixed16_cosine(radiansXYZ->y);
	sZ = fixed16_sine(radiansXYZ->z);
	cZ = fixed16_cosine(radiansXYZ->z);
	tmp0 = fixed16_mul(cZ, sY);
	tmp1 = fixed16_mul(sZ, sY);
	mathVector4MakeFromElements(&result->col0, fixed16_mul(cZ, cY), fixed16_mul(sZ, cY), -sY, fixed16_zero);
	mathVector4MakeFromElements(&result->col1, (fixed16_mul(tmp0, sX) - fixed16_mul(sZ, cX)), (fixed16_mul(tmp1, sX) + fixed16_mul(cZ, cX)), fixed16_mul(cY, sX), fixed16_zero);
	mathVector4MakeFromElements(&result->col2, (fixed16_mul(tmp0, cX) + fixed16_mul(sZ, sX)), (fixed16_mul(tmp1, cX) - fixed16_mul(cZ, sX)), fixed16_mul(cY, cX), fixed16_zero);
	mathVector4MakeFromElements(&result->col3, fixed16_zero, fixed16_zero, fixed16_zero, fixed16_one);
}

static inline IWRAM_CODE void mathMatrix4x4MakeScale(matrix4x4_t* result, fixed16_t scale)
{
	mathVector4MakeFromElements(&result->col0, scale, fixed16_zero, fixed16_zero, fixed16_zero);
	mathVector4MakeFromElements(&result->col1, fixed16_zero, scale, fixed16_zero, fixed16_zero);
	mathVector4MakeFromElements(&result->col2, fixed16_zero, fixed16_zero, scale, fixed16_zero);
	mathVector4MakeFromElements(&result->col3, fixed16_zero, fixed16_zero, fixed16_zero, fixed16_one);
}

static inline IWRAM_CODE void mathMatrix4x4MakeScaleXYZ(matrix4x4_t* result, const vector3_t* scale)
{
	mathVector4MakeFromElements(&result->col0, scale->x, fixed16_zero, fixed16_zero, fixed16_zero);
	mathVector4MakeFromElements(&result->col1, fixed16_zero, scale->y, fixed16_zero, fixed16_zero);
	mathVector4MakeFromElements(&result->col2, fixed16_zero, fixed16_zero, scale->z, fixed16_zero);
	mathVector4MakeFromElements(&result->col3, fixed16_zero, fixed16_zero, fixed16_zero, fixed16_one);
}

static inline IWRAM_CODE void mathMatrix4x4MakePerspective(matrix4x4_t* result, fixed16_t fovyRadians, fixed16_t aspect, fixed16_t zNear, fixed16_t zFar)
{
	fixed16_t f, rangeInv;
	f = fixed16_tangent((fixed16_pi>>1) - fixed16_mul(fixed16_one>>1, fovyRadians));
	rangeInv = fixed16_div(fixed16_one, zNear - zFar);
	mathVector4MakeFromElements(&result->col0, fixed16_div(f, aspect), fixed16_zero, fixed16_zero, fixed16_zero);
	mathVector4MakeFromElements(&result->col1, fixed16_zero, f, fixed16_zero, fixed16_zero);
	mathVector4MakeFromElements(&result->col2, fixed16_zero, fixed16_zero, fixed16_mul(zNear + zFar, rangeInv), -fixed16_one);
	mathVector4MakeFromElements(&result->col3, fixed16_zero, fixed16_zero, fixed16_mul(fixed16_mul(zNear, zFar), rangeInv) << 1, fixed16_zero);
}

//	mathMatrix4x4MakeTRS

static inline IWRAM_CODE void mathMatrix4x4MultiplyPoint3(vector4_t* result, matrix4x4_t* matrix, vector3_t* point)
{
	//	convert to asm 

	fixed16_t tmpX, tmpY, tmpZ, tmpW;
	tmpX = (((fixed16_mul(matrix->col0.x, point->x) + fixed16_mul(matrix->col1.x, point->y)) + fixed16_mul(matrix->col2.x, point->z)) + matrix->col3.x);
	tmpY = (((fixed16_mul(matrix->col0.y, point->x) + fixed16_mul(matrix->col1.y, point->y)) + fixed16_mul(matrix->col2.y, point->z)) + matrix->col3.y);
	tmpZ = (((fixed16_mul(matrix->col0.z, point->x) + fixed16_mul(matrix->col1.z, point->y)) + fixed16_mul(matrix->col2.z, point->z)) + matrix->col3.z);
	tmpW = (((fixed16_mul(matrix->col0.w, point->x) + fixed16_mul(matrix->col1.w, point->y)) + fixed16_mul(matrix->col2.w, point->z)) + matrix->col3.w);
	mathVector4MakeFromElements(result, tmpX, tmpY, tmpZ, tmpW);
}

static inline IWRAM_CODE void mathMatrix4x4MultiplyPoint3_Approx(vector4_t* result, matrix4x4_t* matrix, vector3_t* point)
{
	//	convert to asm, I don't think I can reduce the instructions much by handcrafting the asm

	result->x = (((fixed16_mul_approx2(matrix->col0.x, point->x) + fixed16_mul_approx2(matrix->col1.x, point->y)) + fixed16_mul_approx2(matrix->col2.x, point->z)) + matrix->col3.x);
	result->y = (((fixed16_mul_approx2(matrix->col0.y, point->x) + fixed16_mul_approx2(matrix->col1.y, point->y)) + fixed16_mul_approx2(matrix->col2.y, point->z)) + matrix->col3.y);
	result->z = (((fixed16_mul_approx2(matrix->col0.z, point->x) + fixed16_mul_approx2(matrix->col1.z, point->y)) + fixed16_mul_approx2(matrix->col2.z, point->z)) + matrix->col3.z);
	result->w = (((fixed16_mul_approx2(matrix->col0.w, point->x) + fixed16_mul_approx2(matrix->col1.w, point->y)) + fixed16_mul_approx2(matrix->col2.w, point->z)) + matrix->col3.w);
}

static inline IWRAM_CODE void mathMatrix4x4MultiplyVector(vector4_t* result, matrix4x4_t* matrix, vector4_t* vector)
{
	fixed16_t tmpX, tmpY, tmpZ, tmpW;
	tmpX = (((fixed16_mul(matrix->col0.x, vector->x) + fixed16_mul(matrix->col1.x, vector->y)) + fixed16_mul(matrix->col2.x, vector->z)) + fixed16_mul(matrix->col3.x, vector->w));
	tmpY = (((fixed16_mul(matrix->col0.y, vector->x) + fixed16_mul(matrix->col1.y, vector->y)) + fixed16_mul(matrix->col2.y, vector->z)) + fixed16_mul(matrix->col3.y, vector->w));
	tmpZ = (((fixed16_mul(matrix->col0.z, vector->x) + fixed16_mul(matrix->col1.z, vector->y)) + fixed16_mul(matrix->col2.z, vector->z)) + fixed16_mul(matrix->col3.z, vector->w));
	tmpW = (((fixed16_mul(matrix->col0.w, vector->x) + fixed16_mul(matrix->col1.w, vector->y)) + fixed16_mul(matrix->col2.w, vector->z)) + fixed16_mul(matrix->col3.w, vector->w));
	mathVector4MakeFromElements(result, tmpX, tmpY, tmpZ, tmpW);
}

static inline IWRAM_CODE void mathMatrix4x4Multiply(matrix4x4_t* result, matrix4x4_t* a, matrix4x4_t* b)
{
	matrix4x4_t tmpResult;
	mathMatrix4x4MultiplyVector(&tmpResult.col0, a, &b->col0);
	mathMatrix4x4MultiplyVector(&tmpResult.col1, a, &b->col1);
	mathMatrix4x4MultiplyVector(&tmpResult.col2, a, &b->col2);
	mathMatrix4x4MultiplyVector(&tmpResult.col3, a, &b->col3);
	mathMatrix4x4Copy(result, &tmpResult);
}

static inline IWRAM_CODE void mathMatrix4x4Inverse(matrix4x4_t* result, matrix4x4_t* mat)
{
	vector4_t res0, res1, res2, res3;
	fixed16_t mA, mB, mC, mD, mE, mF, mG, mH, mI, mJ, mK, mL, mM, mN, mO, mP, tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, detInv;

	mA = mat->col0.x;
	mB = mat->col0.y;
	mC = mat->col0.z;
	mD = mat->col0.w;
	mE = mat->col1.x;
	mF = mat->col1.y;
	mG = mat->col1.z;
	mH = mat->col1.w;
	mI = mat->col2.x;
	mJ = mat->col2.y;
	mK = mat->col2.z;
	mL = mat->col2.w;
	mM = mat->col3.x;
	mN = mat->col3.y;
	mO = mat->col3.z;
	mP = mat->col3.w;

	tmp0 = (fixed16_mul(mK, mD) - fixed16_mul(mC, mL));
	tmp1 = (fixed16_mul(mO, mH) - fixed16_mul(mG, mP));
	tmp2 = (fixed16_mul(mB, mK) - fixed16_mul(mJ, mC));
	tmp3 = (fixed16_mul(mF, mO) - fixed16_mul(mN, mG));
	tmp4 = (fixed16_mul(mJ, mD) - fixed16_mul(mB, mL));
	tmp5 = (fixed16_mul(mN, mH) - fixed16_mul(mF, mP));

	res0.x = ((fixed16_mul(mJ, tmp1) - fixed16_mul(mL, tmp3)) - fixed16_mul(mK, tmp5));
	res0.y = ((fixed16_mul(mN, tmp0) - fixed16_mul(mP, tmp2)) - fixed16_mul(mO, tmp4));
	res0.z = ((fixed16_mul(mD, tmp3) + fixed16_mul(mC, tmp5)) - fixed16_mul(mB, tmp1));
	res0.w = ((fixed16_mul(mH, tmp2) + fixed16_mul(mG, tmp4)) - fixed16_mul(mF, tmp0));

	detInv = fixed16_div(fixed16_one, (((fixed16_mul(mA, res0.x) + fixed16_mul(mE, res0.y)) + fixed16_mul(mI, res0.z)) + fixed16_mul(mM, res0.w)));

	res1.x = fixed16_mul(mI, tmp1);
	res1.y = fixed16_mul(mM, tmp0);
	res1.z = fixed16_mul(mA, tmp1);
	res1.w = fixed16_mul(mE, tmp0);

	res3.x = fixed16_mul(mI, tmp3);
	res3.y = fixed16_mul(mM, tmp2);
	res3.z = fixed16_mul(mA, tmp3);
	res3.w = fixed16_mul(mE, tmp2);

	res2.x = fixed16_mul(mI, tmp5);
	res2.y = fixed16_mul(mM, tmp4);
	res2.z = fixed16_mul(mA, tmp5);
	res2.w = fixed16_mul(mE, tmp4);

	tmp0 = (fixed16_mul(mI, mB) - fixed16_mul(mA, mJ));
	tmp1 = (fixed16_mul(mM, mF) - fixed16_mul(mE, mN));
	tmp2 = (fixed16_mul(mI, mD) - fixed16_mul(mA, mL));
	tmp3 = (fixed16_mul(mM, mH) - fixed16_mul(mE, mP));
	tmp4 = (fixed16_mul(mI, mC) - fixed16_mul(mA, mK));
	tmp5 = (fixed16_mul(mM, mG) - fixed16_mul(mE, mO));

	res2.x = ((fixed16_mul(mL, tmp1) - fixed16_mul(mJ, tmp3)) + res2.x);
	res2.y = ((fixed16_mul(mP, tmp0) - fixed16_mul(mN, tmp2)) + res2.y);
	res2.z = ((fixed16_mul(mB, tmp3) - fixed16_mul(mD, tmp1)) - res2.z);
	res2.w = ((fixed16_mul(mF, tmp2) - fixed16_mul(mH, tmp0)) - res2.w);

	res3.x = ((fixed16_mul(mJ, tmp5) - fixed16_mul(mK, tmp1)) + res3.x);
	res3.y = ((fixed16_mul(mN, tmp4) - fixed16_mul(mO, tmp0)) + res3.y);
	res3.z = ((fixed16_mul(mC, tmp1) - fixed16_mul(mB, tmp5)) - res3.z);
	res3.w = ((fixed16_mul(mG, tmp0) - fixed16_mul(mF, tmp4)) - res3.w);

	res1.x = ((fixed16_mul(mK, tmp3) - fixed16_mul(mL, tmp5)) - res1.x);
	res1.y = ((fixed16_mul(mO, tmp2) - fixed16_mul(mP, tmp4)) - res1.y);
	res1.z = ((fixed16_mul(mD, tmp5) - fixed16_mul(mC, tmp3)) + res1.z);
	res1.w = ((fixed16_mul(mH, tmp4) - fixed16_mul(mG, tmp2)) + res1.w);

	mathVector4ScalarMultiply(&result->col0, &res0, detInv);
	mathVector4ScalarMultiply(&result->col1, &res1, detInv);
	mathVector4ScalarMultiply(&result->col2, &res2, detInv);
	mathVector4ScalarMultiply(&result->col3, &res3, detInv);
}

static inline IWRAM_CODE void mathMatrix4x4ExtractTranslation(vector3_t* result, matrix4x4_t* mat)
{
	result->x = mat->col3.x;
	result->y = mat->col3.y;
	result->z = mat->col3.z;
}


#endif
