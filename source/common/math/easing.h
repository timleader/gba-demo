
#ifndef EASING_H
#define EASING_H

#include "common/math/fixed16.h"
#include "common/math/trigonometry.h"


static inline IWRAM_CODE fixed16_t math_easeinquad(fixed16_t x) { return fixed16_mul(x, x); }

static inline IWRAM_CODE fixed16_t math_easeoutquad(fixed16_t x) { return fixed16_mul(x, (fixed16_one << 1) - x); }

static inline IWRAM_CODE fixed16_t math_easeinoutquad(fixed16_t x) 
{ 
	if (x < fixed16_half)
	{
		return fixed16_mul(x, x);
	}
	else
	{
		return fixed16_mul(x, (fixed16_one << 1) - x);
	}
}

static inline IWRAM_CODE fixed16_t math_easeinsine(fixed16_t x) { return fixed16_one - fixed16_cosine(fixed16_mul(x, fixed16_pi) >> 1); }

static inline IWRAM_CODE fixed16_t math_easeoutsine(fixed16_t x) { return fixed16_sine(fixed16_mul(x, fixed16_pi) >> 1); }

static inline IWRAM_CODE fixed16_t math_easeinoutsine(fixed16_t x) { return -(fixed16_cosine(fixed16_mul(x, fixed16_pi)) - fixed16_one) >> 1; }


#endif //EASING_H
