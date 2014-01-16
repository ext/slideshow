/**
 * This file is part of Slideshow.
 * Copyright (C) 2008 David Sveningsson <ext@sidvind.com>
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

#include "module.h"
#include "graphics.h"
#include "Transition.h"
#include "gl.h"
#include "vfade_files.h"

MODULE_INFO("Vertical fade", TRANSITION_MODULE, "David Sveningsson");

void* module_alloc(){
	return malloc(sizeof(transition_module_t));
}

int EXPORT module_init(transition_module_t* module){
	module->render = NULL; /* use default renderer using fsquad */

	/* create shader */
	module->shader = graphics_load_shader(
		SHADER_VERTEX, &vertex_shader,
		SHADER_FRAGMENT, &fragment_shader,
		SHADER_NONE
	);
	if ( !module->shader ) return 1;

	/* setup uniforms */
	module->state_uniform = glGetUniformLocation(module->shader, "s");

	return 0;
}
