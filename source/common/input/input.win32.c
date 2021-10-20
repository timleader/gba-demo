
#pragma once

#include "input.h"

#include <SDL.h>

SDL_Event lEvent;

uint16_t __key_curr, __key_prev;

void input_poll(void)
{
	__key_prev = __key_curr;

	while (SDL_PollEvent(&lEvent))
	{
		switch (lEvent.type)
		{
		case SDL_QUIT:
			_Exit(0);
			break;
		case SDL_KEYDOWN:
			if (lEvent.key.keysym.sym == SDLK_LEFT || lEvent.key.keysym.sym == SDLK_a)
				__key_curr |= KI_LEFT;
			else if (lEvent.key.keysym.sym == SDLK_RIGHT || lEvent.key.keysym.sym == SDLK_d)
				__key_curr |= KI_RIGHT;
			else if (lEvent.key.keysym.sym == SDLK_UP || lEvent.key.keysym.sym == SDLK_w)
				__key_curr |= KI_UP;
			else if (lEvent.key.keysym.sym == SDLK_DOWN || lEvent.key.keysym.sym == SDLK_s)
				__key_curr |= KI_DOWN;
			else if (lEvent.key.keysym.sym == SDLK_q)
				__key_curr |= KI_L;
			else if (lEvent.key.keysym.sym == SDLK_e)
				__key_curr |= KI_R;
			else if (lEvent.key.keysym.sym == SDLK_z)
				__key_curr |= KI_B;
			else if (lEvent.key.keysym.sym == SDLK_x)
				__key_curr |= KI_A;
			else if (lEvent.key.keysym.sym == SDLK_RETURN)
				__key_curr |= KI_START;
			else if (lEvent.key.keysym.sym == SDLK_BACKSPACE)
				__key_curr |= KI_SELECT;
			break;
		case SDL_KEYUP:
			if (lEvent.key.keysym.sym == SDLK_LEFT || lEvent.key.keysym.sym == SDLK_a)
				__key_curr &= ~KI_LEFT;
			else if (lEvent.key.keysym.sym == SDLK_RIGHT || lEvent.key.keysym.sym == SDLK_d)
				__key_curr &= ~KI_RIGHT;
			else if (lEvent.key.keysym.sym == SDLK_UP || lEvent.key.keysym.sym == SDLK_w)
				__key_curr &= ~KI_UP;
			else if (lEvent.key.keysym.sym == SDLK_DOWN || lEvent.key.keysym.sym == SDLK_s)
				__key_curr &= ~KI_DOWN;
			else if (lEvent.key.keysym.sym == SDLK_q)
				__key_curr &= ~KI_L;
			else if (lEvent.key.keysym.sym == SDLK_e)
				__key_curr &= ~KI_R;
			else if (lEvent.key.keysym.sym == SDLK_z)
				__key_curr &= ~KI_B;
			else if (lEvent.key.keysym.sym == SDLK_x)
				__key_curr &= ~KI_A;
			else if (lEvent.key.keysym.sym == SDLK_RETURN)
				__key_curr &= ~KI_START;
			else if (lEvent.key.keysym.sym == SDLK_BACKSPACE)
				__key_curr &= ~KI_SELECT;
			break;
		}
	}
}
