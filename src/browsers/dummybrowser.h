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

#ifndef DUMMY_BROWSER_H
#define DUMMY_BROWSER_H

#include "Browser.h"

class DummyBrowser: public Browser {
	public:
		DummyBrowser(const browser_context_t& context);
		virtual ~DummyBrowser();

		virtual char* get_next_file();

	private:
		unsigned int n;
		const char* _img[4];
};

#endif // DUMMY_BROWSER_H
