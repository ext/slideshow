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
