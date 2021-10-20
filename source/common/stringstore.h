
#ifndef STRINGSTORE_H
#define STRINGSTORE_H

#include "common/types.h"

extern const char* stringlocale_names[5];
extern const char* stringlocale_codes[5];

typedef enum stringlocale_s
{
	LOCALE_EN_GB	= 0,
	LOCALE_FR_FR	= 1,
	LOCALE_IT_IT	= 2,
	LOCALE_DE_DE	= 3,
	LOCALE_ES_ES	= 4,

} stringlocale_t;

typedef struct stringstore_s
{
	uint16_t count;
	uint16_t reserved;

	uint16_t indices[0];

} stringstore_t;

typedef stringstore_t* stringstore_ptr;


void stringstore_initialize(stringlocale_t locale);

void stringstore_shutdown(void);

void stringstore_set_language(stringlocale_t locale);

const char* stringstore_get(uint16_t id);


#endif
