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

#include "backend/platform.h"
#include <map>
#include <cstring>

#ifdef HAVE_SDL
#	include "backend/SDLbackend.h"
#endif

struct ltstr {
	bool operator()(const char* s1, const char* s2) const {
		return strcmp(s1, s2) < 0;
	}
};

typedef std::map<const char*, PlatformBackend::factory_callback, ltstr> map;
typedef std::pair<const char*, PlatformBackend::factory_callback> pair;
typedef map::iterator iterator;

static map factories;

PlatformBackend* PlatformBackend::factory(const char* name){
	iterator it = factories.find(name);

	if ( it == factories.end() ){
		return NULL;
	}

	return it->second();
}

void PlatformBackend::register_factory(const char* name, factory_callback callback){
	factories.insert(pair(name, callback));
}

void PlatformBackend::register_all(){
#ifdef HAVE_SDL
	SDLBackend::register_factory();
#endif
}
