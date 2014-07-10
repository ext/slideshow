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

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "argument_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef WIN32
#	include "win32.h"
#endif

#define ARGUMENT_HEAD \
	struct { \
		const char* name; \
		char flag; \
		const char* description; \
		int type; \
	}

#define arg_flag 1
#define arg_string 2
#define arg_int 3
#define arg_fmt 4

typedef struct {
	ARGUMENT_HEAD;
} argument_t;

typedef struct {
	ARGUMENT_HEAD;
	int* dst;
	int value;
} argument_flag_t;

typedef struct {
	ARGUMENT_HEAD;
	char** dst;
} argument_string_t;

typedef struct {
	ARGUMENT_HEAD;
	int* dst;
} argument_int_t;

typedef struct {
	ARGUMENT_HEAD;
	const char* format_description;
	const char* fmt;
	va_list dst;
} argument_format_t;

typedef struct argument_node_t {
	argument_t* argument;
	struct argument_node_t* next;
} argument_node_t;

void argument_free(argument_node_t* ptr){
	free(ptr->argument);
}

argument_t* argument_allocate(size_t size, int type, const char* name, char flag, const char* description){
	argument_t* arg = (argument_t*)malloc(size);
	arg->name = name;
	arg->flag = flag;
	arg->description = description;
	arg->type = type;
	return arg;
}

void option_initialize(option_set_t* option, int argc, const char* const* argv){
	option->description = 0;
	option->helpline = 0;
	option->argc = argc;
	option->argv = argv;
	option->argument = 0;
}

void option_finalize(option_set_t* option){
	argument_node_t* ptr = NULL;
	argument_node_t* next = NULL;

	free(option->description);
	free(option->helpline);

	ptr = option->argument;
	while ( ptr ){
		argument_free(ptr);
		next = ptr->next;
		free(ptr);
		ptr = next;
	}
}

static int is_argument(const char* arg){
	return arg[0] == '-';
}

static int is_terminator(const char* arg){
	return arg[1] == '-' && arg[2] == '\0';
}

static int is_long_option(const char* arg){
	return arg[1] == '-';
}

static int is_help_option(const char* arg){
	return ( arg[1] == 'h' && arg[2] == '\0' ) || ( strcmp(arg, "--help") == 0 );
}

static const char* extract_long_option(const char* arg){
	return &arg[2];
}

static char extract_short_option(const char* arg){
	return arg[1];
}

static int format_argument_count(const char* fmt){
	int n = 0;
	const char* ch = fmt;

	while ( *ch ){
		if ( ch[0] == '%' && ch[1] != '%' ){
			n++;
		}
		ch++;
	}

	return n;
}

int option_parse(option_set_t* option_set){
	int argc = option_set->argc;
	const char* const* argv = option_set->argv;

	const argument_node_t* node = NULL;
	const argument_t* extracted_option = NULL;

	int i = 1;

	while ( i < argc ){
		const char* arg = argv[i];

		// Something was not an argument, either the commandline is malformed
		// or something that was not meant to be parsed was passed, a filename
		// for instance.
		if ( !is_argument(arg) ){
			return i - 1; // Do not count as successful parse.
		}

		// The -- characters will terminate the parsing but will count as a
		// successful parse.
		if ( is_terminator(arg) ){
			return i;
		}

		// The --help option was passed, display help (and exit).
		if ( is_help_option(arg) ){
			option_display_help(option_set);
		}

		node = option_set->argument;
		extracted_option = NULL;

		if ( is_long_option(arg) ){
			const char* name = extract_long_option(arg);

			for ( ; node; node = node->next ){
				if ( strcmp(name, node->argument->name) == 0 ){
					extracted_option = node->argument;
					break;
				}
			}
		} else {
			const char flag = extract_short_option(arg);

			for ( ; node; node = node->next ){
				if ( flag == node->argument->flag ){
					extracted_option = node->argument;
					break;
				}
			}
		}

		if ( !extracted_option ){
			fprintf(stderr, "%s: unrecognized option '%s'\n", argv[0], arg);
			fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
			return -2;
		}

		if ( extracted_option->type != arg_flag ){
			i++;
			if ( i == argc ){
				fprintf(stderr, "%s: missing argument to option '%s'\n", argv[0], arg);
				fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
				return -2;
			}
		}

		switch ( extracted_option->type ){
			case arg_flag:
			{
				const argument_flag_t* real_option = (const argument_flag_t*)extracted_option;
				*(real_option->dst) = real_option->value;
				break;
			}

			case arg_string:
			{
				const argument_string_t* real_option = (const argument_string_t*)extracted_option;
				free(*(real_option->dst));
				*real_option->dst = strdup(argv[i]);
				break;
			}

			case arg_int:
			{
				const argument_int_t* real_option = (const argument_int_t*)extracted_option;
				sscanf(argv[i], "%d", real_option->dst);
				break;
			}

			case arg_fmt:
			{
				const argument_format_t* real_option = (const argument_format_t*)extracted_option;

				/* copy the va_list since the same option might be passed twice
				 * and the va_list would then be consumed. */
				va_list dst;
				va_copy(dst, real_option->dst);

				int n = format_argument_count(real_option->fmt);
				int r = vsscanf(argv[i], real_option->fmt, dst);

				va_end(dst);

				if ( n != r ){
					fprintf(stderr, "%s: invalid argument to option '%s'\n", argv[0], arg);
					fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
					return -2;
				}

				break;
			}
		}

		i++;
	}

	return i;
}

