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

#include "Log.h"
#include "exception.h"
#include <stdarg.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <time.h>
#include <portable/asprintf.h>
#include <portable/file.h>

#ifdef WIN32
#	include "win32.h"
#endif

Log::Severity Log::_level = Info;
FILE* Log::_file = NULL;

void Log::initialize(const char* filename){
	if ( fopen_s(&_file, filename, "a") != 0 ){
		fprintf(stderr, "Failed to open logfile '%s' ! Fatal error!\n", filename);
		exit(1);
	}
}

void Log::deinitialize(){
	fclose(_file);
}

void Log::message(Severity severity, const char* fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	vmessage(severity, fmt, ap);
	va_end(ap);
}

void Log::vmessage(Severity severity, const char* fmt, va_list ap){
	char buf[255];

	char* line;
	verify( vasprintf(&line, fmt, ap) >= 0 );

	if ( severity >= _level ){
		fprintf(stdout, "(%s) [%s] %s", severity_string(severity), timestring(buf, 255), line);
	}

	fprintf(_file, "(%s) [%s] %s", severity_string(severity), timestring(buf, 255), line);

	free(line);
}

static Log::Severity last_severity;

void Log::message_begin(Severity severity){
	last_severity = severity;

	char buf[255];
	timestring(buf, 255);

	if ( severity >= _level ){
		fprintf(stdout, "(%s) [%s] ", severity_string(severity), buf);
		fflush(stdout);
	}

	fprintf(_file, "(%s) [%s] ", severity_string(severity), buf);
	fflush(_file);
}

void Log::message_ex(const char* str){
	if ( last_severity >= _level ){
		fprintf(stdout, "%s", str);
		fflush(stdout);
	}

	fprintf(_file, "%s", str);
	fflush(_file);
}

void Log::message_ex_fmt(const char* fmt, ...){
	va_list arg;
	va_start(arg, fmt);

	char* line;
	verify( vasprintf(&line, fmt, arg) >= 0 );

	if ( last_severity >= _level ){
		fprintf(stdout, "%s", line);
		fflush(stdout);
	}

	fprintf(_file, "%s", line);
	fflush(_file);

	free(line);
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

void Log::flush(){
	fflush(stdout);
	fflush(_file);
}

int Log::file_no(){
	return fileno(_file);
}
