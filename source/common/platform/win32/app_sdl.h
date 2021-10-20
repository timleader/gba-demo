
#ifndef APP_SDL_H
#define APP_SDL_H

#include "common/types.h"

//	should probably refactor this to GBA Hardware Emulation 

int8_t appSDLInitialize(void);

void appSDLPresent(void);

void appSDLShutdown(void);

#endif
