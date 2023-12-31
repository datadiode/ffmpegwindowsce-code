/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2006 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"

#include "SDL_x11video.h"

/* Functions to be exported */
extern void X11_InitOSKeymap(_THIS);
extern void X11_PumpEvents(_THIS);
extern void X11_SetKeyboardState(Display *display, const char *key_vec);

extern void X11_SaveScreenSaver(Display *display, int *saved_timeout, BOOL *dpms);
extern void X11_DisableScreenSaver(_THIS, Display *display);
extern void X11_RestoreScreenSaver(_THIS, Display *display, int saved_timeout, BOOL dpms);
