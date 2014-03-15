/**
 * This file is part of Slideshow.
 * Copyright (C) 2008-2013 David Sveningsson <ext@sidvind.com>
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
#include "config.h"
#endif

#include "log.h"

void log_message(enum Severity severity, const char* fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	log_vmessage(severity, fmt, ap);
	va_end(ap);
}

void log_vmessage(enum Severity severity, const char* fmt, va_list ap){
	Log::vmessage(severity, fmt, ap);
}
