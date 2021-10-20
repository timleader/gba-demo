
#ifndef FIXED16_H
#define FIXED16_H

#include "common/platform/gba/gba.h"

//	fixed_16.16

//	fixed_6.10	//	maybe fixed_5.10  

typedef int32_t fixed16_t;			//	can we reduce this to 8.8 fixed at least for data that comes from RAM or ROM

static const IWRAM_DATA fixed16_t FOUR_DIV_PI = 0x145F3;            /*!< Fix16 value of 4/PI */
static const IWRAM_DATA fixed16_t _FOUR_DIV_PI2 = 0xFFFF9840;        /*!< Fix16 value of -4/PI² */
static const IWRAM_DATA fixed16_t X4_CORRECTION_COMPONENT = 0x399A; 	/*!< Fix16 value of 0.225 */
static const IWRAM_DATA fixed16_t PI_DIV_4 = 0x0000C90F;             /*!< Fix16 value of PI/4 */
static const IWRAM_DATA fixed16_t THREE_PI_DIV_4 = 0x00025B2F;       /*!< Fix16 value of 3PI/4 */

static const IWRAM_DATA fixed16_t fixed16_maximum = 0x7FFFFFFF; /*!< the maximum value of fix16_t */
static const IWRAM_DATA fixed16_t fixed16_minimum = 0x80000000; /*!< the minimum value of fix16_t */
static const IWRAM_DATA fixed16_t fixed16_overflow = 0x80000000; /*!< the value used to indicate overflows when FIXMATH_NO_OVERFLOW is not specified */

static const IWRAM_DATA fixed16_t fixed16_pi = 205887;     /*!< fix16_t value of pi */
static const IWRAM_DATA fixed16_t fixed16_2pi = 205887 << 1;
static const IWRAM_DATA fixed16_t fixed16_e = 178145;     /*!< fix16_t value of e */
static const IWRAM_DATA fixed16_t fixed16_one = 0x00010000; /*!< fix16_t value of 1 */
static const IWRAM_DATA fixed16_t fixed16_zero = 0x00000000; /*!< fix16_t value of 0 */
static const IWRAM_DATA fixed16_t fixed16_half = 0x00010000 >> 1;

#define inline __inline

/* Conversion functions between fix16_t and float/integer.
* These are inlined to allow compiler to optimize away constant numbers
*/
static inline IWRAM_CODE fixed16_t fixed16_from_int(int a)     { return a * fixed16_one; }		//	convert these to defines to ensure that the inline 

static inline IWRAM_CODE int32_t fixed16_to_int(fixed16_t a)
{
	return (a >> 16);
}


#define F16(x) ((fixed16_t)(((x) >= 0) ? ((x) * 65536.0 + 0.5) : ((x) * 65536.0 - 0.5)))

static inline IWRAM_CODE fixed16_t fixed16_abs(fixed16_t x)
{
	return (x < 0 ? -x : x);
}
static inline IWRAM_CODE fixed16_t fixed16_floor(fixed16_t x)
{
	return (x & 0xFFFF0000UL);
}
static inline IWRAM_CODE fixed16_t fixed16_ceil(fixed16_t x)
{
	return (x & 0xFFFF0000UL) + (x & 0x0000FFFFUL ? fixed16_one : 0);
}
static inline IWRAM_CODE fixed16_t fixed16_min(fixed16_t x, fixed16_t y)
{
	return (x < y ? x : y);
}
static inline IWRAM_CODE fixed16_t fixed16_max(fixed16_t x, fixed16_t y)
{
	return (x > y ? x : y);
}
static inline IWRAM_CODE fixed16_t fixed16_clamp(fixed16_t x, fixed16_t lo, fixed16_t hi)
{
	return fixed16_min(fixed16_max(x, lo), hi);
}

static inline IWRAM_CODE fixed16_t fixed16_add(fixed16_t inArg0, fixed16_t inArg1) { return (inArg0 + inArg1); }
static inline IWRAM_CODE fixed16_t fixed16_sub(fixed16_t inArg0, fixed16_t inArg1) { return (inArg0 - inArg1); }

extern IWRAM_CODE fixed16_t fixed16_lerp(fixed16_t v0, fixed16_t v1, fixed16_t t);

static inline IWRAM_CODE fixed16_t fixed16_mul_approx(fixed16_t inArg0, fixed16_t inArg1) { return (inArg0 * inArg1) >> 16; }
static inline IWRAM_CODE fixed16_t fixed16_mul_approx2(fixed16_t inArg0, fixed16_t inArg1) { return ((inArg0 >> 8) * inArg1) >> 8; }
extern IWRAM_CODE fixed16_t fixed16_mul_precise(fixed16_t inArg0, fixed16_t inArg1);

static inline IWRAM_CODE fixed16_t fixed16_mul(fixed16_t inArg0, fixed16_t inArg1) { return fixed16_mul_precise(inArg0, inArg1); }

extern IWRAM_CODE fixed16_t fixed16_div(fixed16_t a, fixed16_t b);

extern IWRAM_CODE fixed16_t fixed16_mod(fixed16_t x, fixed16_t y);

extern IWRAM_CODE fixed16_t fixed16_sqrt(fixed16_t x);


extern fixed16_t reciprocal_lut[1024];


#endif
