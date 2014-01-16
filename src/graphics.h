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

enum shader_spec_t {
	SHADER_NONE = 0,
	SHADER_VERTEX,
	SHADER_FRAGMENT,
};

int graphics_init(int width, int height);
int graphics_cleanup();
void graphics_render(float state);
void graphics_swap_textures();
int graphics_load_image(const char* filename, int letterbox);
int graphics_set_transition(const char* name, transition_module_t** mod);

/**
 * Load a shader.
 *   GLint sp = graphics_load_shader(SHADER_VERTEX, &datapack_handle, SHADER_FRAGMENT, &datapack_handle, SHADER_NONE);
 *
 * The shader will stay loaded, manually call glUseProgram(..) to unload/change.
 *
 * @return Non-zero if successful, 0 on errors (which is written to log).
 */
int graphics_load_shader(enum shader_spec_t spec, ...);

/**
 * Transition helper: render a fullscreen quad.
 * Use together with "fsquad.vert". Assumes caller binds shader before render.
 */
void graphics_render_fsquad();

#ifdef __cplusplus
}
#endif

#endif /* SLIDESHOW_GRAPHICS_H */
