
#include "coroutine.h"

#include "common/memory.h"
#include "common/debug/debug.h"

//-----------------------------------------------------------------------------
coroutine_ptr g_active_coroutine;

//-----------------------------------------------------------------------------
coroutine_ptr coroutine_new(coroutine_func_ptr func, void_ptr arg, uint8_t memory_section)
{
	coroutine_ptr coroutine = (coroutine_ptr)memory_allocate(sizeof(coroutine_t), memory_section);
	coroutine->func = func;
	coroutine->arg = arg;

	return coroutine;
}

//-----------------------------------------------------------------------------
void coroutine_delete(coroutine_ptr coroutine)
{
	memory_free(coroutine);
}

//-----------------------------------------------------------------------------
void coroutine_start(coroutine_ptr coroutine)
{
	g_active_coroutine = coroutine;

	int32_t ret = setjmp(coroutine->registers[0]);
	if (!ret)
	{
		uint8_t* frame_pointer = (uint8_t*)(coroutine + 1);
		uint8_t* stack_pointer = frame_pointer - 4;

#ifdef __GBA__
		asm volatile(
			"mov %%r0, %1" "\n\t"
			"mov fp, %0"   "\n\t"
			"mov sp, %%r0"
			: /* no output */
			: "r"(frame_pointer), "r"(stack_pointer)
			: "%r0"
		);
#else
		__asm
		{
			mov eax, stack_pointer

			mov ebp, frame_pointer
			mov esp, eax
		};
#endif

		{
			/*
				Because we just changed the stack we can no longer use the
				above 'coroutine' variable have to ensure the compiler
				creates a new variable for use here (either on new stack or on a register)
			*/

			volatile coroutine_ptr alt_coroutine = g_active_coroutine;

			alt_coroutine->func(alt_coroutine->arg);
		}

		{
			volatile coroutine_ptr alt_coroutine = g_active_coroutine;

			longjmp(alt_coroutine->registers[0], 3);
		}
	}
}

//-----------------------------------------------------------------------------
bool8_t coroutine_next(coroutine_ptr coroutine)
{
	g_active_coroutine = coroutine;

	int32_t ret = setjmp(coroutine->registers[0]);
	if (!ret)
	{
		longjmp(coroutine->registers[1], 2);
	}

	g_active_coroutine = NULL;
	return ret != 3;
}

//-----------------------------------------------------------------------------
void coroutine_yield(void)
{
	debug_assert(g_active_coroutine != NULL, "coroutine:yield called outside of a coroutine");

	coroutine_ptr coroutine = (coroutine_ptr)g_active_coroutine;

	int32_t ret = setjmp(coroutine->registers[1]);
	if (!ret)
	{
		longjmp(coroutine->registers[0], 2);
	}
}

