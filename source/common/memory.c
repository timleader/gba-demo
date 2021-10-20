
#include "memory.h"

#include "common/debug/debug.h"
#include "common/platform/gba/gba.h"

#ifdef _WIN32   
#include <stdlib.h>
#endif

memory_section_t g_memory_sections[3];

#define MEMORY_SYSTEM_ID 0xA3A3

typedef struct memory_block_s memory_block_t;

typedef struct memory_block_s
{ 
    // Whether this block is currently allocated.
    bool16_t flag;

    // Magic number used for error checking. Should equal MALLOC_SYSTEM_ID.
    uint16_t magic;

    // Size of the block (not including this header struct).
    uint32_t size;

    // Previous block pointer. Equals sHeapStart if this is the first block.
    memory_block_t* prev;

    // Next block pointer. Equals sHeapStart if this is the last block.
    memory_block_t* next;

    // Data in the memory block. (Arrays of length 0 are a GNU extension.)
    uint8_t data[0];

} memory_block_t;

typedef memory_block_t* memory_block_ptr;

//  size of memory_block  usage is wrong it should be size - 1;

//  add debug stuff !!! 

void PutMemBlockHeader(void* block, memory_block_ptr prev, memory_block_ptr next, uint32_t size)
{
    memory_block_ptr header = (memory_block_ptr)block;

    header->flag = FALSE;
    header->magic = MEMORY_SYSTEM_ID;
    header->size = size;
    header->prev = prev;
    header->next = next;
}

void PutFirstMemBlockHeader(void* block, uint32_t size)
{
    PutMemBlockHeader(block, (memory_block_t*)block, (memory_block_t*)block, size - sizeof(memory_block_t));
}

void memory_platform_initialize(void);

void memory_initialize(void)
{
    memory_platform_initialize();

    PutFirstMemBlockHeader(g_memory_sections[MEMORY_IWRAM].start, g_memory_sections[MEMORY_IWRAM].size);
    PutFirstMemBlockHeader(g_memory_sections[MEMORY_EWRAM].start, g_memory_sections[MEMORY_EWRAM].size);

    debug_printf(DEBUG_LOG_INFO, "memory::initialized");

    debug_printf(DEBUG_LOG_INFO, "\t::iwram start=0x%x size=%i", g_memory_sections[MEMORY_IWRAM].start, g_memory_sections[MEMORY_IWRAM].size);
    debug_printf(DEBUG_LOG_INFO, "\t::ewram start=0x%x size=%i", g_memory_sections[MEMORY_EWRAM].start, g_memory_sections[MEMORY_EWRAM].size);
}

/*
    memory dump so we can track 
*/

void_ptr memory_allocate(uint32_t size_in_bytes, uint8_t section)
{
    memory_section_t* memory_section = &g_memory_sections[section];
    memory_block_t* pos = (memory_block_t*)memory_section->start;
    memory_block_t* head = pos;
    memory_block_t* splitBlock;
    uint32_t foundBlockSize;

    // Alignment
    if (size_in_bytes & 3)
        size_in_bytes = 4 * ((size_in_bytes / 4) + 1);

#ifdef _WIN32    //  if Windows

    //debug_printf(DEBUG_LOG_INFO, "memory::allocate size_in_bytes=%u, section=%u", size_in_bytes, section);

    if (section == MEMORY_DEVELOPMENT)
    {
        void_ptr allocated_ptr = (void_ptr)malloc(size_in_bytes);
        memory_set(allocated_ptr, 0, size_in_bytes);    //  this is needed as it is assumed for smacker to work 
        return allocated_ptr;
    }

#endif 

    for (;;)
    {
        // Loop through the blocks looking for unused block that's big enough.

        if (!pos->flag)
        {
            foundBlockSize = pos->size;

            if (foundBlockSize >= size_in_bytes)
            {
                if (foundBlockSize - size_in_bytes < 2 * sizeof(memory_block_t))        //  foundBlockSize
                {
                    // The block isn't much bigger than the requested size,
                    // so just use it.
                    pos->flag = TRUE;
                }
                else
                {
                    // The block is significantly bigger than the requested
                    // size, so split the rest into a separate block.
                    foundBlockSize -= sizeof(memory_block_t);
                    foundBlockSize -= size_in_bytes;

                    splitBlock = (memory_block_t*)(pos->data + size_in_bytes);

                    pos->flag = TRUE;
                    pos->size = size_in_bytes;

                    PutMemBlockHeader(splitBlock, pos, pos->next, foundBlockSize);

                    pos->next = splitBlock;

                    if (splitBlock->next != head)
                        splitBlock->next->prev = splitBlock;
                }

                memory_section->used += size_in_bytes;

                return pos->data;
            }
        }

        if (pos->next == head)
        {
            debug_assert(0, "memory::allocate out of memory");
            return NULL;
        }

        pos = pos->next;
    }
}

int8_t memory_determine_section(void_ptr ptr)
{
    uint8_t* typed_ptr = (uint8_t*)ptr;
    for (int8_t section = 0; section < 2; ++section)
    {
        if ((typed_ptr >= (uint8_t*)g_memory_sections[section].start) &&
            (typed_ptr < (uint8_t*)g_memory_sections[section].start + g_memory_sections[section].size))
        {
            return section;
        }
    }

    return -1;
}

