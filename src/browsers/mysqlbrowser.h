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

#ifndef MYSQL_BROWSER_H
#define MYSQL_BROWSER_H

#include "Browser.h"

#ifndef NULL
#	define NULL 0
#endif

class MySQLBrowser: public Browser {
	public:
		MySQLBrowser(const browser_context_t& context);
		virtual ~MySQLBrowser();

		virtual const char* get_next_file();
		virtual void reload();

		virtual void dump_queue();

	private:
		void connect();
		void disconnect();
		struct st_mysql_res* query(const char* str, ...);

		void clear_fields();
		void allocate_fields(unsigned int n);
		void set_field(unsigned int n, const char* str);
		const char* get_field(unsigned int n);

		struct st_mysql *_conn;

		char** _fields;
		unsigned int _nr_of_fields;
		unsigned int _current_field;
};

#endif // MYSQL_BROWSER_H
