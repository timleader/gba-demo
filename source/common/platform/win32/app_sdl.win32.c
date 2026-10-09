#include "resource.h"
#include <Windows.h>
#include <SDL.h>
#include <SDL_syswm.h>

void appSDLSetWindowsIcon(SDL_Window* window)
{
	HINSTANCE handle = GetModuleHandle(NULL);
	HICON icon = LoadIcon(handle, IDI_MAIN_ICON);
	if (icon != NULL)
	{
		SDL_SysWMinfo wminfo;
		SDL_VERSION(&wminfo.version);
		if (SDL_GetWindowWMInfo(window, &wminfo) == 1)
		{
			HWND hwnd = wminfo.info.win.window;
			SetClassLongPtr(hwnd, GCLP_HICON, (LONG_PTR)icon);
		}
	}
}