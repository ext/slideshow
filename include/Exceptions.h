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
		FatalException(ErrorCode code, const char* message = NULL): BaseException(message), _code(code){}

		ErrorCode code(){ return _code; }

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
};

/**
 * @brief Xlib error.
 */
class XlibException: public FatalException {
	public:
		XlibException(const char* fmt, ...): FatalException(XLIB_ERROR){
			va_list va;
			va_start(va, fmt);
			set_message(fmt, va);
			va_end(va);
		}
};

/**
 * @brief Kernel error.
 */
class KernelException: public FatalException {
	public:
		KernelException(const char* fmt, ...): FatalException(KERNEL_ERROR){
			va_list va;
			va_start(va, fmt);
			set_message(fmt, va);
			va_end(va);
		}
};

/**
 * @brief Argument error
 */
class ArgumentException: public FatalException {
	public:
		ArgumentException(const char* fmt, ...): FatalException(ARGUMENT_ERROR){
			va_list va;
			va_start(va, fmt);
			set_message(fmt, va);
			va_end(va);
		}
};

/**
 * @brief Browser error
 */
class BrowserException: public FatalException {
	public:
		BrowserException(const char* fmt, ...): FatalException(BROWSER_ERROR){
			va_list va;
			va_start(va, fmt);
			set_message(fmt, va);
			va_end(va);
		}
};

/**
 * @brief IPC error
 */
class IPCException: public FatalException {
	public:
		IPCException(const char* fmt, ...): FatalException(IPC_ERROR){
			va_list va;
			va_start(va, fmt);
			set_message(fmt, va);
			va_end(va);
		}
};

/**
 * @brief Graphics error
 */
class GraphicsException: public FatalException {
	public:
		GraphicsException(const char* fmt, ...): FatalException(GRAPHICS_ERROR){
			va_list va;
			va_start(va, fmt);
			set_message(fmt, va);
			va_end(va);
		}
};

#endif // EXCEPTIONS_H
