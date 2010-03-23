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

#include "exception.h"
#include <stdexcept>
#include <cstdarg>
#include <cstdlib>

#include <portable/asprintf.h>

#undef exception

exception::exception(const char* file, unsigned int line, const char* fmt, ...)
	: _file(file)
	, _line(line)
	, _buf(NULL) {

	va_list arg;
	va_start(arg, fmt);
	create_string(fmt, arg);
	va_end(arg);
}

exception::exception(const char* file, unsigned int line, const char* fmt, va_list arg)
	: _file(file)
	, _line(line)
	, _buf(NULL) {

	create_string(fmt, arg);
}

exception::~exception() throw (){
	free(_buf);
}

const char* exception::what() const throw (){
	return _buf;
}

const char* exception::file() const throw(){
	return _file;
}

unsigned int exception::line() const throw(){
	return _line;
}

void exception::create_string(const char* fmt, va_list arg){
	if ( vasprintf(&_buf, fmt, arg) == -1 ){
		free(_buf);
	}
}

ExitException::ExitException(ErrorCode code)
	: _code(code) {
}

ExitException::~ExitException() throw() {

}

ErrorCode ExitException::code() const throw(){
	return _code;
}
