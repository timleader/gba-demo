
#include "app_sdl.h"

#include "common/graphics/graphics.h"
#include "common/graphics/overlay.h"

#include "resource.h"

#include <Windows.h>

#include <SDL.h>
#include <SDL_syswm.h>

SDL_Window* lWindow = NULL;
SDL_Renderer* lRenderer;

SDL_Texture* background_texture;
SDL_Texture* sprite_texture;

uint16_t background_texture_width;
uint16_t background_texture_height;


#define WINDOW_WIDTH	720	
#define WINDOW_HEIGHT	480

uint16_t* sdl_background_texture_data;		//	VRAM - store both pages, might be useful for inspection
uint16_t* sdl_sprite_texture_data;

extern uint8_t display_mode;

extern uint16_t g_graphics_palette[256];

void appSDLSetWindowsIcon(void) 
{
	HINSTANCE handle = GetModuleHandle(NULL);
	HICON icon = LoadIcon(handle, IDI_MAIN_ICON);
	if (icon != NULL) 
	{
		SDL_SysWMinfo wminfo;
		SDL_VERSION(&wminfo.version);
		if (SDL_GetWindowWMInfo(lWindow, &wminfo) == 1)
		{
			HWND hwnd = wminfo.info.win.window;
			SetClassLong(hwnd, GCL_HICON, (LONG)(icon));
		}
	}
}

int8_t appSDLInitialize(void)
{
	background_texture_width = 240, background_texture_height = 160;

	uint16_t sprite_texture_width = 240, sprite_texture_height = 160;

	SDL_Init(SDL_INIT_VIDEO);

	lWindow = SDL_CreateWindow("gba-demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (lWindow == NULL)
	{
		return 0;
	}

	appSDLSetWindowsIcon();

	lRenderer = NULL;
	lRenderer = SDL_CreateRenderer(lWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (lRenderer == NULL)
	{
		return 0;
	}

	background_texture = NULL;
	background_texture = SDL_CreateTexture(lRenderer, SDL_PIXELFORMAT_BGR555, SDL_TEXTUREACCESS_STREAMING, background_texture_width, background_texture_height);
	if (background_texture == NULL)
	{
		return 0;
	}

	sprite_texture = NULL;
	sprite_texture = SDL_CreateTexture(lRenderer, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_STREAMING, sprite_texture_width, sprite_texture_height);
	if (sprite_texture == NULL)
	{
		return 0;
	}

	sdl_background_texture_data = (uint16_t*)malloc(background_texture_width * background_texture_height * sizeof(uint16_t));

	sdl_sprite_texture_data = (uint16_t*)malloc(sprite_texture_width * sprite_texture_height * sizeof(uint16_t));

	return 1;
}

void overlay_emulate_gba_present(uint16_t* sdlTextureData);

void appSDLPresent(void)
{
	/*
		Rebuild Background Texture
	*/
	if (g_graphics_context.width != background_texture_width ||
		g_graphics_context.height != background_texture_height)
	{
		if (background_texture != NULL)
		{
			SDL_DestroyTexture(background_texture);
			background_texture = NULL;
		}

		background_texture_width = g_graphics_context.width;
		background_texture_height = g_graphics_context.height;

		background_texture = SDL_CreateTexture(lRenderer, SDL_PIXELFORMAT_BGR555, SDL_TEXTUREACCESS_STREAMING, background_texture_width, background_texture_height);
		if (background_texture == NULL)
		{
			return 0;
		}
	}

	/*
		GBA Render Emulation
	*/
	uint16_t sdlTextureTexelCount = g_graphics_context.width * g_graphics_context.height;
	uint16_t* dest = sdl_background_texture_data;
	uint8_t* src = g_graphics_context.frame_pages[g_graphics_context.page_flip];

	while (sdlTextureTexelCount--)
	{
		*dest++ = g_graphics_palette[*src++];
	}

	memset(sdl_sprite_texture_data, 0, 240 * 160 * sizeof(uint16_t));
	overlay_emulate_gba_present(sdl_sprite_texture_data);	//	overlay should be full resolution !!! 

	/*
		SDL Render
	*/
	SDL_RenderClear(lRenderer);

	{
		SDL_Rect lRect = { 0, 0, background_texture_width, background_texture_height };
		SDL_UpdateTexture(background_texture, &lRect, sdl_background_texture_data, g_graphics_context.width * sizeof(uint16_t));
		SDL_RenderCopy(lRenderer, background_texture, 0, 0);
	}

	{
		SDL_SetTextureBlendMode(sprite_texture, SDL_BLENDMODE_BLEND);

		SDL_Rect lRect = { 0, 0, 240, 160 };
		SDL_UpdateTexture(sprite_texture, &lRect, sdl_sprite_texture_data, 240 * sizeof(uint16_t));
		SDL_RenderCopy(lRenderer, sprite_texture, 0, 0);
	}

	SDL_RenderPresent(lRenderer);
}

void appSDLShutdown(void)
{
	free(sdl_background_texture_data);
	free(sdl_sprite_texture_data);

	if (lRenderer != 0)
	{
		SDL_DestroyRenderer(lRenderer);
	}

	if (lWindow != 0)
	{
		SDL_DestroyWindow(lWindow);
	}

	if (background_texture != 0)
	{
		SDL_DestroyTexture(background_texture);
	}

	if (sprite_texture != 0)
	{
		SDL_DestroyTexture(sprite_texture);
	}

	SDL_Quit();
}