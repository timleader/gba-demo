
#include "common/resources/resources.h"

#include "package_7days_res.h"

uint16_t resource_count;

uint8_t* resource_base;
resource_entry_t* resources;

void resources_initialize()
{
	uint8_t* data_ptr = (uint8_t*)&package_7days_res[0];

	resource_base = data_ptr;
	resource_count = *((uint16_t*)data_ptr);

	data_ptr += 2;
	resources = (resource_entry_t*)data_ptr;

	debug_printf(DEBUG_LOG_INFO, "resources::initialized");
	for (uint16_t id = 0; id < resource_count; ++id)
	{
		debug_assert((resources[id].offset & 0x03) == 0, "resources::initialize resources not aligned");
		//debug_printf(DEBUG_LOG_INFO, "\tresources[%u].offset = %u", id, resources[id].offset);
	}
}

void resources_shutdown()
{ }
