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

#ifndef FLATFILE_BROWSER_H
#define FLATFILE_BROWSER_H

#include "Browser.h"

class FlatFileBrowser: public Browser {
	public:
		FlatFileBrowser(const char* filename);
		virtual ~FlatFileBrowser();

		virtual const char* get_next_file();

	private:
		void read_file();
		void clear_records();
		void get_record(unsigned int n);

		const char* _filename;
		const char** _records
		unsigned int _index;
		unsigned int _record_cnt;
};

#endif // FLATFILE_BROWSER_H
