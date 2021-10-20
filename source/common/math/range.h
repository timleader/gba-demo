
#ifndef RANGE_H
#define RANGE_H


#include "common/types.h"
#include "fixed16.h"


typedef struct range_s
{
	int16_t min, max;

} range_t;


static inline IWRAM_CODE void range_encapsulate(range_t* range, int16_t value)
{
	if (value < range->min)
		range->min = value;

	if (value > range->max)
		range->max = value;
}


#endif
