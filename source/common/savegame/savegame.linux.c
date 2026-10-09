#include "savegame.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

savegame_slots_ptr savegame_slots;

void savegame_initialize(void)
{
	const char* path = getenv("GBA_SAVE_PATH");
	if (!path || !*path)
		path = "build/linux/7days.sav";
	int fd = open(path, O_RDWR | O_CREAT, 0600);
	struct stat info;
	if (fd < 0 || fstat(fd, &info) < 0 ||
		(info.st_size < (off_t)sizeof(savegame_slots_t) &&
		 ftruncate(fd, sizeof(savegame_slots_t)) < 0))
	{
		perror(path);
		exit(EXIT_FAILURE);
	}
	void* mapping = mmap(NULL, sizeof(savegame_slots_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	if (mapping == MAP_FAILED)
	{
		perror(path);
		exit(EXIT_FAILURE);
	}
	savegame_slots = mapping;
}

void savegame_shutdown(void)
{
	if (savegame_slots)
	{
		msync(savegame_slots, sizeof(savegame_slots_t), MS_SYNC);
		munmap(savegame_slots, sizeof(savegame_slots_t));
		savegame_slots = NULL;
	}
}