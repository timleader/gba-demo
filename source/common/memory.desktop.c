
#include "memory.h"

#include "common/debug/debug.h"

//  option to increase heaps for development purpose

#define MEMORY_IWRAM_HEAP_SIZE 0x004000		// reduced to 16 KB 
#define MEMORY_EWRAM_HEAP_SIZE 0x040000		// reduced to 128 KB as don't know how much space the program takes yet..

uint8_t g_IWRAM_Heap[MEMORY_IWRAM_HEAP_SIZE];
uint8_t g_EWRAM_Heap[MEMORY_EWRAM_HEAP_SIZE];

extern memory_section_t g_memory_sections[2];

void memory_platform_initialize(void)
{
    g_memory_sections[MEMORY_IWRAM].start = &g_IWRAM_Heap[0];
    g_memory_sections[MEMORY_IWRAM].size = MEMORY_IWRAM_HEAP_SIZE;

    g_memory_sections[MEMORY_EWRAM].start = &g_EWRAM_Heap[0];
    g_memory_sections[MEMORY_EWRAM].size = MEMORY_EWRAM_HEAP_SIZE;
}

void memory_dma_copy32(void_ptr dest, const void_ptr src, const uint32_t word_count)
{
    memory_copy32(dest, src, word_count);
}