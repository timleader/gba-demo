
#include "input.h"

#define	REG_BASE	0x04000000
#define REG_KEYINPUT	*(vuint16_t*)(REG_BASE + 0x130)  // Key Input

//---------------------------------------------------------------------------------
typedef struct {
	uint16_t Up,
		Down,
		Held,
		Last,
		DownRepeat;
} KeyInput;

//---------------------------------------------------------------------------------
// Global variables
//---------------------------------------------------------------------------------
static KeyInput Keys = { 0,0,0,0,0 };

static uint8_t delay = 60, repeat = 30, count = 60;

//---------------------------------------------------------------------------------
void setRepeat(int SetDelay, int SetRepeat)
//---------------------------------------------------------------------------------
{
	delay = SetDelay;
	repeat = SetRepeat;
}

//---------------------------------------------------------------------------------
void scanKeys(void)
//---------------------------------------------------------------------------------
{
	Keys.Last = Keys.Held;
	Keys.Held = (REG_KEYINPUT & 0x03ff) ^ 0x03ff; // upper 6 bits clear on hw not emulated


	uint16_t pressed = Keys.Held & (Keys.Last ^ 0x03ff);

	Keys.DownRepeat |= pressed;
	Keys.Down |= pressed;


	uint16_t released = ((Keys.Held ^ 0x03ff) & Keys.Last);

	Keys.Up |= released;

	Keys.Down &= ~released;
	Keys.DownRepeat &= ~released;

	Keys.Up &= ~pressed;

	if (Keys.Last != Keys.Held) count = delay;


	if (delay != 0)
	{
		count--;
		if (count == 0)
		{
			count = repeat;
			Keys.DownRepeat |= Keys.Held;
		}
	}
}

//---------------------------------------------------------------------------------
uint16_t keysDownRepeat(void)
//---------------------------------------------------------------------------------
{
	uint16_t tmp = Keys.DownRepeat;
	Keys.DownRepeat = 0;

	return tmp;
}

//---------------------------------------------------------------------------------
uint16_t keysDown(void)
//---------------------------------------------------------------------------------
{
	uint16_t tmp = Keys.Down;
	Keys.Down = 0;

	return tmp;
}

//---------------------------------------------------------------------------------
uint16_t keysUp(void)
//---------------------------------------------------------------------------------
{
	uint16_t tmp = Keys.Up;
	Keys.Up = 0;

	return tmp;
}

//---------------------------------------------------------------------------------
uint16_t keysHeld(void)
//---------------------------------------------------------------------------------
{
	return Keys.Held;
}



uint16_t __key_curr, __key_prev;

void input_poll(void)
{
	__key_prev = __key_curr;

	scanKeys();

	__key_curr = keysHeld();
}
