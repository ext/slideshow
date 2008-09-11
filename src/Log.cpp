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

#include "Log.h"
#include <stdarg.h>
#include <cstdlib>
#include <time.h>

Log::Severity Log::_level = Debug;
FILE* Log::_file = NULL;
FILE* Log::_dfile = NULL;

void Log::initialize(const char* filename, const char* debugfilename){
	_file = fopen(filename, "a");
	_dfile = fopen(debugfilename, "a");

	if ( !_file ){
		fprintf(stderr, "Failed to open logfile '%s' ! Fatal error!\n", filename);
		exit(1);
	}

	if ( !_dfile ){
		fprintf(stderr, "Failed to open logfile '%s' ! Fatal error!\n", debugfilename);
		exit(1);
	}
}

void Log::deinitialize(){
	fclose(_file);
	fclose(_dfile);
}

void Log::message(Severity severity, const char* fmt, ...){
#ifdef _NDEBUG
	if ( severity == Debug ){
		return;
	}
#endif

	va_list arg;
	va_start(arg, fmt);

	char buf[255];

	char* line;
	vasprintf(&line, fmt, arg);

	if ( severity >= _level ){
		fprintf(stdout, "(%s) [%s] %s", severity_string(severity), timestring(buf, 255), line);
		fflush(stdout);
	}

	fprintf(_dfile, "(%s) [%s] %s", severity_string(severity), timestring(buf, 255), line);
	fflush(_dfile);

	if ( severity == Fatal ){
		vfprintf(stderr, fmt, arg);
	}

	free(line);
	va_end(arg);
}

char* Log::timestring(char *buffer, int bufferlen) {
	time_t t = time(NULL);
	struct tm* nt;
#ifdef WIN32
	nt = new struct tm;
	localtime_s(nt, &t);
#else
	nt = localtime(&t);
#endif
	strftime(buffer, bufferlen, "%Y-%m-%d %H:%M:%S", nt);
#ifdef WIN32
	delete nt;
#endif
	return buffer;
}

const char* Log::severity_string(Severity severity){
	switch ( severity ){
		case Debug: return "DD";
		case Verbose: return "--";
		case Info: return "  ";
		case Warning: return "WW";
		case Fatal: return "!!";
	}
	return NULL;
}
