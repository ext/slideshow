#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

struct option {
	const char* name;
	char symbol;
	const char* description;
	const char* format;
	void* flag;
	int val;
};

static const int no_argument = 0;

void options_set_description(const char* str);
void options_terminate();
int options_parse(int argc, const char* const argv[], const struct option* longopts, int* longindex);

#ifdef __cplusplus
}
#endif

#endif // ARGUMENT_PARSER_H
