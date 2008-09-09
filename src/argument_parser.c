#include "argument_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

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
