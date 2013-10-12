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
#include "log.h"
#include "path.h"
#include "Transition.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fnmatch.h>

#ifdef WIN32
#	define SO_SUFFIX ".dll"
#else
#	define SO_SUFFIX ".la"
#endif

struct module_context_t {
	lt_dlhandle handle;
};

static enum {
	MODULE_NO_ERROR,
	MODULE_NOT_FOUND,
	MODULE_INVALID,
	MODULE_WRONG_TYPE,
} errnum = MODULE_NO_ERROR;

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
		case MODULE_NO_ERROR: return NULL;
		case MODULE_NOT_FOUND: return "Module not found";
		case MODULE_INVALID: return "Not a valid module";
		case MODULE_WRONG_TYPE: return "Requested module is not of the requested type";
	}

	/* make gcc happy (warning: control reaches end of non-void function) */
	return NULL;
}

static struct module_t* default_alloc(){
	return (struct module_t*)malloc(sizeof(struct module_t));
}

static void default_free(struct module_t* m){
	free(m);
}

/**
 * returns non-zero if MODULE_CALLEE_INIT is set
 * or if neither MODULE_CALLEE_INIT or
 * MODULE_CALLER_INIT is.
 */
static int callee_init(int flags){
	int A = flags & MODULE_CALLEE_INIT;
	int B = flags & MODULE_CALLER_INIT;

	/* converse implication */
	return A | !B;
}

module_handle module_open(const char* name, enum module_type_t type, int flags){
	log_message(Log_Debug, "Loading plugin '%s'\n", name);

	/* dlopenext tries all searchpaths and adds appropriate suffix */
	lt_dlhandle handle = lt_dlopenext(name);

	if ( !handle ){
		log_message(Log_Debug, "Plugin '%s' not found in search path.\n", name);
		errnum = MODULE_NOT_FOUND;
		return NULL;
	}

	/* test if the module is valid by quering a symbol which must exists in
	 * every module. */
	void* sym = lt_dlsym(handle, "__module_type");
	if ( !sym ){
		log_message(Log_Debug, "Plugin '%s' found but is invalid\n", name);
		errnum = MODULE_INVALID;
		return NULL;
	}

	if ( type != ANY_MODULE && *((enum module_type_t*)sym) != type ){
		log_message(Log_Debug, "Plugin '%s' found but is invalid\n", name);
		errnum = MODULE_INVALID;
		return NULL;
	}

	/* base functions */
	void* module_init    = lt_dlsym(handle, "module_init");
	void* module_cleanup = lt_dlsym(handle, "module_cleanup");
	void* module_alloc   = lt_dlsym(handle, "module_alloc");
	void* module_free    = lt_dlsym(handle, "module_free");

	/* create base structure (later copied into the real struct */
	struct module_t base;
	base.handle  = handle;
	base.info    = lt_dlgetinfo(handle);
	base.init    = (module_init_callback)module_init;
	base.cleanup = (module_cleanup_callback)module_cleanup;
	base.alloc   = module_alloc ? (module_alloc_callback)module_alloc : default_alloc;
	base.free    = module_free  ?  (module_free_callback)module_free  : default_free;

	/* allocate real structure and copy base fields */
	module_handle module = base.alloc();
	*module = base;

	/* run module initialization if available */
	if ( callee_init(flags) && module->init && module->init(module) != 0 ){
		log_message(Log_Fatal, "Plugin `%s' initialization failed.\n", name);
		return NULL;
	}

	return module;
}

void module_close(module_handle module){
	/* it should be safe to call module_close on a NULL pointer */
	if ( !module ){
		return;
	}

	/* run module cleanup if available */
	if ( module->cleanup ){
		module->cleanup(module);
	}

	/* close ltdl context */
	lt_dlclose(module->handle);

	/* release handle */
	module->free(module);
}

static int filter(const struct dirent* el){
	return fnmatch("*" SO_SUFFIX , el->d_name, 0) == 0;
}

void module_enumerate(enum module_type_t type, void (*callback)(const char* name, const module_handle mod)){
	char* ctx;
	char* path_list = strdup(pluginpath());
	char* path = strtok_r(path_list, ":", &ctx);

	while ( path ){
		int n;
		struct dirent **namelist;
		if ( (n=scandir(path, &namelist, filter, NULL)) >= 0 ){
			for ( int i = 0; i < n; i++ ){
				char* name = namelist[i]->d_name;
				char* ext = strrchr(name, '.'); /* remove extension from name */
				if ( ext ) *ext = 0;

				module_handle context = module_open(name, ANY_MODULE, MODULE_CALLER_INIT);

				if ( !context ) continue;
				if ( !(type == ANY_MODULE || module_type(context) == type) ){
					continue;
				}

				callback(name, context);
				module_close(context);
			}

			for ( int i = 0; i < n; i++ ){
				free(namelist[i]);
			}
			free(namelist);
		} else {
			log_message(Log_Warning, "Failed to read %s: %s\n", path, strerror(errno));
		}

		path = strtok_r(NULL, ":", &ctx);
	}

	free(path_list);
}

const char* module_get_name(const module_handle module){
	void* sym = lt_dlsym(module->handle, "__module_name");
	return *((char**)sym);

}

const char* module_get_author(const module_handle module){
	void* sym = lt_dlsym(module->handle, "__module_author");
	return *((char**)sym);
}

enum module_type_t module_type(const module_handle module){
	void* sym = lt_dlsym(module->handle, "__module_type");
	return *((enum module_type_t*)sym);
}

const char* module_get_filename(const module_handle handle){
	return handle->info->filename;
}
