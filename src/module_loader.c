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

#include "config.h"
#include "module_loader.h"
#include "assembler.h"
#include "Browser.h"
#include "Transition.h"
#include <ltdl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct module_context_t {
	lt_dlhandle handle;
};

static int errnum = 0;

#define MODULE_NOT_FOUND 1
#define MODULE_INVALID 2

void moduleloader_init(const char* searchpath){
	lt_dlinit();

	char* path_list = strdup(searchpath);
	char* path = strtok(path_list, ":");
	while ( path ){
		lt_dladdsearchdir(path);
		path = strtok(NULL, ":");
	}
	free(path_list);
}

void moduleloader_cleanup(){
	lt_dlexit();
}

int module_error(){
	return errnum;
}

const char* module_error_string(){
	switch ( errnum ){
		case MODULE_NOT_FOUND: return "Module not found";
		case MODULE_INVALID: return "Not a valid module";

		default: return NULL;
	}

}

struct module_context_t* module_open(const char* name){
	lt_dlhandle handle = lt_dlopenext(name);

	if ( !handle ){
		errnum = MODULE_NOT_FOUND;
		return NULL;
	}

	void* sym = lt_dlsym(handle, "__module_name");

	if ( !sym ){
		errnum = MODULE_INVALID;
		return NULL;
	}

	struct module_context_t* context = (struct module_context_t*)malloc(sizeof(struct module_context_t));
	context->handle = handle;

	return context;
}

void module_close(struct module_context_t* context){
	if ( !context ){
		return;
	}

	lt_dlclose(context->handle);
	free(context);
}

static void* dlsym_abort(struct module_context_t* context, const char* name){
	void* ptr;

	if( !(ptr = lt_dlsym(context->handle, name)) ){
		fprintf(stderr, "module does not implement required function '%s'.", name);
		abort();
	}

	return ptr;
}

module_t* module_get(struct module_context_t* context){
	module_t* module = NULL;

	unsigned int* custom_size = (unsigned int*)lt_dlsym(context->handle, "__module_context_size");

	switch ( module_type(context) ){
		case TRANSITION_MODULE:
		{
			transition_module_t* m = (transition_module_t*)malloc(custom_size ? *custom_size : sizeof(transition_module_t));
			m->render = (render_callback)dlsym_abort(context, "render");
			module = (module_t*)m;
		}
		break;

		case ASSEMBLER_MODULE:
		{
			assembler_module_t* m = (assembler_module_t*)malloc(custom_size ? *custom_size : sizeof(assembler_module_t));
			m->assemble = (assemble_callback)dlsym_abort(context, "assemble");
			module = (module_t*)m;
		}
		break;

		case BROWSER_MODULE:
		{
			size_t base_size = sizeof(browser_module_t) - sizeof(browser_context_t);
			size_t context_size = custom_size ? *custom_size : sizeof(browser_context_t);

			browser_module_t* m = (browser_module_t*)malloc(base_size + context_size);

			m->next_slide   = (next_slide_callback)  dlsym_abort(context, "next_slide");
			m->queue_reload = (queue_reload_callback)dlsym_abort(context, "queue_reload");
			m->queue_dump   = (queue_dump_callback)  dlsym_abort(context, "queue_dump");
			m->queue_set    = (queue_set_callback)   dlsym_abort(context, "queue_set");
			m->init2        = (browser_init_callback)dlsym_abort(context, "module_init");

			module = (module_t*)m;
			printf("module: %p\n", module);
		}
		break;

		default:
			fprintf(stderr, "module_loader: unknown module, type id is %d\n", module_type(context));
			return NULL;
	}

	module->init = (module_init_callback)lt_dlsym(context->handle, "module_init");
	module->cleanup = (module_cleanup_callback)lt_dlsym(context->handle, "module_cleanup");

	return module;
}

const char* module_get_name(const struct module_context_t* context){
	void* sym = lt_dlsym(context->handle, "__module_name");
	return *((char**)sym);

}

const char* module_get_author(const struct module_context_t* context){
	void* sym = lt_dlsym(context->handle, "__module_author");
	return *((char**)sym);
}

int module_type(const struct module_context_t* context){
	void* sym = lt_dlsym(context->handle, "__module_type");
	return *((int*)sym);
}
