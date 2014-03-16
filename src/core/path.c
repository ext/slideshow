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

#include "path.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <portable/asprintf.h>

#ifdef WIN32
#	include "win32.h"
#endif

char* real_path(const char* filename){
	char* dst;

	if ( filename[0] == '/' || filename[1] == ':' ){
		dst = strdup(filename);
	} else {
		if ( asprintf(&dst, "%s/%s", datapath(), filename) == -1 ){
			fprintf(stderr, "out-of-memory\n");
			return NULL;
		}
	}

	return dst;
}

const char* datapath(){
	const char* path = getenv("SLIDESHOW_DATA_DIR");
	if ( !path ){
		path = DATA_DIR;
	}
	return path;
}

const char* pluginpath(){
	const char* path = getenv("SLIDESHOW_PLUGIN_DIR");
	if ( !path ){
		path = "src/transitions/.libs:" PLUGIN_DIR;
	}
	return path;
}
