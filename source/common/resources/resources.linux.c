#include "resources.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

uint8_t* resource_base;
resource_entry_t* resources;
uint16_t resource_count;
static size_t mapping_size;

void resources_initialize(void)
{
	const char* path = "data/package_7days.res";
	int fd = open(path, O_RDONLY);
	struct stat info;
	if (fd < 0 || fstat(fd, &info) < 0 || info.st_size < 2)
	{
		fprintf(stderr, "Cannot read %s; run from the repository root.\n", path);
		exit(EXIT_FAILURE);
	}
	mapping_size = (size_t)info.st_size;
	void* mapping = mmap(NULL, mapping_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (mapping == MAP_FAILED)
	{
		perror(path);
		exit(EXIT_FAILURE);
	}
	resource_base = mapping;
	resource_count = *(uint16_t*)resource_base;
	if (2 + (size_t)resource_count * sizeof(resource_entry_t) > mapping_size)
	{
		fprintf(stderr, "Invalid resource manifest: %s\n", path);
		exit(EXIT_FAILURE);
	}
	resources = (resource_entry_t*)(resource_base + 2);
	for (uint16_t id = 0; id < resource_count; ++id)
	{
		if (resources[id].offset >= mapping_size || (resources[id].offset & 3))
		{
			fprintf(stderr, "Invalid resource offset at index %u\n", id);
			exit(EXIT_FAILURE);
		}
	}
}

void resources_shutdown(void)
{
	if (resource_base)
		munmap(resource_base, mapping_size);
	resource_base = NULL;
	resources = NULL;
	resource_count = 0;
}