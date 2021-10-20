
#include "stringstore.h"

#include "common/resources/resources.h"
#include "common/memory.h"

EWRAM_DATA const char* stringlocale_names[5] = {
	"English",
	"Français",
	"Italiano",
	"Deutsche",
	"Español"
};

EWRAM_DATA const char* stringlocale_codes[5] = {
	"en",
	"fr",
	"it",
	"de",
	"es",
};

stringstore_ptr stringstore;

void stringstore_initialize(stringlocale_t locale)
{
	stringstore_set_language(locale);
}

void stringstore_shutdown(void)
{
}

void stringstore_set_language(stringlocale_t locale)
{
	char stringstore_asset_name[23] = "str/";
	uint32_t len = string_length(stringstore_asset_name);
	
	memory_copy(stringstore_asset_name + len, (const void_ptr)stringlocale_codes[(uint32_t)locale], 2);
	stringstore_asset_name[len + 2] = 0;

	stringstore = resources_find_stringstore_fromname(stringstore_asset_name);
	 
	debug_assert(stringstore != NULL, "stringstore::set_language stringstore == NULL");
}

const char* stringstore_get(uint16_t id)
{
	debug_assert(id < stringstore->count, "stringstore::get id >= stringstore->count");

	uint8_t* base = (uint8_t*)stringstore->indices;
	return (const char*)(base + stringstore->indices[id]);
}

