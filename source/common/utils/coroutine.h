
#ifndef COROUTINE_H
#define COROUTINE_H

#include "common/types.h"

#include <setjmp.h>

/*
	Limitation: no support for nested coroutines
*/

//-----------------------------------------------------------------------------
#define COROUTINE_STACK_SIZE 4096	//	debug_printf consumes a crazy amount 

//-----------------------------------------------------------------------------
typedef void(*coroutine_func_ptr)(void_ptr arg);

//-----------------------------------------------------------------------------
typedef struct coroutine_s
{
	coroutine_func_ptr func;
	void_ptr arg;

	jmp_buf registers[2];			

	uint8_t stack[COROUTINE_STACK_SIZE];

} coroutine_t;

typedef coroutine_t* coroutine_ptr;


//-----------------------------------------------------------------------------
coroutine_ptr coroutine_new(coroutine_func_ptr func, void_ptr arg, uint8_t memory_section);

//-----------------------------------------------------------------------------
void coroutine_delete(coroutine_ptr handle);


//-----------------------------------------------------------------------------
void coroutine_start(coroutine_ptr handle);

//-----------------------------------------------------------------------------
bool8_t coroutine_next(coroutine_ptr handle);

//-----------------------------------------------------------------------------
void coroutine_yield(void);	


#endif