void option_set_description(option_set_t* option, const char* description){
	option->description = strdup(description);
}

void option_set_helpline(option_set_t* option, const char* helpline){
	option->helpline = strdup(helpline);
}

void option_display_help(option_set_t* option){
	argument_node_t* node = option->argument;

	printf("Usage: %s [options] %s\n", option->argv[0], option->helpline ? option->helpline : "");

	if ( option->description ){
		printf("%s\n\n", option->description);
	}

	printf("Options:\n");

	while ( node ){
		argument_t* arg = node->argument;

		int n = 0;
		if ( arg->flag != 0 ){
			n = printf("  -%c, --%s", arg->flag, arg->name);
		} else {
			n = printf("      --%s", arg->name);
		}

		switch ( arg->type ){
			case arg_string:
				n += printf(" STRING");
				break;

			case arg_int:
				n += printf(" INT");
				break;

			case arg_fmt:
				n += printf(" %s", ((argument_format_t*)arg)->format_description);
				break;
		}

		if ( n > 31 ){
			putchar('\n');
			n = 0;
		}

		while ( n++ < 32 ){
			putchar(' ');
		}

		printf("%s\n", arg->description);

		node = node->next;
	}

	exit(0);
}

void option_add_argument(option_set_t* option, argument_t* arg){
	argument_node_t* node = (argument_node_t*)malloc(sizeof(argument_node_t));
	node->argument = arg;
	node->next = option->argument;
	option->argument = node;
}

void option_add_flag(option_set_t* option, const char* name, char flag, const char* description, int* dst, int value){
	argument_flag_t* arg = (argument_flag_t*)argument_allocate(sizeof(argument_flag_t), arg_flag,name, flag, description);
	arg->dst = dst;
	arg->value = value;
	option_add_argument(option, (argument_t*)arg);
}

void option_add_string(option_set_t* option, const char* name, char flag, const char* description, char** dst){
	argument_string_t* arg = (argument_string_t*)argument_allocate(sizeof(argument_string_t), arg_string, name, flag, description);
	arg->dst = dst;
	option_add_argument(option, (argument_t*)arg);
}

void option_add_int(option_set_t* option, const char* name, char flag, const char* description, int* dst){
	argument_int_t* arg = (argument_int_t*)argument_allocate(sizeof(argument_int_t), arg_int, name, flag, description);
	arg->dst = dst;
	option_add_argument(option, (argument_t*)arg);
}

void option_add_format(option_set_t* option, const char* name, char flag, const char* description, const char* format_description, const char* fmt, ...){
#ifndef HAVE_VA_COPY
	void** fmtbuf = NULL;
	int args = 0;
#endif /* HA_VA_COPY */

	argument_format_t* arg = (argument_format_t*)argument_allocate(sizeof(argument_format_t), arg_fmt, name, flag, description);
	arg->format_description = format_description;
	arg->fmt = fmt;

#ifdef HAVE_VA_COPY
	{
		va_list ap;
		va_start(ap, fmt);
		va_copy(arg->dst, ap);
		va_end(ap);
	}
#else  /* HAVE_VA_COPY */
	/* The va_list cannot be copied directly since it might just be a pointer
	 * to the stack, in which case it will probably be overwritten by the time
	 * it is actually used. Therefore a buffer is manually allocated and filled
	 * with the passed pointers. */

	/* allocate argument buffer */
	{
		const char* ch = fmt;

		while ( *ch ){
			if ( *ch++ == '%' ) args++;
		}

		/* @todo memory leak */
		fmtbuf = (void**)malloc(args * sizeof(void*));
	}

	/* fill argument buffer */
	{
		int i;
		va_list ap;
		va_start(ap, fmt);

		for ( i = 0; i < args; i++ ){
			fmtbuf[i] = va_arg(ap, void*);
		}
	}

	arg->dst = (va_list)fmtbuf;
#endif /* HAVE_VA_COPY */

	option_add_argument(option, (argument_t*)arg);
}
