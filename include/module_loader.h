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

#include <ltdl.h>

void moduleloader_init(const char* searchpath);
void moduleloader_cleanup();

struct module_t;

typedef void  (*module_init_callback)(struct module_t* handle);
typedef void  (*module_cleanup_callback)(struct module_t* handle);
typedef struct module_t* (*module_alloc_callback)();
typedef void  (*module_free_callback)(struct module_t* handle);

struct module_t {
	lt_dlhandle handle;

	/**
	 * Called when loading the plugin.
	 */
	module_init_callback init;

	/**
	 * Called before unloading the plugin.
	 */
	module_cleanup_callback cleanup;

	/**
	 * Allocate context structure.
	 */
	module_alloc_callback alloc;

	/**
	 * Free context structure *AND* its members.
	 */
	module_free_callback free;
};

typedef struct module_t* module_handle;

enum module_flags {
	/* Initialization flags (CALLEE_INIT is default and takes precedance if both is set) */
	MODULE_CALLEE_INIT = (1<<0), /* Automatically initializes module (default) */
	MODULE_CALLER_INIT = (2<<0), /* Caller must manually initialize module. */
};

int module_error();
const char* module_error_string();

module_handle module_open(const char* name, enum module_type_t type, int flags);
void module_close(module_handle handle);

const char* module_get_name(const module_handle handle);
const char* module_get_author(const module_handle handle);
enum module_type_t module_type(const module_handle handle);

#ifdef __cplusplus
}
#endif

#endif // SLIDESHOW_MODULE_LOADER_H
