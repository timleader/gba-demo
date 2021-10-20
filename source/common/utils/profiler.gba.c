
#include "profiler.h"

#include "common/debug/debug.h"

#ifdef PROFILER_ENABLE

typedef volatile uint16_t vuint16_t;

//! \name Timer registers
//\{
#define REG_BASE		0x04000000

#define TM_ENABLE		0x0080	//!< Enable timer
#define TM_CASCADE		0x0004	//!< Increment when preceding timer overflows

#define REG_TM2D			*(vuint16_t*)(REG_BASE+0x0108)	//!< Timer 2 data
#define REG_TM2CNT			*(vuint16_t*)(REG_BASE+0x010A)	//!< Timer 2 control
#define REG_TM3D			*(vuint16_t*)(REG_BASE+0x010C)	//!< Timer 3 data
#define REG_TM3CNT			*(vuint16_t*)(REG_BASE+0x010E)	//!< Timer 3 control
//\}

//	Timer0 + Timer1 is used for DirectSound, 
//	We can only use Timer2 + Timer3 here 

typedef struct profiler_timer_s
{
	char label[PROFILER_MAX_LABEL_LENGTH];
	uint32_t start_cycles;

	uint8_t flags;
	uint8_t reserved[3];

} profiler_perf_timer_t;

profiler_perf_timer_t g_profiler_timers[PROFILER_MAX_CONCURRENT];

profiler_entry_t g_profiler_entries[PROFILER_MAX_ENTRIES_COUNT];
uint32_t g_profiler_entries_cursor;

/* ---- API ---- */

//  60Hz => 280,896 cycles per frame
//	30Hz => 561,792 cycles per frame
//	20Hz => 842,688 cycles per frame

void profiler_initialize(void)
{
	for (uint16_t i = 0; i < PROFILER_MAX_CONCURRENT; ++i)
		g_profiler_timers[i].flags = 0x00;

	g_profiler_entries_cursor = 0x00;

	//	Start the GBA Timer, this will wrap every now and then, but this will be so 
	//		infrequent that I don't believe we need to do anything special to handle this 

	REG_TM2D = 0; REG_TM3D = 0;
	REG_TM2CNT = 0; REG_TM3CNT = 0;
	REG_TM3CNT = TM_ENABLE | TM_CASCADE;
	REG_TM2CNT = TM_ENABLE;
}

profiler_sample_handle_t profiler_begin(const char* label)
{
	profiler_sample_handle_t handle = -1;

	for (uint16_t i = 0; i < PROFILER_MAX_CONCURRENT; ++i)
	{
		if (g_profiler_timers[i].flags == 0x00)
		{
			g_profiler_timers[i].flags = 0x01;
			handle = i;
			break;
		}
	}

	debug_assert(handle >= 0, "profiler::begin - no timers left");

	for (uint16_t i = 0; i < PROFILER_MAX_LABEL_LENGTH - 1; ++i)
	{
		g_profiler_timers[handle].label[i] = label[i];

		if (label[i] == 0x00)
			break;
	}
	g_profiler_timers[handle].label[PROFILER_MAX_LABEL_LENGTH - 1] = 0x00;

	uint32_t cycles = (REG_TM3D << 16) | REG_TM2D;
	g_profiler_timers[handle].start_cycles = cycles;

	return handle;
}

void profiler_end(profiler_sample_handle_t handle)
{
	debug_assert(handle >= 0, "profiler::end - invalid");
	debug_assert(handle < PROFILER_MAX_CONCURRENT, "profiler::end - invalid");

	uint32_t end_cycles = (REG_TM3D << 16) | REG_TM2D;
	uint32_t cycles = end_cycles - g_profiler_timers[handle].start_cycles;

	g_profiler_timers[handle].flags = 0x00;

	for (uint16_t i = 0; i < PROFILER_MAX_LABEL_LENGTH - 1; ++i)
		g_profiler_entries[g_profiler_entries_cursor].label[i] = g_profiler_timers[handle].label[i];
	g_profiler_entries[g_profiler_entries_cursor].value = cycles;
	g_profiler_entries[g_profiler_entries_cursor].nested_depth = handle;	//	handle will implicitly be the nested depth of the profiler entry .

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
