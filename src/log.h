/**
 * This file is part of Slideshow.
 * Copyright (C) 2008-2012 David Sveningsson <ext@sidvind.com>
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

#ifndef LOG_BASE_H
#define LOG_BASE_H

/**
 * c api for logging.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

enum Severity {
	Log_Debug = 0,
	Log_Verbose,
	Log_Info,
	Log_Warning,
	Log_Fatal
};

void  log_message(enum Severity severity, const char* fmt, ...) __attribute__((format(printf,2,3)));
void log_vmessage(enum Severity severity, const char* fmt, va_list ap);

#ifdef __cplusplus
}
#endif

#endif /* LOG_BASE_H */
