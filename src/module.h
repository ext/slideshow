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

#ifndef SLIDESHOW_MODULE_H
#define SLIDESHOW_MODULE_H

enum module_type_t {
	ANY_MODULE,
	TRANSITION_MODULE,
	ASSEMBLER_MODULE,
	BROWSER_MODULE,
};

#ifdef WIN32
#	ifdef BUILD_DLL
#		define EXPORT __declspec(dllexport)
#	else /* BUILD_DLL */
#		define EXPORT __declspec(dllimport)
#	endif /* BUILD_DLL */
#else /* WIN32 */
#	define EXPORT
#endif

#ifdef __cpluscpls
#define EXTERN extern "C"
#else
#define EXTERN
#endif

#define MODULE_INFO(name, type, author) \
	EXPORT EXTERN const char *             __module_name = name; \
	EXPORT EXTERN const enum module_type_t __module_type = type; \
	EXPORT EXTERN const char *             __module_author = author

#endif // SLIDESHOW_MODULE_H
