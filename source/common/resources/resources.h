
#ifndef RESOURCES_H
#define RESOURCES_H


#include "common/types.h"
#include "common/graphics/image.h"
#include "common/graphics/highlight_field.h"
#include "common/graphics/model.h"
#include "common/audio/audio.h"
#include "common/video/video.h"
#include "common/collision/collision.h"
#include "common/string.h"
#include "common/stringstore.h"

#include "games/7days/level.h"
#include "games/7days/dialogue.h" 
#include "games/7days/itemstore.h"
#include "games/7days/sequence.h"

#include "common/debug/debug.h"

//	ensure alignment (4 bytes) in the resource pack, so each resource starts at (4 byte)

//	package_main.res
//		need a process to create the .res file


//	resource packs , eg, - PACK_NAME/ASSET_NAME
//		splits character packs and scene packs - more maintanable 
//		-- shared resources??? 

typedef enum resource_type_s// : uint8_t
{
	/* Primitive Resources */
	RESOURCE_TYPE_PALETTE		= 0, 
	RESOURCE_TYPE_IMAGE			= 1,
	RESOURCE_TYPE_TILED_IMAGE	= 2,
	RESOURCE_TYPE_DEPTH_MAP		= 3,
	RESOURCE_TYPE_SOUND			= 4,
	RESOURCE_TYPE_VIDEO			= 5,
	RESOURCE_TYPE_MODEL			= 6,
	RESOURCE_TYPE_STRING_STORE	= 7,

	/* Complex Resources */
	RESOURCE_TYPE_LEVEL			= 8,
	RESOURCE_TYPE_DIALOGUE		= 9,
	RESOURCE_TYPE_SEQUENCE		= 10,
	RESOURCE_TYPE_ITEMSTORE		= 11,

	/* Advanced Graphics Resources */
	RESOURCE_TYPE_HIGHLIGHT_FIELD=12,

	//	Predefined Paths 

	RESOURCE_TYPE_UNKNOWN		= 32,

} resource_type_t;

extern const char* resource_type_names[];

//	this pragma is required by gba,   
#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

//	resource manifest					//	 put all named resources at the start to reduce search time
typedef struct resource_entry_s
{
	char name[23];						//	"title/bg"		//	need to up to 23		// only keep this for debug resource pack ... 
	uint8_t type;						//	model | image | etc...

	uint32_t offset;					//	ptr???

} resource_entry_t;

#pragma pack(pop)   /* restore original alignment from stack */


extern uint8_t* resource_base;
extern resource_entry_t* resources;
extern uint16_t resource_count;



void resources_initialize();

void resources_shutdown();


/*

//	be able to iterate over resource manifest


resourcesGet  

*/

#define inline __inline

inline int16_t resources_find_index_of(const char* name)
{
	for (int16_t id = 0; id < resource_count; ++id)
	{
		if (string_compare(name, resources[id].name) == 0)
		{
			return id;
		}
	}
	return -1;
}

inline const char* resources_get_name(uint16_t id)
{
	debug_assert(id < resource_count, "resources::get_name id > resource_count");

	return resources[id].name;
}

inline palette_ptr resources_find_palette(uint16_t id)			//	these resource_id only needs 10-bit precision
{
	debug_assert(resources[id].type == RESOURCE_TYPE_PALETTE, "resources::find_palette type != RESOURCE_TYPE_PALETTE");

	return (palette_ptr)(resource_base + resources[id].offset);
}

inline palette_ptr resources_find_palette_from_name(const char* name)
{
	for (int id = 0; id < resource_count; ++id)		
	{
		if (string_compare(name, resources[id].name) == 0)
		{
			return resources_find_palette(id);
		}
	}
	debug_assert(0, "palette not found");
	return 0;
}

inline image_ptr resources_find_image(uint16_t id)
{
	debug_assert(resources[id].type == RESOURCE_TYPE_IMAGE, "resources::find_image type != RESOURCE_TYPE_IMAGE");

	return (image_ptr)(resource_base + resources[id].offset);
}

inline image_ptr resources_find_image_from_name(const char* name)
{
	for (int id = 0; id < resource_count; ++id)
	{
		if (string_compare(name, resources[id].name) == 0)
		{
			return resources_find_image(id);
		}
	}
	debug_assert(0, "image not found");
	return 0;
}

inline tiledimage_ptr resources_find_tiledimage(uint16_t id)
{
	debug_assert(resources[id].type == RESOURCE_TYPE_TILED_IMAGE, "resources::find_tiledimage type != RESOURCE_TYPE_TILED_IMAGE");

	return (tiledimage_ptr)(resource_base + resources[id].offset);
}

inline tiledimage_ptr resources_find_tiledimage_from_name(const char* name)
{
	for (int id = 0; id < resource_count; ++id)		
	{
		if (string_compare(name, resources[id].name) == 0)
		{
			return resources_find_tiledimage(id);
		}
	}
	debug_assert(0, "tiledimage not found");
	return 0;
}

