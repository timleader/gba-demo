
#include <stdio.h>
#include <windows.h>

#include "application/argparse.h"

#include "common/video/smacker.h"
#include "common/resources/resources.h "
#include "common/states/state.h"
#include "common/utils/profiler.h"
#include "common/input/input.h"
#include "common/utils/timer.h"
#include "common/memory.h"
#include "common/debug/debug.h"
#include "common/math/trigonometry.h"


static void print_header(void)
{
	printf(" _____ ______  ___    ___________ \n");
	printf("|  __ \\| ___ \\/ _ \\  |____ |  _  \\\n");
	printf("| |  \\/| |_/ / /_\\ \\     / / | | |\n");
	printf("| | __ | ___ \\  _  |     \\ \\ | | |\n");
	printf("| |_\\ \\| |_/ / | | | .___/ / |/ / \n");
	printf(" \\____/\\____/\\_| |_/ \\____/|___/  \n");
	printf("\n");
	printf("resources extract\n");
	printf("\n");
}

int main(int argc, char** argv)
{
	print_header();

	const char* resource_name = NULL;
	const char* output_path = NULL;

	static const char* const usage[] =
	{
		"test_argparse [options] [[--] args]",
		"test_argparse [options]",
		NULL,
	};

	struct argparse_option options[] =
	{
		OPT_HELP(),
		OPT_GROUP("Basic options"),
		OPT_STRING('r', "resource", &resource_name, "mode to do"),
		OPT_STRING('o', "output", &output_path, "mode to do"),
		OPT_END(),
	};

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

	resources_initialize();

	{
		videoclip_ptr video = resource_find_videoclip_from_name(resource_name);

		//	issue: can't extract the last resource as we don't know it's size

		if (video != NULL)
		{
			//	write bs to file
			FILE* outfile = fopen(output_path, "wb");
			fwrite(&video->data[0], 1, video->size, outfile);
			fclose(outfile);
		}
	}

	//	core shutdown
	resources_shutdown();

	return 0;
}
