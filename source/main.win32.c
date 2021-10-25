
#include <stdio.h>

#include <Windows.h>

#define SDL_MAIN_HANDLED

#include <SDL.h>

#if !defined (USE_SDL_DLL)
#pragma comment (lib, "SDL2.lib")
#endif

#include "common/platform/win32/app_sdl.h"
#include "common/application/argparse.h"
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

static const char* const usage[] =
{
	"test_argparse [options] [[--] args]",
	"test_argparse [options]",
	NULL,
};

const char* mode = NULL;
const char* path = NULL;

// headless 
//	smk_patch, input, output 

struct argparse_option options[] = 
{
	OPT_HELP(),
	OPT_GROUP("Basic options"),
	OPT_STRING('m', "mode", &mode, "mode to do"),
	OPT_STRING('p', "path", &path, "mode to do"),
	OPT_END(),
};

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

	//	Args Parsing

	struct argparse argparse;
	argparse_init(&argparse, options, usage, 0);
	argparse_describe(
		&argparse, 
		"\nA brief description of what the program does and how it works.", 
		"\nAdditional description of the program after the description of the arguments.");
	argc = argparse_parse(&argparse, argc, argv);


	//	core initialization
	debug_initialize();
	memory_initialize();
	profiler_initialize();

	//	split this out ??? into its own tool file in common

	if (mode != NULL &&
		string_compare(mode, "smk_patch") == 0)
	{
		debug_printf(0, "smk_patch");

		FILE* infile = fopen(path, "rb");
		fseek(infile, 0, SEEK_END);
		long fsize = ftell(infile);
		fseek(infile, 0, SEEK_SET);  /* same as rewind(f); */

		char* data = malloc(fsize);
		fread(data, 1, fsize, infile);
		fclose(infile);

		smk_output_stream_t* out_stream = smk_patch(data, fsize);

		//	write bs to file
		FILE* outfile = fopen("G:/workspace/git/gba3d/data/southpark_small_v5.smk", "wb");
		fwrite(out_stream->data_ptr, 1, out_stream->data_size, outfile);
		fclose(outfile);

		debug_printf(0, "smk_patch end");

		return;
	}

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
	
	state_initialize(4, 1536, MEMORY_IWRAM);
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
