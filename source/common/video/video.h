
#ifndef VIDEO_H
#define VIDEO_H

#include "common/math/vector.h"
#include "common/math/matrix.h"


typedef struct videoclip_s
{
	int16_t sequeunce_collection_id;
	uint16_t reserved;

	uint32_t size;
	uint8_t data[0];

} videoclip_t;

typedef videoclip_t* videoclip_ptr;



#endif
