/**
 * This file is part of Slideshow.
 * Copyright (C) 2008-2011 David Sveningsson <ext@sidvind.com>
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

#include "browser.h"
#include "core/asprintf.h"
#include "core/curl_local.h"
#include "core/log.h"
#include <json/json.h>
#include <string.h>

#define FRONTEND_API_VERSION "1"

typedef struct {
	struct browser_module_t module;
	void* handle;
	struct curl_httppost* formpost;
	int id;
} frontend_context_t;

MODULE_INFO("Frontend Browser", BROWSER_MODULE, "David Sveningsson");

static int next_slide_v1(frontend_context_t* this, slide_context_t* slide, struct json_object* data){
	struct json_object* assembler = json_object_object_get(data, "assembler");
	struct json_object* slide_id  = json_object_object_get(data, "slide-id");
	struct json_object* filename  = json_object_object_get(data, "filename");
	struct json_object* context   = json_object_object_get(data, "context");

	/* is assembler isn't set, no field can be assumed to be. It means no slide could be fetched (e.g. empty queue). */
	if ( assembler ){
		slide->assembler = strdup(json_object_get_string(assembler));

		if ( strcmp(slide->assembler, "video") != 0 ){
			slide->filename = asprintf2("%s/slides/show/%d", this->module.context.host, json_object_get_int(slide_id));
		} else {
			slide->filename = strdup(json_object_get_string(filename));
		}

		this->id = json_object_get_int(context);
	}

	json_object_put(context);
	json_object_put(filename);
	json_object_put(slide_id);
	json_object_put(assembler);

	return 0;
}

static slide_context_t next_slide(frontend_context_t* this){
	slide_context_t slide;
	slide.filename = NULL;
	slide.assembler = NULL;

	struct MemoryStruct chunk;
	long response;

	chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
	chunk.size = 0;    /* no data at this point */

	char* url = asprintf2("%s/instance/next/%d", this->module.context.host, this->id);
	curl_easy_setopt(this->handle, CURLOPT_URL, url);
	free(url);

	curl_easy_setopt(this->handle, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_perform(this->handle);
	curl_easy_getinfo(this->handle, CURLINFO_RESPONSE_CODE, &response);

	if ( response != 200 ){ /* HTTP OK */
		log_message(Log_Warning, "Server replied with code %ld\n", response);
		return slide;
	}

	/* parse */
	json_object* data = json_tokener_parse(chunk.memory);
	if ( !data ){
		log_message(Log_Warning, "Failed to parse server reply: %s\n", json_object_to_json_string(data));
		return slide;
	}

	int version;
	{
		struct json_object* tmp = json_object_object_get(data, "version");
		version = json_object_get_int(tmp);
		json_object_put(tmp);
	}

	int ret;
	switch ( version ){
	case 1:
		ret = next_slide_v1(this, &slide, data);
		break;
	default:
		log_message(Log_Warning, "frontend replied with unsuppored version %d\n", version);
		return slide;
	}

	if ( ret != 0 ){
		log_message(Log_Warning, "frontend failed to parse reply (ret %d)\n", ret);
	}

	return slide;
}

static void queue_reload(frontend_context_t* this){

}

static void queue_dump(frontend_context_t* this){

}

static int queue_set(frontend_context_t* this, unsigned int id){
	return 0;
}

void* module_alloc(){
	return malloc(sizeof(frontend_context_t));
}

int EXPORT module_init(frontend_context_t* this){
	/* setup function table (casting added because it expects a different
	 * pointer type (we are using an extended struct so it is compatible).*/
	this->module.next_slide   = (next_slide_callback)next_slide;
	this->module.queue_reload = (queue_reload_callback)queue_reload;
	this->module.queue_dump   = (queue_dump_callback)queue_dump;
	this->module.queue_set    = (queue_set_callback)queue_set;

	/* initialize variables */
	this->handle = curl_easy_init();
	this->formpost = 0;
	this->id = -1;

	struct curl_httppost *lastptr = 0;
	curl_formadd(&this->formpost, &lastptr, CURLFORM_COPYNAME, "name", CURLFORM_COPYCONTENTS, this->module.context.name, CURLFORM_END);
	curl_formadd(&this->formpost, &lastptr, CURLFORM_COPYNAME, "version", CURLFORM_COPYCONTENTS, FRONTEND_API_VERSION, CURLFORM_END);
	curl_easy_setopt(this->handle, CURLOPT_HTTPPOST, this->formpost);
	curl_easy_setopt(this->handle, CURLOPT_WRITEFUNCTION, curl_local_resize);

	return 0;
}

int EXPORT module_cleanup(frontend_context_t* this){
	/* it looks weird, but free_context only releases the fields not the
	 * pointer itself, so this is safe. */
	free_context(&this->module.context);

	curl_easy_cleanup(this->handle);
	curl_formfree(this->formpost);

	return 0;
}
