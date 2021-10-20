
#include "common/graphics/graphics.h"
#include "common/graphics/overlay.h"
#include "common/audio/audio.h"
#include "common/video/smacker.h"
#include "common/resources/resources.h"
#include "common/states/state.h"
#include "common/utils/profiler.h"
#include "common/input/input.h"
#include "common/utils/timer.h"
#include "common/memory.h"
#include "common/debug/debug.h"
#include "common/stringstore.h"
#include "common/states/debug/states.h"
#include "common/savegame/savegame.h"

#include "games/7days/states/states.h"
#include "games/7days/itemstore.h"


//---------------------------------------------------------------------------------
// Program entry point
//-------------------------------------- -------------------------------------------
int main(void)
{
	//	Engine Initialization

	//	maybe a device_initialize()
	//		initialize interrupt 
	irqInit();

	debug_initialize();
	memory_initialize();
	profiler_initialize();

	resources_initialize();

	graphics_initialize();
	overlay_initialize();
	audio_initialize();

	stringstore_initialize(LOCALE_EN_GB);
	itemstore_initialize();
	savegame_initialize();

	state_initialize(4, 1024, MEMORY_EWRAM);
	state_goto(&st_splash, 0);

	profiler_sample_handle_t profile_handle_01, profile_handle_02;
	uint32_t ewram_used = 0, iwram_used = 0;

	while (1)
	{

		profile_handle_02 = profiler_begin("empty:sample");	
		profiler_end(profile_handle_02);

		profile_handle_02 = profiler_begin("frame");	//	841,105   -- just hitting 20FPS 

		/*
			280,896 cycles @ 60Hz
			561,792 cycels @ 30Hz		-- aim for this --- might actually be able to hit this 
			842,688 cycels @ 20Hz		-- just reaching this, most of the time  
		*/

		profile_handle_01 = profiler_begin("state:apply");	//	264 cycles		-- this is just a func call and return, WTF 

		state_apply();

		profiler_end(profile_handle_01);


		profile_handle_01 = profiler_begin("input:poll");		//	690 cycles

		input_poll();

		profiler_end(profile_handle_01);


		profile_handle_01 = profiler_begin("state:update");	//	15,550 cycles

		state_update();

		profiler_end(profile_handle_01);


		profile_handle_01 = profiler_begin("sound:update");	//	159,594 cycles

		audio_update();

		profiler_end(profile_handle_01);


		profile_handle_01 = profiler_begin("state:draw");		//	409,230 cycles

		state_draw();

		profiler_end(profile_handle_01);


		profile_handle_01 = profiler_begin("graphics:vsync");	//	~~~  226,213 cycles

		graphics_vsync();			

		profiler_end(profile_handle_01);


		profile_handle_01 = profiler_begin("graphics:present");	//	3,364 cycles

		graphics_present();

		profiler_end(profile_handle_01);


		profile_handle_01 = profiler_begin("overlay:present");	//	270 cycles 

		overlay_present();

		profiler_end(profile_handle_01);

		if (memory_usage(MEMORY_EWRAM) != ewram_used ||
			memory_usage(MEMORY_IWRAM) != iwram_used)
		{
			iwram_used = memory_usage(MEMORY_IWRAM);
			ewram_used = memory_usage(MEMORY_EWRAM);

			memory_output_usage();
		}

		profiler_end(profile_handle_02);
	}

	stringstore_shutdown();

	//	Engine Shutdown

	audio_shutdown();
	overlay_shutdown();
	graphicsShutdown();

	resources_shutdown();

}
