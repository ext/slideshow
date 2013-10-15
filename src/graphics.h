/**
 * This file is part of Slideshow.
 * Copyright (C) 2008-2012 David Sveningsson <ext@sidvind.com>
 *
 * Slideshow is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Slideshow is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Slideshow.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SLIDESHOW_GRAPHICS_H
#define SLIDESHOW_GRAPHICS_H

#include "Transition.h"
#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif

int graphics_init(int width, int height);
int graphics_cleanup();
void graphics_render(float state);
void graphics_swap_textures();
int graphics_load_image(const char* filename, int letterbox);
int graphics_set_transition(const char* name, transition_module_t** mod);

#ifdef __cplusplus
}
#endif

#endif /* SLIDESHOW_GRAPHICS_H */
