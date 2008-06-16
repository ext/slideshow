/**
 * This file is part of Slideshow.
 * Copyright (C) 2008 David Sveningsson <ext@sidvind.com>
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

#ifndef BROWSER_H
#define BROWSER_H

class Browser {
	public:
		Browser(): _bin(1){}
		virtual ~Browser(){}

		virtual const char* get_next_file() = 0;
		virtual void reload() = 0;
		virtual void dump_queue() = 0;

		virtual void change_bin(unsigned int id){ _bin = id; }

	protected:
		unsigned int current_bin(){ return _bin; }

	private:
		unsigned int _bin;
};

#endif // BROWSER_H
