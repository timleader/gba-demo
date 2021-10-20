
#ifndef INPUT_H
#define INPUT_H


#include "common/types.h"

typedef enum eKeyIndex
{
	KI_A			= (1 << 0),
	KI_B			= (1 << 1),
	KI_SELECT		= (1 << 2),
	KI_START		= (1 << 3),
	KI_RIGHT		= (1 << 4),
	KI_LEFT			= (1 << 5),
	KI_UP			= (1 << 6),
	KI_DOWN			= (1 << 7),
	KI_R			= (1 << 8), 
	KI_L			= (1 << 9)
} eKeyIndex;

extern uint16_t __key_curr, __key_prev;

#define inline __inline

//! Get current keystate
inline uint16_t key_curr_state(void) { return __key_curr; }

//! Get previous key state
inline uint16_t key_prev_state(void) { return __key_prev; }

//! Gives the keys of \a key that are currently down
inline uint16_t key_is_down(uint16_t key) { return  __key_curr & key; }

//! Gives the keys of \a key that are currently up
inline uint16_t key_is_up(uint16_t key) { return ~__key_curr & key; }

//! Gives the keys of \a key that were previously down
inline uint16_t key_was_down(uint16_t key) { return  __key_prev & key; }

//! Gives the keys of \a key that were previously down
inline uint16_t key_was_up(uint16_t key) { return ~__key_prev & key; }

//! Gives the keys of \a key that are different from before
inline uint16_t key_transit(uint16_t key)
{
	return (__key_curr ^ __key_prev)& key;
}

//! Gives the keys of \a key that are being held down
inline uint16_t key_held(uint16_t key)
{
	return (__key_curr & __key_prev)& key;
}

//! Gives the keys of \a key that are pressed (down now but not before)
inline uint16_t key_hit(uint16_t key)
{
	return (__key_curr & ~__key_prev)& key;
}

//! Gives the keys of \a key that are being released
inline uint16_t key_released(uint16_t key)
{
	return (~__key_curr & __key_prev)& key;
}

void input_poll(void);

/*


current , previous input state

input_poll()

INLINE u32 key_is_down(u32 key);
INLINE u32 key_is_up(u32 key);

INLINE u32 key_was_down(u32 key);
INLINE u32 key_was_up(u32 key);

INLINE u32 key_transit(u32 key);
INLINE u32 key_held(u32 key);
INLINE u32 key_hit(u32 key);
INLINE u32 key_released(u32 key);

input system, is input going to fill laggy if we don't poll this at 60Hz

	up
	down
	left
	right

	a,
	b,

	L,
	R,

	select,
	start



	TODO:
		input
		states

 */

#endif
