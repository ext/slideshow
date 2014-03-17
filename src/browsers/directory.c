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
#include "core/log.h"
#include "core/asprintf.h"
#include <string.h>
#include <dirent.h>

typedef struct {
	struct browser_module_t module;
	struct dirent** namelist;
	int current;
} context_t;

MODULE_INFO("Directory browser", BROWSER_MODULE, "David Sveningsson");

static int filter(const struct dirent* d){
	if (
	    strcmp(d->d_name, ".") == 0 ||
	    strcmp(d->d_name, "..") == 0 ||
	    strcmp(d->d_name, "Thumbs.db") == 0 ){
		return 0;
	}

	if ( d->d_type == DT_DIR ) return 0;
	return 1;
}

#ifdef WIN32
#	define SEPARATOR "\\"
#else
#	define SEPARATOR "/"
#endif

static void queue_reload(context_t* this){
	this->current = scandir(this->module.context.name, &this->namelist, filter, NULL);
}

static slide_context_t next_slide(context_t* this){
	slide_context_t slide;
	slide.filename = NULL;
	slide.assembler = NULL;

	while ( 1 ) {
		if ( this->current-- ){
			slide.filename = asprintf2("%s" SEPARATOR "%s", this->module.context.name, this->namelist[this->current]->d_name);
			slide.assembler = strdup("image");
			free(this->namelist[this->current]);
			return slide;
		} else {
			free(this->namelist);
			queue_reload(this);

			/* empty directory, don't repeat */
			if ( !this->current ){
				return slide;
			}

			log_message(Log_Debug, "queue wrapping\n");

			/* retry */
			continue;
		}
	}
}

static void queue_clear(context_t* this){
	while ( this->current-- ){
		free(this->namelist[this->current]);
	}
	free(this->namelist);
	this->namelist = NULL;
}

void* module_alloc(){
	return malloc(sizeof(context_t));
}

int EXPORT module_init(context_t* this){
	this->module.next_slide   = (next_slide_callback)next_slide;
	this->module.queue_reload = (queue_reload_callback)queue_reload;
	this->namelist = NULL;
	this->current = 0;

	return 0;
}

int EXPORT module_cleanup(context_t* this){
	/* it looks weird, but free_context only releases the fields not the
	 * pointer itself, so this is safe. */
	free_context(&this->module.context);

	queue_clear(this);
	return 0;
}
