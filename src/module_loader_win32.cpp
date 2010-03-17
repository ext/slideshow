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

#include "module_loader.h"
#include "assembler.h"
#include "Transition.h"
#include <stdlib.h>
#include <stdio.h>
#include "win32.h"

#ifdef __cplusplus
extern "C" {
#endif

struct module_context_t {
	HMODULE handle;
};

static int errnum = 0;

#define MODULE_NOT_FOUND 1
#define MODULE_INVALID 2

/* convert to LPCTSTR, release memory with free */
static TCHAR* to_tchar(const char* src){
	size_t len = mbstowcs(NULL, src, INT_MAX);
	TCHAR* buf = (TCHAR*)malloc(sizeof(TCHAR) * (len+1));
	mbstowcs(buf, src, INT_MAX);
	return buf;
}

void moduleloader_init(const char* searchpath){
	char* path_list = strdup(searchpath);
	char* cur = NULL;
	char* path = strtok_s(path_list, ":", &cur);
	while ( path ){
		LPTSTR tpath = to_tchar(path);
		SetDllDirectory(tpath);
		
		free(tpath);
		path = strtok_s(NULL, ":", &cur);
	}
	free(path_list);
}

void moduleloader_cleanup(){
	SetDllDirectory(NULL);
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
	LPTSTR tname = to_tchar(name);
	HMODULE handle = LoadLibrary(tname);
	free(tname);

	if ( !handle ){
		errnum = MODULE_NOT_FOUND;
		return NULL;
	}

	void* sym = (void*)GetProcAddress(handle, "__module_name");

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

	FreeLibrary(context->handle);
	free(context);
}

module_t* module_get(struct module_context_t* context){
	module_t* module = NULL;

	switch ( module_type(context) ){
		case TRANSITION_MODULE:
		{
			transition_module_t* m = (transition_module_t*)malloc(sizeof(transition_module_t));
			m->render = (render_callback)GetProcAddress(context->handle, "render");
			module = (module_t*)m;
		}
		break;

		case ASSEMBLER_MODULE:
		{
			assembler_module_t* m = (assembler_module_t*)malloc(sizeof(assembler_module_t));
			m->assemble = (assemble_callback)GetProcAddress(context->handle, "assemble");
			module = (module_t*)m;
		}
		break;

		default:
			fprintf(stderr, "Unknown module, type id is %d\n", module_type(context));
			return NULL;
	}

	module->init = (module_init_callback)GetProcAddress(context->handle, "module_init");
	module->cleanup = (module_cleanup_callback)GetProcAddress(context->handle, "module_cleanup");

	return module;
}

const char* module_get_name(const struct module_context_t* context){
	void* sym = GetProcAddress(context->handle, "__module_name");
	return *((char**)sym);

}

const char* module_get_author(const struct module_context_t* context){
	void* sym = GetProcAddress(context->handle, "__module_author");
	return *((char**)sym);
}

int module_type(const struct module_context_t* context){
	void* sym = GetProcAddress(context->handle, "__module_type");
	return *((int*)sym);
}

#ifdef __cplusplus
}
#endif
