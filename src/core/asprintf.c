/*
 * This file is part of libportable.
 * Copyright (C) 2008-2014 David Sveningsson <ext@sidvind.com>
 *
 * libportable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libportable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libportable.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "core/asprintf.h"

char* asprintf2(const char* fmt, ...){
	va_list ap;
	char* ret;

	va_start(ap, fmt);
	ret = vasprintf2(fmt, ap);
	va_end(ap);

	return ret;
}

char* vasprintf2(const char* fmt, va_list ap){
	char* tmp = NULL;
	if ( vasprintf(&tmp, fmt, ap) < 0 ){
		return NULL;
	}
	return tmp;
}
