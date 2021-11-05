
#include "debug.h"

#include "common/types.h"
#include "common/memory.h"
#include "common/string.h"

#include <stdarg.h>
#include <stdio.h>

//-----------------------------------------------------------------------------
#define REG_DEBUG_ENABLE (vuint16_t*) 0x4FFF780
#define REG_DEBUG_FLAGS (vuint16_t*) 0x4FFF700
#define REG_DEBUG_STRING (char*) 0x4FFF600

#define REG_DEBUG_VAR_FLAGS (vuint16_t*) 0x4FFF500
#define REG_DEBUG_VAR_ADDR_LO (vuint16_t*) 0x4FFF580
#define REG_DEBUG_VAR_ADDR_HI (vuint16_t*) 0x4FFF582

/* mGBA Logging */

//-----------------------------------------------------------------------------
void debug_initialize(void) 
{
	*REG_DEBUG_ENABLE = 0xC0DE;
}

//-----------------------------------------------------------------------------
void debug_shutdown(void) 
{
	*REG_DEBUG_ENABLE = 0;
}

//-----------------------------------------------------------------------------
void debug_printf(uint8_t level, const char* ptr, ...)
{
    level &= 0x7;
    va_list args;
    va_start(args, ptr);
    vsnprintf(REG_DEBUG_STRING, 0x100, ptr, args);
    va_end(args);
    *REG_DEBUG_FLAGS = level | 0x100;
}

//-----------------------------------------------------------------------------
void debug_assert(int32_t condition, const char* message)
{ 
	if (!condition)
	{
		debug_printf(DEBUG_LOG_FATAL, "debug::assert failed - %s", message);
		while (1);
	}
}

//-----------------------------------------------------------------------------
void debug_variable_set(const char* name, uint32_t type, const void_ptr address)
{
#if DEBUG_VARS      //  TEST 
    uint32_t strlen = string_length(name) + 1;
    if (strlen > 0x100)
        strlen = 0x100;

    memory_copy(REG_DEBUG_STRING, (const void_ptr)name, strlen);

    uint32_t address_value = (uint32_t)address;
    *REG_DEBUG_VAR_ADDR_LO = (vuint16_t)address_value;
    *REG_DEBUG_VAR_ADDR_HI = (vuint16_t)(address_value >> 16);
    
    *REG_DEBUG_VAR_FLAGS = 0x01 | (type << 4);
#endif
}

//-----------------------------------------------------------------------------
void debug_variable_unset(const char* name)
{
#if DEBUG_VARS
    uint32_t strlen = string_length(name) + 1;
    if (strlen > 0x100)
        strlen = 0x100;

    memory_copy(REG_DEBUG_STRING, (const void_ptr)name, strlen);

    *REG_DEBUG_VAR_FLAGS = 0x00;
#endif
}
