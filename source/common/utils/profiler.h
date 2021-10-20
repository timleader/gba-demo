
#ifndef PROFILER_H
#define PROFILER_H

#include "common/types.h"

#define PROFILER_MAX_CONCURRENT	8
#define PROFILER_MAX_ENTRIES_COUNT 64
#define PROFILER_MAX_LABEL_LENGTH 32

typedef int32_t profiler_sample_handle_t;

typedef struct profiler_entry_s
{
	char label[PROFILER_MAX_LABEL_LENGTH];
	uint32_t value;

	uint8_t nested_depth;
	uint8_t reserved[3];

} profiler_entry_t;

/* ---- API ---- */

#define PROFILER_IS_VALID_HANDLE(handle) (handle >= 0)

#ifdef PROFILER_ENABLE

void profiler_initialize(void);

profiler_sample_handle_t profiler_begin(const char* label);		//	if we change the signature of this we can better remove it for release builds 

void profiler_end(profiler_sample_handle_t handle);

uint32_t profiler_entry_cursor(void);

profiler_entry_t* profiler_entries(void);

// counters - number over frame
//	max, avg, min

#else

#define profiler_initialize()

#define profiler_begin(label) (-1)

#define profiler_end(handle)

#define profiler_entry_cursor() (0)

#define profiler_entries() (0)

#endif

//	pull last samples ... 

#endif
