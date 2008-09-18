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

#include "Browser.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

static const int state_provider = 0;
static const int state_user = 1;
static const int state_pass = 2;
static const int state_host = 3;
static const int state_name = 4;

void allocate_context(browser_context_t& context, size_t provider, size_t user, size_t pass, size_t host, size_t name);

static int extract_part(char* dst, const char* src, int offset, int n){
	if ( n == 0 ){
		return offset;
	}

	strncpy(dst, &src[offset], n);
	dst[n] = '\0';

	// We move the offset past the extracted string AND the delimiter.
	return offset + n + 1;
}

browser_context_t get_context(const char* string){
	int part_len[5] = { 0, 0, 0, 0, 0 };
	size_t string_len = strlen(string);

	unsigned int state = state_provider;
	for ( unsigned int i = 0; i < string_len; i++ ){
		char ch = string[i];

		switch ( state ){
			case state_provider:
				if ( ch == ':' ){
					i += 2; // offset to move past :// delimiter
					state = state_user;
					continue;
				}
				break;

			case state_user:
				if ( ch == ':' ){
					state = state_pass;
					continue;
				}
				if ( ch == '@' ){
					state = state_host;
					continue;
				}
				break;

			case state_pass:
				if ( ch == '@' ){
					state = state_host;
					continue;
				}
				break;

			case state_host:
				if ( ch == '/' ){
					state = state_name;
					continue;
				}
				break;
		}

		part_len[state]++;
	}

	browser_context_t context;

	allocate_context(context,
		part_len[state_provider],
		part_len[state_user],
		part_len[state_pass],
		part_len[state_host],
		part_len[state_name]
	);

	int offset = 0;

	offset = extract_part(context.provider, string, offset, part_len[state_provider]) + 2; // The first delimiter is 2 extra characters wide
	offset = extract_part(context.user, 	string, offset, part_len[state_user]);
	offset = extract_part(context.pass, 	string, offset, part_len[state_pass]);
	offset = extract_part(context.host, 	string, offset, part_len[state_host]);
	offset = extract_part(context.name, 	string, offset, part_len[state_name]);

	return context;
}

void allocate_context(browser_context_t& context, size_t provider, size_t user, size_t pass, size_t host, size_t name){
	context.provider = 	provider > 0	? (char*)malloc(provider + 1) : NULL;
	context.user = 		user > 0 		? (char*)malloc(user + 1) : NULL;
	context.pass = 		pass > 0 		? (char*)malloc(pass + 1) : NULL;
	context.host = 		host > 0 		? (char*)malloc(host + 1) : NULL;
	context.name = 		name > 0 		? (char*)malloc(name + 1) : NULL;
}

void free_context(browser_context_t& context){
	free(context.provider);
	free(context.user);
	free(context.pass);
	free(context.host);
	free(context.name);
}
