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

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "Transition.h"

class Graphics {
	public:
		Graphics(int width, int height, bool fullscreen);
		~Graphics();

		void render(float state);
		void load_image(const char* filename);

		void set_transition(transition_module_t* module);

	private:
		void imageloader_init();
		void imageloader_cleanup();
		void gl_setup();
		void gl_set_matrices();
		void gl_init_textures();
		void gl_cleanup_textures();

		void swap_textures();

		transition_module_t* _transition;
		unsigned int _texture[2];
		int _width;
		int _height;
};

#endif // GRAPHICS_H
