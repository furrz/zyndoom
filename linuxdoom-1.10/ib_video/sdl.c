/******************************************************************************

   Copyright (C) 1993-1996 by id Software, Inc.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   DESCRIPTION:
        DOOM graphics stuff for SDL2.

******************************************************************************/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"

#include "../doomstat.h"
#include "../i_system.h"
#include "../v_video.h"
#include "../m_argv.h"
#include "../d_main.h"

#include "../doomdef.h"

#include "../ib_video.h"


static SDL_Window *window;
static SDL_Surface *surface;

static void (*output_size_changed_callback)(size_t width, size_t height);

static int SDLKeyToNative(const SDL_Keycode keycode, const SDL_Scancode scancode)
{
	switch (keycode)
	{
		case SDLK_LEFT:   return KEY_LEFTARROW;
		case SDLK_RIGHT:  return KEY_RIGHTARROW;
		case SDLK_DOWN:   return KEY_DOWNARROW;
		case SDLK_UP:     return KEY_UPARROW;
		case SDLK_ESCAPE: return KEY_ESCAPE;
		case SDLK_RETURN: return KEY_ENTER;
		case SDLK_TAB:    return KEY_TAB;
		case SDLK_F1:     return KEY_F1;
		case SDLK_F2:     return KEY_F2;
		case SDLK_F3:     return KEY_F3;
		case SDLK_F4:     return KEY_F4;
		case SDLK_F5:     return KEY_F5;
		case SDLK_F6:     return KEY_F6;
		case SDLK_F7:     return KEY_F7;
		case SDLK_F8:     return KEY_F8;
		case SDLK_F9:     return KEY_F9;
		case SDLK_F10:    return KEY_F10;
		case SDLK_F11:    return KEY_F11;
		case SDLK_F12:    return KEY_F12;

		case SDLK_BACKSPACE:
		case SDLK_DELETE: return KEY_BACKSPACE;

		case SDLK_PAUSE:  return KEY_PAUSE;

		case SDLK_LSHIFT:
		case SDLK_RSHIFT: return KEY_RSHIFT;

		case SDLK_LCTRL:
		case SDLK_RCTRL:  return KEY_RCTRL;

		case SDLK_LALT:
		case SDLK_RALT:
		case SDLK_LGUI:
		case SDLK_RGUI:
			return KEY_RALT;

		default:
			break;
	}

	switch(scancode)
	{
		case SDL_SCANCODE_EQUALS: return KEY_EQUALS;
		case SDL_SCANCODE_MINUS: return KEY_MINUS;
		case SDL_SCANCODE_SPACE: return ' ';
		case SDL_SCANCODE_LEFTBRACKET: return '[';
		case SDL_SCANCODE_RIGHTBRACKET: return ']';
		case SDL_SCANCODE_BACKSLASH: return '\\';
		case SDL_SCANCODE_SEMICOLON: return ';';
		case SDL_SCANCODE_APOSTROPHE: return '\'';
		case SDL_SCANCODE_GRAVE: return '`';
		case SDL_SCANCODE_COMMA: return ',';
		case SDL_SCANCODE_PERIOD: return '.';
		case SDL_SCANCODE_SLASH: return '/';
		case SDL_SCANCODE_0: return '0';
		case SDL_SCANCODE_1: return '1';
		case SDL_SCANCODE_2: return '2';
		case SDL_SCANCODE_3: return '3';
		case SDL_SCANCODE_4: return '4';
		case SDL_SCANCODE_5: return '5';
		case SDL_SCANCODE_6: return '6';
		case SDL_SCANCODE_7: return '7';
		case SDL_SCANCODE_8: return '8';
		case SDL_SCANCODE_9: return '9';
		case SDL_SCANCODE_A: return 'a';
		case SDL_SCANCODE_B: return 'b';
		case SDL_SCANCODE_C: return 'c';
		case SDL_SCANCODE_D: return 'd';
		case SDL_SCANCODE_E: return 'e';
		case SDL_SCANCODE_F: return 'f';
		case SDL_SCANCODE_G: return 'g';
		case SDL_SCANCODE_H: return 'h';
		case SDL_SCANCODE_I: return 'i';
		case SDL_SCANCODE_J: return 'j';
		case SDL_SCANCODE_K: return 'k';
		case SDL_SCANCODE_L: return 'l';
		case SDL_SCANCODE_M: return 'm';
		case SDL_SCANCODE_N: return 'n';
		case SDL_SCANCODE_O: return 'o';
		case SDL_SCANCODE_P: return 'p';
		case SDL_SCANCODE_Q: return 'q';
		case SDL_SCANCODE_R: return 'r';
		case SDL_SCANCODE_S: return 's';
		case SDL_SCANCODE_T: return 't';
		case SDL_SCANCODE_U: return 'u';
		case SDL_SCANCODE_V: return 'v';
		case SDL_SCANCODE_W: return 'w';
		case SDL_SCANCODE_X: return 'x';
		case SDL_SCANCODE_Y: return 'y';
		case SDL_SCANCODE_Z: return 'z';

		default:
			break;
	}

	return -1;
}


