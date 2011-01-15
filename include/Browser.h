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

#ifndef BROWSER_H
#define BROWSER_H

#include "module_loader.h"

typedef struct {
	char* provider;
	char* user;
	char* pass;
	char* host;
	char* name;
} browser_context_t;

#ifdef __cplusplus
extern "C" {
#endif

browser_context_t get_context(const char* string);
void free_context(browser_context_t* context);

#ifdef __cplusplus
}
#endif

typedef struct {
	char* filename;
	char* assembler;
} slide_context_t;

struct browser_module_t;

/**
 * Get next slide from queue.
 * @return A slide context with copies of the strings which the caller must
 *         deallocate using free.
 */
typedef slide_context_t (*next_slide_callback)(struct browser_module_t* data);

/**
 * Force reload of queue.
 */
typedef int (*queue_reload_callback)(struct browser_module_t* data);

/**
 * Dump queue to log.
 */
typedef int (*queue_dump_callback)(struct browser_module_t* data);

/**
 * Change the active queue.
 */
typedef int (*queue_set_callback)(struct browser_module_t* data, unsigned int id);

struct browser_module_t {
	struct module_t module;
	browser_context_t context;

	next_slide_callback next_slide;
	queue_reload_callback queue_reload;
	queue_dump_callback queue_dump;
	queue_set_callback queue_set;
};

#endif // BROWSER_H
