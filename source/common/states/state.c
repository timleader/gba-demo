
#include "state.h"

#include "common/memory.h"
#include "common/debug/debug.h"
#include "common/input/input.h"
#include "common/graphics/graphics.h"

//-----------------------------------------------------------------------------
typedef enum state_op_e
{
	STATE_OP_NOP,
	STATE_OP_GOTO,
	STATE_OP_REPLACE,
	STATE_OP_PUSH,
	STATE_OP_POP,

} state_op_t;

//-----------------------------------------------------------------------------
typedef struct state_command_s
{
	state_op_t operation;

	state_ptr state;
	uint32_t parameter;

} state_command_t;


//-----------------------------------------------------------------------------
typedef struct state_context_s		
{
	uint16_t state_stack_capacity;
	uint16_t state_stack_top;
	state_ptr *state_stack;		//	don't do a pointer

	//	simple bump allocator 
	uint8_t* memory_block_base;
	uint8_t* memory_block_head;		//	pointer to the start of the memory block that the current state can use.. 
	uint8_t* memory_block_end;

	state_command_t pending_command;

} state_context_t;

state_context_t g_state_context;



//-----------------------------------------------------------------------------
void state_initialize(uint8_t state_stack_capacity, uint32_t state_memory_size, uint8_t memory_sector)
{
	debug_printf(DEBUG_LOG_INFO, "state::initialize");

	g_state_context.state_stack_capacity = state_stack_capacity;
	g_state_context.state_stack_top = 0;
	g_state_context.state_stack = memory_allocate(sizeof(state_ptr) * state_stack_capacity, MEMORY_EWRAM);
	g_state_context.state_stack[0] = NULL;

	g_state_context.memory_block_base = memory_allocate(state_memory_size, memory_sector);
	g_state_context.memory_block_head = g_state_context.memory_block_base;
	g_state_context.memory_block_end = g_state_context.memory_block_base + state_memory_size;

	g_state_context.pending_command.operation = STATE_OP_NOP;

	debug_variable_set("state_stack_top", DEBUG_VAR_TYPE_UINT16, &g_state_context.state_stack_top);
	debug_variable_set("state_stack", DEBUG_VAR_TYPE_UNDEFINED, g_state_context.state_stack);
}


//-----------------------------------------------------------------------------
state_ptr state_peek(void)
{
	return g_state_context.state_stack[g_state_context.state_stack_top];
}


//-----------------------------------------------------------------------------
uint8_t state_goto(state_ptr st, uint32_t param)	
{
	g_state_context.pending_command.operation = STATE_OP_GOTO;
	g_state_context.pending_command.state = st;
	g_state_context.pending_command.parameter = param;

	debug_printf(DEBUG_LOG_INFO, "state::goto %s", st->name);

	return 1;
}

//-----------------------------------------------------------------------------
uint8_t state_replace(state_ptr st, uint32_t param)
{
	g_state_context.pending_command.operation = STATE_OP_REPLACE;
	g_state_context.pending_command.state = st;
	g_state_context.pending_command.parameter = param;

	debug_printf(DEBUG_LOG_INFO, "state::replace %s", st->name);

	return 1;
}

//-----------------------------------------------------------------------------
uint8_t state_push(state_ptr st, uint32_t param)
{
	g_state_context.pending_command.operation = STATE_OP_PUSH;
	g_state_context.pending_command.state = st;
	g_state_context.pending_command.parameter = param;

	debug_printf(DEBUG_LOG_INFO, "state::push %s", st->name);

	return 1;
}

//-----------------------------------------------------------------------------
uint8_t state_pop(uint32_t parameter)
{
	debug_assert(g_state_context.pending_command.operation == STATE_OP_NOP, "state::pop operation already pending");

	g_state_context.pending_command.operation = STATE_OP_POP;
	g_state_context.pending_command.state = NULL;
	g_state_context.pending_command.parameter = parameter;

	debug_printf(DEBUG_LOG_INFO, "state::pop");

	return 1;
}

//-----------------------------------------------------------------------------
void_ptr state_context(void)
{
	return g_state_context.memory_block_head;
}

