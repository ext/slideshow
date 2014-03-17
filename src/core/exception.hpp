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

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <cstdarg>
#include <cstdlib>

#include "core/asprintf.h"

/**
 * @brief Base exception.
 */
class exception: public std::exception {
	public:
		exception(const char* file, unsigned int line, const char* fmt, ...);
		exception(const char* file, unsigned int line, const char* fmt, va_list arg);
		~exception() throw ();

		virtual const char* what() const throw ();
		virtual const char* file() const throw();
		virtual unsigned int line() const throw();

	private:
		void create_string(const char* fmt, va_list arg);

		const char* _file;
		unsigned int _line;
		char* _buf;

};

#define exception(x, ...) exception(__FILE__, __LINE__, x, ## __VA_ARGS__)

enum ErrorCode {
	NO_ERROR = 0,

	XLIB_ERROR,
	KERNEL_ERROR,
	ARGUMENT_ERROR,
	BROWSER_ERROR,
	IPC_ERROR,
	GRAPHICS_ERROR,
	DAEMON_ERROR,

	DAEMON_UNKNOWN_ERROR = 50,
	DAEMON_UNHANDLED_EXCEPTION,
	DAEMON_FILE_ERROR,
	DAEMON_PID_ERROR,
	DAEMON_SIGNAL_ERROR,

	UNHANDLED_ERROR = 99
};

/**
 * @brief Causes the application to exit.
 * Throw this exception if you want to exit the application but want
 * it to shut down properly.
 * @param code Which error code to exit with.
 */
class ExitException: public std::exception {
	public:
		ExitException(ErrorCode code = NO_ERROR);
		virtual ~ExitException() throw();
		virtual ErrorCode code() const throw();

	private:
		ErrorCode _code;
};

/*
 * verify is like assert but is always compiled.
 */
#define verify(x) \
	do \
		if ( !(x) ){ \
			fprintf(stderr, " %s:%d *** verify failed: %s", __FILE__, __LINE__, #x); \
		} \
	while (0)

#endif // EXCEPTIONS_H
