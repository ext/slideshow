/**
 * This file is part of Slideshow.
 * 
 * Slideshow is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Slideshow is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Slideshow.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GRAPHICS_H
#define GRAPHICS_H

class Transition;

class Graphics {
	public:
		Graphics(int width, int height, bool fullscreen);
		~Graphics();
		
		void render(float state);
		void load_image(const char* filename);
		void set_transition(Transition* transition);
		
	private:
		void swap_textures();
		
		Transition* _transition;
		unsigned int texture_0;
		unsigned int texture_1;
};

#endif // GRAPHICS_H
