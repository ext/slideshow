#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

struct option {
	const char* name;
	char symbol;
	const char* desc;
	const char* arg_fmt;
	void* flag;
	int val;
};

static const int no_argument = 0;

int getopt_long(int argc, const char* const argv[], const struct option* longopts, int* longindex);

#ifdef __cplusplus
}
#endif

#endif // ARGUMENT_PARSER_H
