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

#ifndef PLATFORM_H
#define PLATFORM_H

// Derived from blueflower (c) David Sveningsson 2009-2010

#include "core/vector.h"

class PlatformBackend {
	public:
		typedef PlatformBackend* (*factory_callback)();
		static PlatformBackend* factory(const char* name);
		static void register_factory(const char* name, factory_callback callback);
		static void register_all();
		static void register_cleanup();

		PlatformBackend(){}
		virtual ~PlatformBackend(){}

		/**
		 * Initializes the backend.
		 * @return zero if successful
		 */
		virtual int init(const Vector2ui &resolution, bool fullscreen) = 0;

		/**
		 * Cleanup method.
		 */
		virtual void cleanup() = 0;

		/**
		 * Poll backend for updates.
		 */
		virtual void poll(bool& running) = 0;

		/**
		 * Swap buffers.
		 */
		virtual void swap_buffers() const = 0;

		/**
		 * Get current resolution.
		 */
		virtual const Vector2ui& resolution() const { return _resolution; }

		/**
		 * Get center of window.
		 */
		virtual const Vector2ui& center() const { return _center; }

		/**
		 * Lock to mouse to a given position.
		 * @param state Whenever to lock or not.
		 */
		virtual void lock_mouse(bool state) = 0;

	protected:
		void set_resolution(unsigned int w, unsigned int h){
			_resolution = Vector2ui(w, h);
			_center = _resolution / 2;
		}

	private:
		/* Current resolution */
		Vector2ui _resolution;

		/* Center of window */
		Vector2ui _center;
};

#endif // OS_H
