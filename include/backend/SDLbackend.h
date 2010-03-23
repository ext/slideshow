/**
 * This file is part of Slideshow.
 * Copyright (C) 2008-2010 David Sveningsson <ext@sidvind.com>
 *                         Pernilla Sveningsson <estel@sidvind.com>
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

#ifndef SDLBACKEND_H
#define SDLBACKEND_H

#include "backend/platform.h"
#include <SDL/SDL.h>

class SDLBackend:public PlatformBackend {
	public:
		static void register_factory();

		SDLBackend();
		virtual ~SDLBackend();

		virtual int init(const Vector2ui &resolution, bool fullscreen);
		virtual void cleanup();

		virtual void poll(bool& running);

		virtual void swap_buffers() const;

		virtual void lock_mouse(bool state);

	private:
		bool _lock;
		bool _fullscreen;
};

#endif /* SDLBACKEND_H */