/* IB_StartTic */
void IB_StartTic (void)
{
	event_t event;
	static int button_state;
	SDL_Event sdl_event;

	/* calculate framebuffer scale for in-game mouse positioning */
	int window_width, window_height;
	SDL_GetWindowSize(window, &window_width, &window_height);
	const float fb_scale = (float)SCREENHEIGHT / (float)window_height;

	while (SDL_PollEvent(&sdl_event))
	{
		switch (sdl_event.type)
		{
			case SDL_QUIT:
				I_Quit();
				break;

			case SDL_KEYDOWN:
				event.type = ev_keydown;
				event.data1 = SDLKeyToNative(sdl_event.key.keysym.sym, sdl_event.key.keysym.scancode);
				if (event.data1 != -1)
					D_PostEvent(&event);
				/* I_Info("k"); */
				break;
			case SDL_KEYUP:
				event.type = ev_keyup;
				event.data1 = SDLKeyToNative(sdl_event.key.keysym.sym, sdl_event.key.keysym.scancode);
				if (event.data1 != -1)
					D_PostEvent(&event);
				/* I_Info("ku"); */
				break;
			case SDL_MOUSEBUTTONDOWN:
				switch (sdl_event.button.button)
				{
					case SDL_BUTTON_LEFT:
						button_state |= 1;
						break;
					case SDL_BUTTON_RIGHT:
						button_state |= 2;
						break;
					case SDL_BUTTON_MIDDLE:
						button_state |= 4;
						break;
				}
				event.type = ev_mouse;
				event.data1 = button_state;
				event.data2 = event.data3 = 0;
				event.data4 = sdl_event.button.x * fb_scale;
				event.data5 = sdl_event.button.y * fb_scale;
				D_PostEvent(&event);
				/* I_Info("b"); */
				break;
			case SDL_MOUSEBUTTONUP:
				switch (sdl_event.button.button)
				{
					case SDL_BUTTON_LEFT:
						button_state &= ~1;
						break;
					case SDL_BUTTON_RIGHT:
						button_state &= ~2;
						break;
					case SDL_BUTTON_MIDDLE:
						button_state &= ~4;
						break;
				}
				event.type = ev_mouse;
				event.data1 = button_state;
				event.data2 = event.data3 = 0;
				event.data4 = sdl_event.button.x * fb_scale;
				event.data5 = sdl_event.button.y * fb_scale;
				D_PostEvent(&event);
				/* I_Info("bu"); */
				break;
			case SDL_MOUSEMOTION:
				event.type = ev_mouse;
				event.data1 = button_state;
				event.data2 = sdl_event.motion.xrel * (1 << 5);
				event.data3 = -sdl_event.motion.yrel * (1 << 5);
				event.data4 = sdl_event.motion.x * fb_scale;
				event.data5 = sdl_event.motion.y * fb_scale;
				if (event.data2 || event.data3)
				{
					D_PostEvent(&event);
					/* I_Info("m"); */
				}
				break;
			case SDL_WINDOWEVENT:
				switch (sdl_event.window.event)
				{
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						surface = SDL_GetWindowSurface(window);
						/* Clear surface of any garbage pixels. */
						SDL_FillRect(surface, NULL, 0);
						output_size_changed_callback(surface->w, surface->h);
						break;
				}
				break;

		}
	}
}


void IB_GetFramebuffer(unsigned char **pixels, size_t *pitch)
{
	SDL_LockSurface(surface);

	*pixels = (unsigned char*)surface->pixels;
	*pitch = surface->pitch;
}


/* IB_FinishUpdate */
void IB_FinishUpdate (void)
{
	SDL_UnlockSurface(surface);
	SDL_UpdateWindowSurface(window);
}


void IB_GetColor(unsigned char *bytes, unsigned char red, unsigned char green, unsigned char blue)
{
	unsigned int i;

	const Uint32 color = SDL_MapRGB(surface->format, red, green, blue);

	for (i = 0; i < surface->format->BytesPerPixel; ++i)
		bytes[i] = (color >> (i * 8)) & 0xFF;
}


void IB_InitGraphics(const char *title, size_t screen_width, size_t screen_height, size_t *bytes_per_pixel, void (*output_size_changed_callback_p)(size_t width, size_t height))
{
	output_size_changed_callback = output_size_changed_callback_p;

	/* Enable high-DPI support on Windows because SDL2 is bad at being a platform abstraction library. */
	SDL_SetHint(SDL_HINT_WINDOWS_DPI_SCALING, "1");
	SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

	if (window == NULL)
		I_Error("Could not create SDL window");

	surface = SDL_GetWindowSurface(window);

	*bytes_per_pixel = surface->format->BytesPerPixel;

	output_size_changed_callback(surface->w, surface->h);
}


void IB_ShutdownGraphics(void)
{
	SDL_FreeSurface(surface);
	SDL_DestroyWindow(window);
	SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
}


void IB_GrabMouse(d_bool grab)
{
	SDL_SetRelativeMouseMode(grab ? SDL_TRUE : SDL_FALSE);
}

void IB_ToggleFullscreen(void)
{
	static d_bool fullscreen;

	fullscreen = !fullscreen;
	SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}
