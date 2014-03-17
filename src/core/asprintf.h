/**
 * This file is part of Slideshow.
 * Copyright (C) 2008-2014 David Sveningsson <ext@sidvind.com>
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

#ifndef SLIDESHOW_ASPRINTF_H
#define SLIDESHOW_ASPRINTF_H

#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Wrapper functions for {,v}asprintf which returns the string or NULL if an
 * error has occurred.
 */
char*  asprintf2(const char* fmt, ...);
char* vasprintf2(const char* fmt, va_list ap);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SLIDESHOW_ASPRINTF_H */