void memory_free(void_ptr ptr)  //  double freeing ?? 
{
    if (ptr) 
    {
        int8_t section = memory_determine_section(ptr);

#ifdef _WIN32   
        if (section == -1)
        {
            free(ptr);
            return;
        }
#endif

        memory_section_t* memory_section = &g_memory_sections[section];
        memory_block_ptr head = (memory_block_ptr)memory_section->start;
        memory_block_ptr block = (memory_block_ptr)((uint8_t*)ptr - sizeof(memory_block_t));

        debug_assert(block->magic == MEMORY_SYSTEM_ID, "memory::free memory block header has been corrupted");
        debug_assert(block->flag == TRUE, "memory::free memory block is already free or corrupted");
        block->flag = FALSE;

        memory_section->used -= block->size;

        // If the freed bl ock isn't the last one, merge with the next block
        // if it's not in use.
        if (block->next != head)
        {
            if (!block->next->flag) 
            {
                block->size += sizeof(memory_block_t) + block->next->size;
                block->next->magic = 0;
                block->next = block->next->next;
                if (block->next != head)
                    block->next->prev = block;
            }
        }

        // If the freed block isn't the first one, merge with the previous block
        // if it's not in use.
        if (block != head) 
        {
            if (!block->prev->flag) 
            {
                block->prev->next = block->next;

                if (block->next != head)
                    block->next->prev = block->prev;

                block->magic = 0;
                block->prev->size += sizeof(memory_block_t) + block->size;
            }
        }
    }
}

IWRAM_CODE void memory_copy(void_ptr dest, const void_ptr src, const uint32_t size)
{
    char* csrc = (char*)src;
    char* cdest = (char*)dest;

    for (uint32_t i = 0; i < size; i++)
        *cdest++ = *csrc++;
}

IWRAM_CODE void memory_copy8(void_ptr dest, const void_ptr src, const uint32_t size)
{
    //  is this getting optimized by the compiler ??? 
    //  using volatile to try to enforce 8-bit wide copy 

    vuint8_t* csrc = (vuint8_t*)src;
    vuint8_t* cdest = (vuint8_t*)dest;

    for (uint32_t i = 0; i < size; i++)
        *cdest++ = *csrc++;
}

IWRAM_CODE void memory_copy16(void_ptr dest, const void_ptr src, const uint32_t half_word_count)
{
    debug_assert((uint32_t)dest % 2 == 0, "memory::copy16 dest isn't aligned");
    debug_assert((uint32_t)src % 2 == 0, "memory::copy16 src isn't aligned");

    uint32_t wdN = half_word_count;
    uint16_t* dstw = (uint16_t*)dest, * srcw = (uint16_t*)src;
    // Residual words
    while (wdN--)
        *dstw++ = *srcw++;
}

typedef struct copy32block_s { uint32_t data[8]; } copy32block_t;       //  speed tests for this 

IWRAM_CODE void memory_copy32(void_ptr dst, const void_ptr src, const uint32_t word_count)
{
    //  if src / dest is not 4 byte aligned then assert 
    debug_assert(((uint32_t)dst & 0x03) == 0, "memory::copy32 dest isn't aligned");
    debug_assert(((uint32_t)src & 0x03) == 0, "memory::copy32 src isn't aligned");;

    uint32_t blkN = word_count / 8, wdN = word_count & 7;
    uint32_t* dstw = (uint32_t*)dst, * srcw = (uint32_t*)src;
    if (blkN)
    {
        // 8-word copies
        copy32block_t* dst2 = (copy32block_t*)dst, * src2 = (copy32block_t*)src;
        while (blkN--)
            *dst2++ = *src2++;

        dstw = (uint32_t*)dst2;  srcw = (uint32_t*)src2;
    }
    // Residual words
    while (wdN--)
        *dstw++ = *srcw++;
}

IWRAM_CODE void memory_set(void_ptr dest, const uint32_t byte_count, const uint8_t value)
{
    uint8_t* cdest = (uint8_t*)dest;

    for (uint32_t i = 0; i < byte_count; i++)
        *cdest++ = value;
}

IWRAM_CODE void memory_set32(void_ptr dest, const uint32_t word_count, const uint32_t value)
{
    uint32_t* cdest = (uint32_t*)dest;

    for (uint32_t i = 0; i < word_count; i++)
        *cdest++ = value;
}

uint32_t memory_usage(uint8_t sector)
{
    return g_memory_sections[sector].used;
}

void memory_output_usage(void)
{
    debug_printf(DEBUG_LOG_INFO, "memory::usage");

    debug_printf(DEBUG_LOG_INFO, "\t::iwram used=%u size=%u", g_memory_sections[MEMORY_IWRAM].used, g_memory_sections[MEMORY_IWRAM].size);
    debug_printf(DEBUG_LOG_INFO, "\t::ewram used=%u size=%u", g_memory_sections[MEMORY_EWRAM].used, g_memory_sections[MEMORY_EWRAM].size);
}
