#include "coroutine.h"
#include "common/memory.h"
#include "common/debug/debug.h"
#include <stdlib.h>

coroutine_ptr g_active_coroutine;

static void coroutine_entry(void)
{
	coroutine_ptr coroutine = g_active_coroutine;
	coroutine->func(coroutine->arg);
	coroutine->finished = TRUE;
}

coroutine_ptr coroutine_new(coroutine_func_ptr func, void_ptr arg, uint8_t memory_section)
{
	(void)memory_section;
	coroutine_ptr coroutine = memory_allocate(sizeof(coroutine_t), MEMORY_DEVELOPMENT);
	if (!coroutine)
		abort();
	coroutine->func = func;
	coroutine->arg = arg;
	return coroutine;
}

void coroutine_delete(coroutine_ptr coroutine)
{
	memory_free(coroutine);
}

void coroutine_start(coroutine_ptr coroutine)
{
	if (getcontext(&coroutine->contexts[1]) < 0)
		abort();
	coroutine->finished = FALSE;
	coroutine->contexts[1].uc_stack.ss_sp = coroutine->stack;
	coroutine->contexts[1].uc_stack.ss_size = sizeof(coroutine->stack);
	coroutine->contexts[1].uc_link = &coroutine->contexts[0];
	makecontext(&coroutine->contexts[1], coroutine_entry, 0);
	coroutine_next(coroutine);
}

bool8_t coroutine_next(coroutine_ptr coroutine)
{
	if (coroutine->finished)
		return FALSE;
	g_active_coroutine = coroutine;
	if (swapcontext(&coroutine->contexts[0], &coroutine->contexts[1]) < 0)
		abort();
	g_active_coroutine = NULL;
	return !coroutine->finished;
}

void coroutine_yield(void)
{
	debug_assert(g_active_coroutine != NULL, "coroutine:yield called outside of a coroutine");
	coroutine_ptr coroutine = g_active_coroutine;
	if (swapcontext(&coroutine->contexts[1], &coroutine->contexts[0]) < 0)
		abort();
}