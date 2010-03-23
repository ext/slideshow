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

#include "Exceptions.h"
#include <cstdlib>
#include <cstdio>
#include <portable/asprintf.h>

BaseException::BaseException(const char* message): std::exception(), _msg(NULL) {
	if ( message ){
		verify( asprintf(&_msg, "%s", message) >= 0 );
	}
}

BaseException::BaseException(const BaseException& e): std::exception(), _msg(NULL) {
	if ( e._msg ){
		verify( asprintf(&_msg, "%s", e._msg) >= 0 );
	}
}

BaseException::~BaseException() throw() {
	free(_msg);
}

const char* BaseException::what() const throw() {
	return _msg;
}

void BaseException::set_message(const char* fmt, va_list va){
	if ( _msg ){
		free(_msg);
	}

	verify( vasprintf(&_msg, fmt, va) >= 0);
}

FatalException::FatalException(ErrorCode code, const char* message): BaseException(message), _code(code){

}

FatalException::FatalException(const FatalException& e): BaseException(e), _code(e._code) {
}

FatalException::~FatalException() throw() {

}

ErrorCode FatalException::code(){
	return _code;
}

ADD_EXCEPTION_IMPLEMENTATION(XlibException, XLIB_ERROR);
ADD_EXCEPTION_IMPLEMENTATION(KernelException, KERNEL_ERROR);
ADD_EXCEPTION_IMPLEMENTATION(ArgumentException, ARGUMENT_ERROR);
ADD_EXCEPTION_IMPLEMENTATION(BrowserException, BROWSER_ERROR);
ADD_EXCEPTION_IMPLEMENTATION(IPCException, IPC_ERROR);
ADD_EXCEPTION_IMPLEMENTATION(GraphicsException, GRAPHICS_ERROR);
ADD_EXCEPTION_IMPLEMENTATION(DaemonException, DAEMON_ERROR);
