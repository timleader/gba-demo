
#include "debug.h"

#include "common/utils/timer.h"

#include <stdio.h>  
#include <stdarg.h>
#include <assert.h>

//-----------------------------------------------------------------------------
#define DEBUG_LOG_FRAME_COUNT_ENABLE	1 

//-----------------------------------------------------------------------------
FILE* g_debug_log_file;

//-----------------------------------------------------------------------------
void debug_initialize(void)
{
#ifdef _WIN32
	g_debug_log_file = fopen("../log.txt", "w+");
#else
	g_debug_log_file = fopen("build/linux/log.txt", "w+");
#endif
}

//-----------------------------------------------------------------------------
void debug_shutdown(void)
{
	if (g_debug_log_file)
		fclose(g_debug_log_file);

	g_debug_log_file = NULL;
}

//-----------------------------------------------------------------------------
void debug_printf(uint8_t level, const char* format, ...)
{
#if DEBUG_LOG_FRAME_COUNT_ENABLE
	uint32_t frame = timer_get_frame_count();
	printf("[%u]\t", frame);
	if (g_debug_log_file)
		fprintf(g_debug_log_file, "[%u]\t", frame);
#endif

	va_list args;
	va_start(args, format);
	{
		if (g_debug_log_file)
		{
			va_list file_args;
			va_copy(file_args, args);
			vfprintf(g_debug_log_file, format, file_args);
			va_end(file_args);
		}
		vprintf(format, args);
	}
	va_end(args);

	printf("\n");

	if (g_debug_log_file)
	{
		fprintf(g_debug_log_file, "\n");
		fflush(g_debug_log_file);
	}
}  

//-----------------------------------------------------------------------------
void debug_assert(int32_t condition, const char* message)
{
	if (!condition)
	{
		debug_printf(DEBUG_LOG_FATAL, "debug::assert - failed : %s", message);
#ifdef _WIN32
		__debugbreak();
#else
		assert(condition);
#endif
		//	if release, then this shouldn't exist... 
	}
}

//-----------------------------------------------------------------------------
void debug_variable_set(const char* name, uint32_t type, const void_ptr address)
{ }

//-----------------------------------------------------------------------------
void debug_variable_unset(const char* name)
{ }
