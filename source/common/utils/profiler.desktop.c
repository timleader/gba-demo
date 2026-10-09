
#include "profiler.h"

#include "common/debug/debug.h"

#ifdef PROFILER_ENABLE

#include <SDL.h>


uint64_t g_frequency;

typedef struct profiler_timer_s
{
	char label[PROFILER_MAX_LABEL_LENGTH];
	uint8_t in_use;
	uint64_t start_time;

} profiler_perf_timer_t;

profiler_perf_timer_t g_profiler_timers[PROFILER_MAX_CONCURRENT];

profiler_entry_t g_profiler_entries[PROFILER_MAX_ENTRIES_COUNT];
uint32_t g_profiler_entries_cursor;

/* ---- API ---- */

void profiler_initialize(void)
{
	g_frequency = SDL_GetPerformanceFrequency();

	for (uint16_t i = 0; i < PROFILER_MAX_CONCURRENT; ++i)
		g_profiler_timers[i].in_use = 0x00;

	g_profiler_entries_cursor = 0x00;

	debug_printf(DEBUG_LOG_INFO, "profiler::initialized");
}

profiler_sample_handle_t profiler_begin(const char* label)
{
	profiler_sample_handle_t handle = -1;

	for (uint16_t i = 0; i < PROFILER_MAX_CONCURRENT; ++i)
	{
		if (g_profiler_timers[i].in_use == 0x00)
		{
			g_profiler_timers[i].in_use = 0x01;
			handle = i;
			break;
		}
	}

	debug_assert(handle >= 0, "profiler::begin - no timers left");

	if (!PROFILER_IS_VALID_HANDLE(handle))
		return handle;

	for (uint16_t i = 0; i < PROFILER_MAX_LABEL_LENGTH - 1; ++i)
	{
		g_profiler_timers[handle].label[i] = label[i];

		if (label[i] == 0x00)
			break;
	}
	g_profiler_timers[handle].label[PROFILER_MAX_LABEL_LENGTH - 1] = 0x00;

	g_profiler_timers[handle].start_time = SDL_GetPerformanceCounter();

	return handle;
}

void profiler_end(profiler_sample_handle_t handle)
{
	if (!PROFILER_IS_VALID_HANDLE(handle))
		return;

	uint64_t elapsed_micro = SDL_GetPerformanceCounter() - g_profiler_timers[handle].start_time;
	elapsed_micro = elapsed_micro * 1000000 / g_frequency;

	for (uint16_t i = 0; i < PROFILER_MAX_LABEL_LENGTH - 1; ++i)
		g_profiler_entries[g_profiler_entries_cursor].label[i] = g_profiler_timers[handle].label[i];
	g_profiler_entries[g_profiler_entries_cursor].value = (uint32_t)elapsed_micro;

	g_profiler_timers[handle].in_use = 0x00;

	++g_profiler_entries_cursor;
	if (g_profiler_entries_cursor >= PROFILER_MAX_ENTRIES_COUNT)
		g_profiler_entries_cursor = 0;
}

uint32_t profiler_entry_cursor(void)
{
	return g_profiler_entries_cursor;
}

profiler_entry_t* profiler_entries(void)
{
	return g_profiler_entries;
}

#endif