
#include "games/7days/states/states.h"

#include "common/utils/coroutine.h"
#include "common/debug/debug.h"
#include "common/memory.h"


//-----------------------------------------------------------------------------
typedef struct st_sandbox_context_s
{
	coroutine_ptr test_coroutine;

} st_sandbox_context_t;

typedef st_sandbox_context_t* st_sandbox_context_ptr;


//-----------------------------------------------------------------------------
void st_sandbox_coroutine(st_sandbox_context_ptr context)
{
	for (int32_t i = 0; i < 10; ++i)
	{
		debug_printf(DEBUG_LOG_DEBUG, "st_sandbox_coroutine cycle=%i", i);

		coroutine_yield();	
	}
}

//-----------------------------------------------------------------------------
void st_sandbox_enter(st_sandbox_context_ptr context, uint32_t parameter)
{
	context->test_coroutine = coroutine_new((coroutine_func_ptr)st_sandbox_coroutine, context, MEMORY_EWRAM);

	coroutine_start(context->test_coroutine);
}

//-----------------------------------------------------------------------------
void st_sandbox_exit(st_sandbox_context_ptr context)
{
	if (context->test_coroutine != NULL)
	{
		coroutine_delete(context->test_coroutine);
		context->test_coroutine = NULL;
	}
}

//-----------------------------------------------------------------------------
void st_sandbox_update(st_sandbox_context_ptr context, fixed16_t t)
{
	if (context->test_coroutine != NULL)
	{
		debug_printf(DEBUG_LOG_DEBUG, "st_sandbox_update");

		bool8_t result = coroutine_next(context->test_coroutine);

		if (result == 0)
		{
			coroutine_delete(context->test_coroutine);
			context->test_coroutine = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_sandbox =
{
	(state_enter_func_ptr)st_sandbox_enter,
	(state_exit_func_ptr)st_sandbox_exit,

	NULL,
	NULL,

	(state_update_func_ptr)st_sandbox_update,
	NULL,

	"st_sandbox",
	sizeof(st_sandbox_context_t)
};