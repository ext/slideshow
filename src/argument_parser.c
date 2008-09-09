#include "argument_parser.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

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

	printf("Usage: %s\n", argv[0]);

	while ( optp->name ){
		int n = 0;
		if ( optp->symbol != 0 ){
			n = printf("  --%s, -%c", optp->name, optp->symbol);
		} else {
			n = printf("  --%s", optp->name);
		}

		if ( n > 31 ){
			putchar('\n');
			n = 0;
		}

		while ( n++ < 32 ){
			putchar(' ');
		}

		printf("%s\n", optp->desc);

		optp++;
	}
}

int getopt_long(int argc, const char* const argv[], const struct option* longopts, int* longindex){
	while ( *longindex < argc ){
		const char* arg = argv[*longindex];
		const struct option* option = 0;

		if ( !is_argument(arg) || is_terminator(arg) ){
			return -1;
		}

		if ( is_help_option(arg) ){
			display_help(longopts);
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
