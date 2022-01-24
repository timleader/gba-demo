
#include <stdio.h>

#include <Windows.h>

#define SDL_MAIN_HANDLED

#include <SDL.h>

#if !defined (USE_SDL_DLL)
#pragma comment (lib, "SDL2.lib")
#endif

#include "common/platform/win32/app_sdl.h"
#include "common/graphics/graphics.h"
#include "common/graphics/overlay.h"
#include "common/video/smacker.h"
#include "common/resources/resources.h "
#include "common/states/state.h"
#include "common/utils/profiler.h"
#include "common/input/input.h"
#include "common/utils/timer.h"
#include "common/memory.h"
#include "common/debug/debug.h"
#include "common/stringstore.h"
#include "common/savegame/savegame.h"
#include "common/math/trigonometry.h"

#include "games/7days/states/states.h"
#include "games/7days/itemstore.h"


static void print_header(void)
{
	printf(" _____ ______  ___    ___________ \n");
	printf("|  __ \\| ___ \\/ _ \\  |____ |  _  \\\n");
	printf("| |  \\/| |_/ / /_\\ \\     / / | | |\n");
	printf("| | __ | ___ \\  _  |     \\ \\ | | |\n");
	printf("| |_\\ \\| |_/ / | | | .___/ / |/ / \n");
	printf(" \\____/\\____/\\_| |_/ \\____/|___/  \n");
	printf("\n");
}

int main(int argc, char** argv)
{
	print_header();

	SDL_SetMainReady();


	//	core initialization
	debug_initialize();
	memory_initialize();
	profiler_initialize();

	//	App SDL Initialization

	appSDLInitialize();

	//	Engine Initialization

	resources_initialize();

	graphics_initialize();
	overlay_initialize();
	audio_initialize();

	stringstore_initialize(LOCALE_EN_GB);	//	initialize after language select 
	itemstore_initialize();
	savegame_initialize();
	
	state_initialize(4, 2048, MEMORY_EWRAM);
	state_goto(&st_splash, 0);

	uint32_t ewram_used = 0, iwram_used = 0;

	for (;;)
	{
		state_apply();

		input_poll();

		state_update();

		audio_update();

		state_draw();

		graphics_vsync();

		graphics_present();
		overlay_present();

		appSDLPresent();	

		if (memory_usage(MEMORY_EWRAM) != ewram_used ||
			memory_usage(MEMORY_IWRAM) != iwram_used)
		{
			iwram_used = memory_usage(MEMORY_IWRAM);
			ewram_used = memory_usage(MEMORY_EWRAM);

			memory_output_usage();
		}
	}

	savegame_shutdown();

	stringstore_shutdown();

	//	Engine Shutdown

	audio_shutdown();
	overlay_shutdown();
	graphicsShutdown(); 

	resources_shutdown();

	//	App SDL Shutdown

	appSDLShutdown();
	
	return 0;
}
