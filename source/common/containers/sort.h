
#ifndef SORT_H
#define SORT_H

#include "common/types.h"

//	this struct is referenced in asm, don't change it without updating sort.gba.s
//-----------------------------------------------------------------------------
typedef struct sort_index_s
{
	int16_t value;
	uint16_t idx;

} sort_index_t;

//-----------------------------------------------------------------------------
#define swap_values(type, a, b) \
{ \
	type c = a; \
	a = b; \
	b = c; \
} 

//-----------------------------------------------------------------------------
void sort_insertion(sort_index_t* base, const uint32_t num);

#endif
