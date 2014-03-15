/**
 * This file is part of Slideshow.
 * Copyright (C) 2008-2010 David Sveningsson <ext@sidvind.com>
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

#ifndef TRANSITION_H
#define TRANSITION_H

#include "module_loader.h"
#include "gl.h"

struct transition_context;
struct transition_module;
typedef struct transition_context* transition_context_t;
typedef struct transition_module* transition_module_t;

typedef void (*render_callback)(transition_module_t transition, transition_context_t context);

struct transition_context {
	unsigned int texture[2];
	float state;                  /* [0,1] 0: current slide fully visible 1: new slide fully visible */
};

struct transition_module {
	struct module_t base;
	render_callback render;       /* function to call when rendering or NULL for default (using fsquad) */

	GLuint state_uniform;         /* uniform location of last loaded shader for this transition (be vary if using multiple shaders) */
	GLuint shader;                /* shader used by default render */
};

#endif // TRANSITION_H
