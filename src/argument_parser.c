#include "argument_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define ARGUMENT_HEAD \
	const char* name; \
	char flag; \
	const char* description; \
	int type;

#define arg_flag 1
#define arg_string 2
#define arg_int 3
#define arg_fmt 4

typedef struct {
	ARGUMENT_HEAD
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
	option->argc = argc;
	option->argv = argv;
	option->argument = 0;
}

void option_finalize(option_set_t* option){
	free(option->description);

	argument_node_t* ptr = option->argument;
	while ( ptr ){
		argument_free(ptr);
		struct argument_node_t* next = ptr->next;
		free(ptr);
		ptr = next;
	}
}

int option_parse(option_set_t* option){
	option_display_help(option);
	return 0;
}

void option_set_description(option_set_t* option, const char* description){
	option->description = (char*)malloc(strlen(description) + 1);
	strcpy(option->description, description);
}

void option_display_help(option_set_t* option){
	printf("Usage: %s [options]\n", option->argv[0]);

	if ( option->description ){
		printf("%s\n\n", option->description);
	}

	printf("Options:\n");

	argument_node_t* node = option->argument;
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
	argument_format_t* arg = (argument_format_t*)argument_allocate(sizeof(argument_format_t), arg_fmt, name, flag, description);
	arg->format_description = format_description;
	arg->fmt = fmt;
	va_start(arg->dst, fmt);
	option_add_argument(option, (argument_t*)arg);
}

/*
static char* description = 0;

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

static char* format_argument_modify(const char* fmt){
	// Size of format string + number of asterisks to insert + %n and null terminator
	char* buf = (char*)malloc(strlen(fmt) + format_argument_count(fmt) + 3);

	const char* ch = fmt;
	int i = 0;
	while ( *ch ){
		if ( ch[0] == '%' ){
			buf[i++] = '%';

			if ( ch[1] == '%' ){
				buf[i++] = '%';
				ch++;
			} else {
				buf[i++] = '*';
			}
		} else {
			buf[i++] = *ch;
		}

		ch++;
	}

	buf[i++] = '%';
	buf[i++] = 'n';
	buf[i++] = '\0';

	return buf;
}

static int validate_data_format(const char* data, const char* fmt){
	char* new_format = format_argument_modify(fmt);
	int b = 0;
	sscanf(data, new_format, &b);
	free(new_format);

	return b == strlen(data);
}

static void display_help(const char* const argv[], const struct option* longopts){
	const struct option* optp = longopts;

	printf("Usage: %s [options]\n", argv[0]);

	if ( description ){
		printf("%s\n\n", description);
	}

	printf("Options:\n");

	while ( optp->name ){
		int n = 0;
		if ( optp->symbol != 0 ){
			n = printf("  -%c, --%s", optp->symbol, optp->name);
		} else {
			n = printf("      --%s", optp->name);
		}

		if ( optp->format ){
			n += printf(" %s", optp->format);
		}

		if ( n > 31 ){
			putchar('\n');
			n = 0;
		}

		while ( n++ < 32 ){
			putchar(' ');
		}

		printf("%s\n", optp->description);

		optp++;
	}

	exit(0);
}

void options_set_description(const char* str){
	description = (char*)malloc(strlen(str)+1);
	strcpy(description, str);
}

void options_terminate(){
	free(description);
}

int options_parse(int argc, const char* const argv[], const struct option* longopts, int* longindex){
	(*longindex)++;

	while ( *longindex < argc ){
		const char* arg = argv[*longindex];
		const struct option* option = 0;

		if ( !is_argument(arg) || is_terminator(arg) ){
			printf("not an argument or was a terminator\n");
			return -1;
		}

		if ( is_help_option(arg) ){
			display_help(argv, longopts);
			return -2;
		}

		const struct option* optp = longopts;
		if ( is_long_option(arg) ){
			const char* name = extract_long_option(arg);

			for ( ; optp->name != 0; ++optp ){
				if ( strcmp(name, optp->name) == 0 ){
					option = optp;
					break;
				}
			}
		} else {
			const char symbol = extract_short_option(arg);

			for ( ; optp->name != 0; ++optp ){
				if ( symbol == optp->symbol ){
					option = optp;
					break;
				}
			}
		}

		if ( !option ){
			printf("%s: unrecognized option '--%s'\n", argv[0], arg);
			printf("Try `%s --help' for more information.\n", argv[0]);
			return -2;
		}

		(*longindex)++;
		const char* data = argv[*longindex];

		if ( option->format ){
			if ( !validate_data_format(data, option->format) ){
				printf("%s: the '--%s' option requires data to use the '%s' format\n", argv[0], option->name, option->format);
				printf("Try `%s --help' for more information.\n", argv[0]);
				return -2;
			}
		}

		if ( option->flag ){
			if ( option->format ){
				sscanf(data, option->format, option->flag);
				printf("%s set to %s\n", option->name, data);
			} else {
				*((int*)option->flag) = option->val;
			}
		} else {
			return option->val;
		}
	}

	return -1;
}
*/
