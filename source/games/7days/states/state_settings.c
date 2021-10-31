
#include "games/7days/states/states.h"

#include "common/input/input.h"


//-----------------------------------------------------------------------------
typedef struct st_settings_context_s
{
	uint32_t reserved;

} st_settings_context_t;

typedef st_settings_context_t* st_settings_context_ptr;


//-----------------------------------------------------------------------------
void st_settings_enter(st_settings_context_ptr context, uint32_t parameter)
{
}

//-----------------------------------------------------------------------------
void st_settings_exit(st_settings_context_ptr context)
{
}

//-----------------------------------------------------------------------------
void st_settings_update(st_settings_context_ptr context, fixed16_t dt)
{

	/*
		
		palette mode { backlit, not backlit }
		credits .. 
	
	*/


	if (key_hit(KI_B))
	{
		state_pop(0);
	}
}

//-----------------------------------------------------------------------------
EWRAM_DATA state_t st_settings =
{
	(state_enter_func_ptr)st_settings_enter,
	(state_exit_func_ptr)st_settings_exit,

	NULL,
	NULL,

	(state_update_func_ptr)st_settings_update,
	NULL,

	"st_settings",
	sizeof(st_settings_context_t)

};


//	credits, 
//	clear saves 
//	color mode { normal, backlit } 
