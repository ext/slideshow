#include "argument_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
}

void options_set_description(const char* str){
	description = (char*)malloc(strlen(str)+1);
	strcpy(description, str);
}

void options_terminate(){
	free(description);
}

int options_parse(int argc, const char* const argv[], const struct option* longopts, int* longindex){
	while ( *longindex < argc ){
		const char* arg = argv[*longindex];
		const struct option* option = 0;

		if ( !is_argument(arg) || is_terminator(arg) ){
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

		(*longindex)++;
	}

	return -1;
}