//-----------------------------------------------------------------------------
void state_apply(void)
{
	uint32_t loop_count = 0;

	while (g_state_context.pending_command.operation != STATE_OP_NOP) 
	{
		uint32_t parameter = g_state_context.pending_command.parameter;

		state_ptr prev_state = state_peek();
		state_ptr next_state = g_state_context.pending_command.state;
		state_op_t operation = g_state_context.pending_command.operation;

		g_state_context.pending_command.operation = STATE_OP_NOP;

		switch (operation)
		{
			case STATE_OP_GOTO:
			{
				while (prev_state)
				{
					if (prev_state->exit)
						prev_state->exit(g_state_context.memory_block_head);

					--g_state_context.state_stack_top;
					prev_state = state_peek();

					if (prev_state)
						g_state_context.memory_block_head -= prev_state->memory_size;
				}

				debug_assert(g_state_context.memory_block_head == g_state_context.memory_block_base, "state::memory_block holy shit");
				debug_assert(g_state_context.state_stack_top == 0, "state::stack_top holy shit");

				g_state_context.state_stack[++g_state_context.state_stack_top] = next_state;

				state_ptr current = state_peek();

				debug_assert((g_state_context.memory_block_head + current->memory_size) < g_state_context.memory_block_end, "state::memory_block exceeded");

				if (current && current->enter)
					current->enter(g_state_context.memory_block_head, parameter);

				break;
			}
			case STATE_OP_REPLACE:
			{
				if (prev_state && prev_state->exit)
					prev_state->exit(g_state_context.memory_block_head);

				g_state_context.state_stack[g_state_context.state_stack_top] = next_state;
				state_ptr current = state_peek();

				debug_assert((g_state_context.memory_block_head + current->memory_size) < g_state_context.memory_block_end, "state::memory_block exceeded");

				if (current && current->enter)
					current->enter(g_state_context.memory_block_head, parameter);

				break;
			}
			case STATE_OP_PUSH:
			{
				if (prev_state)
				{
					if (prev_state->pause)
						prev_state->pause(g_state_context.memory_block_head);

					g_state_context.memory_block_head += prev_state->memory_size;
				}

				g_state_context.state_stack[++g_state_context.state_stack_top] = next_state;

				state_ptr current = state_peek();

				debug_assert((g_state_context.memory_block_head + current->memory_size) < g_state_context.memory_block_end, "state::memory_block exceeded");

				if (current && current->enter)
					current->enter(g_state_context.memory_block_head, parameter);

				break;
			}
			case STATE_OP_POP:
			{
				debug_assert(prev_state != NULL, "state:: no state to pop");

				if (prev_state->exit)
					prev_state->exit(g_state_context.memory_block_head);

				--g_state_context.state_stack_top;

				debug_assert(g_state_context.memory_block_head > g_state_context.memory_block_base, "state::memory_block holy fuck");

				state_ptr current = state_peek();

				if (current)
				{
					g_state_context.memory_block_head -= current->memory_size;

					if (current->resume)
						current->resume((void_ptr)g_state_context.memory_block_head, parameter);
				}

				break;
			}
			default:
			{
				debug_assert(0, "state::apply unknown operation");
			}
		}

		debug_assert(loop_count < 4, "state:apply too many immediate state changes");
		++loop_count;

		state_ptr current = state_peek();
		if (current)
		{
			debug_variable_set("state_name", DEBUG_VAR_TYPE_STRING, current->name);
		}
		else
		{
			debug_variable_unset("state_name");
		}
	}
}

//-----------------------------------------------------------------------------
void state_update(void)
{
	state_ptr current = state_peek();

	if (current->update)
		current->update(g_state_context.memory_block_head, fixed16_zero);
}

//-----------------------------------------------------------------------------
void state_draw(void)
{
	state_ptr current = state_peek();

	if (current->draw)
		current->draw(g_state_context.memory_block_head, fixed16_zero);
}

//-----------------------------------------------------------------------------
void st_common_update(void_ptr data, fixed16_t dt)		
{
	if (key_hit(KI_B))
	{
		state_pop(0);
	}
}

//-----------------------------------------------------------------------------
void st_common_draw(void_ptr data, fixed16_t dt)
{
	graphics_pageflip();
}
