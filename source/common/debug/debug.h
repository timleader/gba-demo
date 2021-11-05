
#ifndef DEBUG_H
#define DEBUG_H

#include "common/types.h"

#define DEBUG_LOG_FATAL 0
#define DEBUG_LOG_ERROR 1
#define DEBUG_LOG_WARN 2
#define DEBUG_LOG_INFO 3
#define DEBUG_LOG_DEBUG 4

//#define logging_printf(...)

#define DEBUG_VAR_TYPE_UNDEFINED	0
#define DEBUG_VAR_TYPE_UINT32		1
#define DEBUG_VAR_TYPE_UINT16		2
#define DEBUG_VAR_TYPE_UINT8		3
#define DEBUG_VAR_TYPE_STRING		4

#if DEBUG 

	void debug_initialize(void);

	void debug_shutdown(void);

	void debug_printf(uint8_t level, const char* format, ...);		//	refactor to debug_printf(...)

	void debug_assert(int32_t condition, const char* message);	//	if release remove this function 


	void debug_variable_set(const char* name, uint32_t type, const void_ptr address);

	void debug_variable_unset(const char* name);

#else


#define debug_initialize()

#define debug_shutdown()

#define debug_printf(...)

#define debug_assert(...)	

#define debug_variable_set(...)	

#define debug_variable_unset(...)	

#endif


#endif
