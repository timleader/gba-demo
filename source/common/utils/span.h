
#ifndef SPAN_H
#define SPAN_H

#include "common/types.h"


typedef struct span_s
{
	int16_t start;
	int16_t end;

} span_t;



#define inline __inline


static inline IWRAM_CODE void span_encapsulate(span_t* span, int16_t start, int16_t end)
{
	if (start < span->start)
		span->start = start;

	if (end > span->end)
		span->end = end;
}


#endif
