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

#ifndef SLIDESHOW_MODULE_LOADER_H
#define SLIDESHOW_MODULE_LOADER_H

#include "module.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#	define SO_SUFFIX ".dll"
#else
#	define SO_SUFFIX ".la"
#endif

void moduleloader_init(const char* searchpath);
void moduleloader_cleanup();

typedef void (*module_init_callback)();
typedef void (*module_cleanup_callback)();

#define MODULE_HEAD() \
	module_init_callback init; \
	module_cleanup_callback cleanup

typedef struct {
	MODULE_HEAD();
} module_t;

int module_error();
const char* module_error_string();

struct module_context_t* module_open(const char* name);
void module_close(struct module_context_t* context);

module_t* module_get(struct module_context_t* context);
const char* module_get_name(const struct module_context_t* context);
const char* module_get_author(const struct module_context_t* context);
int module_type(const struct module_context_t* module_context_t);

#ifdef __cplusplus
}
#endif

#endif // SLIDESHOW_MODULE_LOADER_H
