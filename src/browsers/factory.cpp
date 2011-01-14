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

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "Browser.h"
#include "Log.h"
#include <map>
#include <cstring>

struct ltstr {
	bool operator()(const char* s1, const char* s2) const {
		return strcmp(s1, s2) < 0;
	}
};

typedef std::map<const char*, Browser::factory_callback, ltstr> map;
typedef std::pair<const char*, Browser::factory_callback> pair;
typedef map::iterator iterator;

static map factories;

Browser* Browser::factory(const char* string, const char* password){
	Browser* browser = NULL;
	browser_context_t context = get_context(string);

	// If the contex doesn't contain a password and a password was passed from stdin (arg password)
	// we set that as the password in the context.
	if ( !context.pass && password ){
		set_string(context.pass, password);
	}

	iterator it = factories.find(context.provider);

	if ( it == factories.end() ){
		Log::message(Log::Warning, "Unknown database provider '%s'\n", context.provider);
		goto fin;
	}

	browser = it->second(context);

	fin:
	free_context(context);
	return browser;
}

void Browser::register_factory(const char* name, factory_callback callback){
	factories.insert(pair(name, callback));
}

void Browser::register_all(){
#ifdef HAVE_SQLITE3
	void sqlite3_register_factory();
	sqlite3_register_factory();
#endif
}

void Browser::register_cleanup(){

}
