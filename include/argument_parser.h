#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct option_set_t {
	char* description;
	int argc;
	const char* const* argv;
	struct argument_node_t* argument;
} option_set_t;

void option_initialize(option_set_t* option, int argc, const char* const* argv);
void option_finalize(option_set_t* option);
int option_parse(option_set_t* option);

void option_set_description(option_set_t* option, const char* description);
void option_display_help(option_set_t* option);

void option_add_flag(option_set_t* option, const char* name, char flag, const char* description, int* dst, int value);
void option_add_string(option_set_t* option, const char* name, char flag, const char* description, char** dst);
void option_add_int(option_set_t* option, const char* name, char flag, const char* description, int* dst);
void option_add_format(option_set_t* option, const char* name, char flag, const char* description, const char* format_description, const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif // ARGUMENT_PARSER_H