inline depthmap_ptr resources_find_depthmap(uint16_t id)
{
	debug_assert(resources[id].type == RESOURCE_TYPE_DEPTH_MAP, "resources::find_depthmap type != RESOURCE_TYPE_DEPTH_MAP");

	return (depthmap_ptr)(resource_base + resources[id].offset);
}

inline highlight_field_ptr resources_find_highlightfield(uint16_t id)
{
	debug_assert(resources[id].type == RESOURCE_TYPE_HIGHLIGHT_FIELD, "resources::find_highlightfield type != RESOURCE_TYPE_HIGHLIGHT_FIELD");

	return (highlight_field_ptr)(resource_base + resources[id].offset);
}

inline audioclip_ptr resources_find_audioclip(uint16_t id)
{
	debug_assert(resources[id].type == RESOURCE_TYPE_SOUND, "resources::find_soundclip type != RESOURCE_TYPE_SOUND");

	return (audioclip_ptr)(resource_base + resources[id].offset);
}

inline audioclip_ptr resources_find_audioclip_from_name(const char* name)
{
	for (int id = 0; id < resource_count; ++id)
	{
		if (string_compare(name, resources[id].name) == 0)
		{
			return resources_find_audioclip(id);
		}
	}
	debug_assert(0, "audioclip not found");
	return 0;
}

inline videoclip_ptr resource_find_videoclip(uint16_t id)
{
	debug_assert(resources[id].type == RESOURCE_TYPE_VIDEO, "resources::find_videoclip type != RESOURCE_TYPE_VIDEO");

	return (videoclip_ptr)(resource_base + resources[id].offset);
}

inline videoclip_ptr resource_find_videoclip_from_name(const char* name)
{
	for (int id = 0; id < resource_count; ++id)
	{
		if (string_compare(name, resources[id].name) == 0)
		{
			return resource_find_videoclip(id);
		}
	}
	debug_assert(0, "videoclip not found");
	return 0;
}

inline stringstore_ptr resources_find_stringstore(uint16_t id)
{
	debug_assert(resources[id].type == RESOURCE_TYPE_STRING_STORE, "resources::find_stringstore type != RESOURCE_TYPE_STRING_STORE");

	return (stringstore_ptr)(resource_base + resources[id].offset);
}

inline stringstore_ptr resources_find_stringstore_fromname(const char* name)
{
	for (int id = 0; id < resource_count; ++id)	
	{
		if (string_compare(name, resources[id].name) == 0)
		{
			return resources_find_stringstore(id);
		}
	}
	debug_assert(0, "stringstore not found");
	return 0;
}

inline itemstore_ptr resources_find_itemstore(uint16_t id)
{
	debug_assert(resources[id].type == RESOURCE_TYPE_ITEMSTORE, "resources::find_itemstore type != RESOURCE_TYPE_ITEMSTORE");

	return (itemstore_ptr)(resource_base + resources[id].offset);
}

inline itemstore_ptr resources_find_itemstore_from_name(const char* name)
{
	for (int id = 0; id < resource_count; ++id)	
	{
		if (string_compare(name, resources[id].name) == 0)
		{
			return resources_find_itemstore(id);
		}
	}
	debug_assert(0, "itemstore not found");
	return NULL;
}

inline dialogue_ptr resources_find_dialogue(uint16_t id)
{
	debug_assert(resources[id].type == RESOURCE_TYPE_DIALOGUE, "resources::find_dialogue type != RESOURCE_TYPE_DIALOGUE");

	return (dialogue_ptr)(resource_base + resources[id].offset);
}

model_t* resources_find_model(uint16_t id);

inline model_t* resources_find_model_from_name(const char* name)
{
	for (int id = 0; id < resource_count; ++id)		//	fix this !!! 
	{
		if (string_compare(name, resources[id].name) == 0)
		{
			return resources_find_model(id);
		}
	}
	debug_assert(0, "model not found");
	return 0;
}

inline sequence_store_ptr resources_find_sequencestore(uint16_t id)
{
	debug_assert(resources[id].type == RESOURCE_TYPE_SEQUENCE, "resources::find_sequencestore type != RESOURCE_TYPE_SEQUENCE");

	return (sequence_store_ptr)(resource_base + resources[id].offset);
}

inline sequence_store_ptr resourceFindSequenceStoreFromName(const char* name)
{
	for (int id = 0; id < resource_count; ++id)
	{
		if (string_compare(name, resources[id].name) == 0)
		{
			return resources_find_sequencestore(id);
		}
	}
	return 0;
}

level_t* resources_find_level(uint16_t id);

inline level_t* resourcesFindSceneFromName(const char* name)
{
	for (int id = 0; id < 17; ++id)		//	fix this !!! 
	{
		if (string_compare(name, resources[id].name) == 0)
		{
			return resources_find_level(id);
		}
	}
	return 0;
}

//	how should the resources system be extended for game code.

//	all resources are stored as packedStructs, with an unpack function, with option to unpack to given memory


//	FUTURE

//	resource working memory... 

//image_t* resourcesDecompressImage(short resourceIdx);

//	DMA		for		animation / video / audio



/**
	Maybe a different res package  per levels ?? + common ??? 
 */

#endif
