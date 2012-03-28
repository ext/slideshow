/* -*- mode: c; -*- */
/**
 * This file is part of Slideshow.
 * Copyright (C) 2008-2010 David Sveningsson <ext@sidvind.com>
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

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "Browser.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>

#ifdef WIN32
#	include "win32.h"
#endif

#define BUFLEN 1024

struct state_t {
	char buffer[BUFLEN+1];
	int buflen;
	int cs;
};

%%{
	machine fx_parser;
	access fsm->;

	# Append to the buffer.
	action append { if ( fsm->buflen < BUFLEN ) fsm->buffer[fsm->buflen++] = fc; }
	action clear { fsm->buflen = 0; }
	action term { if ( fsm->buflen < BUFLEN ) fsm->buffer[fsm->buflen++] = 0; }
	
	action set_provider { ctx->provider = strdup(fsm->buffer); }
	action set_user     { ctx->user = strdup(fsm->buffer); }
	action set_pass     { ctx->pass = strdup(fsm->buffer); }
	action set_host     { ctx->host = strdup(fsm->buffer); }
	action set_src      { ctx->name = strdup(fsm->buffer); }
	
	name = alnum+ >clear $append %term;
	string = [^\0]+ >clear $append %term;
	
	provider = name %set_provider;
	user = name %set_user;
	pass = name %set_pass;
	host = name %set_host;
	src = string %set_src 0;
	
	main := provider '://' (
		user ':' pass '@' host '/' src |
		user '@' host '/' src |
		host '/' src |
		src # note that this pattern DOES NOT start with an initial slash, this
		    # is because that would require the user to pass an extra slash with
		    # filebacked browsers. Rather the slash is considered a parth of the
		    # path instead of as a separator.
		    #
		    # eg sqlite:///abs/path would be sqlite:////abs/path 
	);
}%%

%% write data;

int parse(struct state_t* fsm, browser_context_t* ctx, const char* line){
	assert(fsm);
	assert(ctx);
	
	fsm->buflen = 0;
	
	%% write init;
	
	const char* p = line;
	const char* pe = line + (strlen(p) + 1);
	
	%% write exec;
		
	if ( fsm->cs == fx_parser_error ) {
		printf("parse error\n");
		return -1;
	}
	
	return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

browser_context_t get_context(const char* string){
	struct state_t fsm;
	browser_context_t ctx;
	ctx.provider = NULL;
	ctx.user = NULL;
	ctx.pass = NULL;
	ctx.host = NULL;
	ctx.name = NULL;
	
//	fsm.stack = NULL;
//	fsm.stack_size = 0;

	if ( parse(&fsm, &ctx, string) != 0 ){
		fprintf(stderr, "Failed to parse browser-string\n");
	}
	
	return ctx;
}

void free_context(browser_context_t* context){
	free(context->provider);
	free(context->user);
	free(context->pass);
	free(context->host);
	free(context->name);
}

#ifdef __cplusplus
}
#endif
