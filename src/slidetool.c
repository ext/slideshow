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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "slidelib.h"
#include "module_loader.h"
#include "path.h"

static const char* program_name = NULL;

static void print_error(const char* fmt, ...){
	fprintf(stderr, "%s: ", program_name);
	va_list arg;
	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
}

static void print_usage_suggestion(){
	print_error("Try `%s --help' for more information.\n", program_name);
}

static void print_usage(){
	printf("Usage:\n");
	printf("%s create TARGET\n", program_name);
	printf("%s resample TARGET RESOLUTION [VIRTUAL_RESOLUTION]\n", program_name);
}

int action(int argc, const char* argv[]){
	if ( strcmp("create", argv[1]) == 0 ){
		if ( argc < 3 ){
			print_error("create requires target.\n");
			print_usage_suggestion();
			return USAGE_ERROR;
		}

		const char* name = argv[2];

		return slide_create(name);

	} else if ( strcmp("resample", argv[1]) == 0 ){
		if ( argc < 4 ){
			print_error("resample requires target and resolution.\n");
			print_usage_suggestion();
			return USAGE_ERROR;
		}

		const char* filename = argv[2];
		slide_t* target = slide_from_name(filename);
		if ( !target ){
			print_error("Target is not a valid slide or you do not have permission to read it.\n");
			print_error("failed to load `%s'.\n", filename);
			return ACCESS_ERROR;
		}

		resolution_t resolution = resolution_from_string(argv[3]);

		int rc = slide_resample(target, &resolution, NULL);
		slide_free(target);

		return rc;
	} else {
		print_error("Unknown action `%s'.\n", argv[1]);
		print_usage_suggestion();
		return USAGE_ERROR;
	}
}

int main(int argc, const char* argv[]){
	program_name = argv[0];

	if ( argc < 2 ){
		print_usage();
		return 1;
	} else {
		moduleloader_init(pluginpath());
		int rc = action(argc, argv);
		moduleloader_cleanup();
		return rc;
	}
}
