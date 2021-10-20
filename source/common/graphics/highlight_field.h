
#ifndef HIGHLIGHT_FIELD_H
#define HIGHLIGHT_FIELD_H


#include "common/types.h"
#include "common/math/fixed16.h"


typedef struct highlight_span_s
{
	uint16_t row;
	uint8_t start, end;

} highlight_span_t;

//	this can be packed far better 
typedef struct highlight_field_s
{
	uint16_t span_count;

	uint8_t row_start;
	uint8_t row_count;

	highlight_span_t spans[0];

} highlight_field_t;

typedef highlight_field_t* highlight_field_ptr;


#endif	//	!HIGHLIGHT_FIELD_H
