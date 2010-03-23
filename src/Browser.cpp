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

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "Browser.h"
#include "Log.h"
#include <map>
#include <cstring>
#include <cstdlib>

typedef std::map<const char*, Browser::factory_callback> factory_map;
typedef std::pair<const char*, Browser::factory_callback> pair;
typedef factory_map::iterator iterator;

factory_map* factories = NULL;

Browser::Browser(const browser_context_t& context):
	_bin(0),
	_username(NULL),
	_password(NULL),
	_database(NULL),
	_hostname(NULL){

	set_username(context.user);
	set_password(context.pass);
	set_hostname(context.host);
	set_database(context.name);
}

Browser::~Browser(){
	set_username(NULL);
	set_password(NULL);
	set_hostname(NULL);
	set_database(NULL);
}

void Browser::set_username(const char* username){
	set_string(_username, username);
}

void Browser::set_password(const char* password){
	set_string(_password, password);
}

void Browser::set_database(const char* database){
	set_string(_database, database);
}

void Browser::set_hostname(const char* hostname){
	set_string(_hostname, hostname);
}

void Browser::set_string(char*& dst, const char* src){
	free(dst);
	dst = NULL;

	if ( !src ){
		return;
	}

	dst = strdup(src);
}

void Browser::register_factory(factory_callback callback, const char* name){
	if ( !factories ){
		factories = new factory_map;
	}

	factories->insert(pair(name, callback));
}

Browser* Browser::factory(const char* string, const char* password){
	if ( !(factories && string) ){
		return NULL;
	}

	browser_context_t context = get_context(string);

	// If the contex doesn't contain a password and a password was passed from stdin (arg password)
	// we set that as the password in the context.
	if ( !context.pass && password ){
		set_string(context.pass, password);
	}

	for ( iterator it = factories->begin(); it != factories->end(); ++it ){
		if ( strcmp(context.provider, it->first) == 0){
			Browser* browser = it->second(context);
			free_context(context);
			return browser;
		}
	}

	Log::message(Log::Warning, "Unknown database provider '%s'\n", context.provider);

	free_context(context);

	return NULL;
}
