
#include "memory.h"

extern uint8_t __iheap_start[];
extern uint8_t __eheap_start[];

const uint8_t* __iheap_end = (uint8_t*)0x03007FFF;      
const uint8_t* __eheap_end = (uint8_t*)0x0203FFFF; 

extern memory_section_t g_memory_sections[2];

void memory_platform_initialize(void)
{
    const uint32_t alignment = 4;

    uint8_t* iheap_start = &__iheap_start[0];
    iheap_start = (uint8_t*)(((uint32_t)iheap_start + (alignment - 1)) & ~(alignment - 1));

    g_memory_sections[MEMORY_IWRAM].start = iheap_start;
    g_memory_sections[MEMORY_IWRAM].size = __iheap_end - iheap_start;

    uint8_t* eheap_start = &__eheap_start[0];
    eheap_start = (uint8_t*)(((uint32_t)eheap_start + (alignment - 1)) & ~(alignment - 1));

    g_memory_sections[MEMORY_EWRAM].start = eheap_start;
    g_memory_sections[MEMORY_EWRAM].size = __eheap_end - eheap_start;
}

void memory_dma_copy32(void_ptr dest, const void_ptr src, const uint32_t word_count)
{
    typedef struct DMA_REC
    {
        vuint32_t src;
        vuint32_t dst;
        vuint32_t cnt;
    } DMA_REC;

    #define REG_DMA ((volatile DMA_REC*)0x040000B0)
    #define DMA_32			0x04000000	//!< Transfer by word
    #define DMA_ON (1 << 31)

    REG_DMA[3].cnt = 0; // shut off any previous transfer
    REG_DMA[3].src = (vuint32_t)src;
    REG_DMA[3].dst = (vuint32_t)dest;
    REG_DMA[3].cnt = (word_count) | DMA_32 | DMA_ON;
}
