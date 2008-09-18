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

#ifndef BROWSER_H
#define BROWSER_H

typedef struct {
	char* provider;
	char* user;
	char* pass;
	char* host;
	char* name;
} browser_context_t;

browser_context_t get_context(const char* string);
void free_context(browser_context_t& context);

class Browser {
	public:
		Browser(const browser_context_t& context);
		virtual ~Browser();

		virtual const char* get_next_file() = 0;
		virtual void reload() = 0;
		virtual void dump_queue() = 0;

		virtual void change_bin(unsigned int id){ _bin = id; }

		void set_username(const char* username);
		void set_password(const char* password);
		void set_database(const char* database);
		void set_hostname(const char* hostname);

		const char* username(){ return _username; }
		const char* password(){ return _password; }
		const char* database(){ return _database; }
		const char* hostname(){ return _hostname; }

		typedef Browser* (*factory_callback)(const browser_context_t& context);
		static void register_factory(factory_callback callback, const char* name);
		static Browser* factory(const char* string, const char* password);

	protected:
		unsigned int current_bin(){ return _bin; }

	private:
		static void set_string(char*& dst, const char* src);

		unsigned int _bin;

		char* _username;
		char* _password;
		char* _database;
		char* _hostname;
};

#endif // BROWSER_H
