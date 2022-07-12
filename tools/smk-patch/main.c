
#include <stdio.h>


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
	printf("smacker video patcher (v4 -> v5)\n");
	printf("\n");
}

int main(int argc, char** argv)
{
	print_header();

	const char* input_path = NULL;
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
		OPT_STRING('i', "input", &input_path, "mode to do"),
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

	  
	//	Validate Parameter>?? 

	//	core initialization
	debug_initialize();
	memory_initialize();

	//Patch file
	{
		FILE* infile = fopen(input_path, "rb");
		fseek(infile, 0, SEEK_END);
		long fsize = ftell(infile);
		fseek(infile, 0, SEEK_SET);  /* same as rewind(f); */

		char* data = malloc(fsize);
		fread(data, 1, fsize, infile);
		fclose(infile);

		smk_output_stream_t* out_stream = smk_patch(data, fsize);

		//	write bs to file
		FILE* outfile = fopen(output_path, "wb");
		fwrite(out_stream->data_ptr, 1, out_stream->data_size, outfile);
		fclose(outfile);

		debug_printf(0, "smk_patch end");
	}

	return 0;
}