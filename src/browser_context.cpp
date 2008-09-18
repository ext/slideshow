#include "Browser.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

static const int state_provider = 0;
static const int state_user = 1;
static const int state_pass = 2;
static const int state_host = 3;
static const int state_name = 4;

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
	context.provider = (char*)malloc(part_len[state_provider] + 1);
	context.user = (char*)malloc(part_len[state_user] + 1);
	context.pass = part_len[state_pass] > 0 ? (char*)malloc(part_len[state_pass] + 1) : NULL;
	context.host = (char*)malloc(part_len[state_host] + 1);
	context.name = (char*)malloc(part_len[state_name] + 1);

	int offset = 0;

	offset = extract_part(context.provider, string, offset, part_len[state_provider]); offset += 2;
	offset = extract_part(context.user, string, offset, part_len[state_user]);
	offset = extract_part(context.pass, string, offset, part_len[state_pass]);
	offset = extract_part(context.host, string, offset, part_len[state_host]);
	offset = extract_part(context.name, string, offset, part_len[state_name]);

	return context;

}

void free_context(browser_context_t& context){
	free(context.provider);
	free(context.user);
	free(context.pass);
	free(context.host);
	free(context.name);
}
