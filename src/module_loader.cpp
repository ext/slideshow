#include "module_loader.h"
#include "Transition.h"
#include <ltdl.h>
#include <cstdio>

struct module_context_t {
	lt_dlhandle handle;
};

static int errnum = 0;

static const int MODULE_NOT_FOUND = 1;
static const int MODULE_INVALID = 2;

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

	module_context_t* context = (module_context_t*)malloc(sizeof(module_context_t));
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
			printf("Unknown module, type id is %d\n", module_type(context));
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
