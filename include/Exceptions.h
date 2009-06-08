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

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <cstdarg>

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
 * @brief Base exception.
 * You should never throw this specifically but rather catch it
 * when you wish to catch all exceptions.
 *
 * @param message The error message.
 */
class BaseException: public std::exception {
	public:
		BaseException(const char* message = NULL);
		BaseException(const BaseException& e);
		virtual ~BaseException() throw();

		virtual const char* what() const throw();

	protected:
		void set_message(const char* fmt, va_list va);

	private:
		char* _msg;
};

/**
 * @brief Fatal exceptions will cause the application to terminate.
 * The application will terminate but will try to cleanup.
 *
 * @param message The error message.
 * @param code The error code which will be returned.
 */
class FatalException: public BaseException {
	public:
		FatalException(ErrorCode code, const char* message = NULL);
		FatalException(const FatalException& e);
		virtual ~FatalException() throw();

		ErrorCode code();

	private:
		ErrorCode _code;
};

/**
 * @brief Causes the application to exit.
 * Throw this exception if you want to exit the application but want
 * it to shut down properly.
 */
class ExitException: public FatalException {
	public:
		ExitException(): FatalException(NO_ERROR){}
		virtual ~ExitException() throw() {}
};

#define ADD_EXCEPTION_INTERFACE(name, code) \
class name: public FatalException { \
	public: \
		name(const char* fmt, ...); \
		virtual ~name() throw();\
}

#define ADD_EXCEPTION_IMPLEMENTATION(name, code) \
name::name(const char* fmt, ...): FatalException(code){ \
	va_list va; \
	va_start(va, fmt); \
	set_message(fmt, va); \
	va_end(va);\
} \
name::~name() throw() { \
\
}

ADD_EXCEPTION_INTERFACE(XlibException, XLIB);
ADD_EXCEPTION_INTERFACE(KernelException, KERNEL);
ADD_EXCEPTION_INTERFACE(ArgumentException, ARGUMENT);
ADD_EXCEPTION_INTERFACE(BrowserException, BROWSER);
ADD_EXCEPTION_INTERFACE(IPCException, IPC_ERROR);
ADD_EXCEPTION_INTERFACE(GraphicsException, GRAPHICS_ERROR);
ADD_EXCEPTION_INTERFACE(DaemonException, DAEMON_ERROR);

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
