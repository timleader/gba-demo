
#ifndef STATE_H
#define STATE_H

#include "common/math/fixed16.h"


//-----------------------------------------------------------------------------
typedef void (*state_enter_func_ptr)(void_ptr data, uint32_t parameter);
typedef void (*state_exit_func_ptr)(void_ptr data);

typedef void (*state_pause_func_ptr)(void_ptr data);
typedef void (*state_resume_func_ptr)(void_ptr data, uint32_t parameter);

typedef void (*state_update_func_ptr)(void_ptr data, fixed16_t dt);		
typedef void (*state_draw_func_ptr)(void_ptr data, fixed16_t dt);


//-----------------------------------------------------------------------------
typedef struct state_s
{
	// instance of state_t void_ptr can be replaced with the st_STATE_context_ptr type 

	state_enter_func_ptr enter;
	state_exit_func_ptr exit;

	state_pause_func_ptr pause;
	state_resume_func_ptr resume;

	state_update_func_ptr update;
	state_draw_func_ptr draw;

	char name[12];
	uint32_t memory_size;

} state_t;

typedef state_t* state_ptr;


//-----------------------------------------------------------------------------
void state_initialize(uint8_t state_stack_capacity, uint32_t state_memory_size, uint8_t memory_sector);

//-----------------------------------------------------------------------------
state_ptr state_peek(void);

//-----------------------------------------------------------------------------
uint8_t state_goto(state_ptr state, uint32_t parameter);
uint8_t state_replace(state_ptr state, uint32_t parameter);
uint8_t state_push(state_ptr state, uint32_t parameter);
uint8_t state_pop(uint32_t parameter);

//-----------------------------------------------------------------------------
void_ptr state_context(void);

//-----------------------------------------------------------------------------
void state_apply(void);
void state_update(void);
void state_draw(void);

//-----------------------------------------------------------------------------
void st_common_update(void_ptr data, fixed16_t dt);
void st_common_draw(void_ptr data, fixed16_t dt);

#endif
