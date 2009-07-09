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
#include "Transition.h"
#include <ltdl.h>
#include <stdio.h>
#include <stdlib.h>

struct module_context_t {
	lt_dlhandle handle;
};

static int errnum = 0;

#define MODULE_NOT_FOUND 1
#define MODULE_INVALID 2

void moduleloader_init(const char* searchpath){
	lt_dlinit();
	lt_dladdsearchdir(searchpath);
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

module_t* module_get(struct module_context_t* context){
	module_t* module = NULL;

	switch ( module_type(context) ){
		case TRANSITION_MODULE:
		{
			transition_module_t* m = (transition_module_t*)malloc(sizeof(transition_module_t));
			m->render = (render_callback)lt_dlsym(context->handle, "render");
			module = (module_t*)m;
		}
		break;

		default:
			fprintf(stderr, "Unknown module, type id is %d\n", module_type(context));
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
