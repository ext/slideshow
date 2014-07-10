
#line 1 "../../src/browsers/context.rl"
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

#include "browsers/browser.h"
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


#line 77 "../../src/browsers/context.rl"



#line 50 "browsers/context.cpp"
static const char _fx_parser_actions[] = {
	0, 1, 0, 2, 1, 0, 2, 2, 
	3, 2, 2, 7, 3, 2, 4, 0, 
	3, 2, 5, 0, 3, 2, 6, 0
	
};

static const char _fx_parser_key_offsets[] = {
	0, 0, 6, 13, 14, 15, 22, 23, 
	33, 34, 41, 49, 56, 64
};

static const char _fx_parser_trans_keys[] = {
	48, 57, 65, 90, 97, 122, 58, 48, 
	57, 65, 90, 97, 122, 47, 47, 0, 
	48, 57, 65, 90, 97, 122, 0, 0, 
	47, 58, 64, 48, 57, 65, 90, 97, 
	122, 0, 0, 48, 57, 65, 90, 97, 
	122, 0, 64, 48, 57, 65, 90, 97, 
	122, 0, 48, 57, 65, 90, 97, 122, 
	0, 47, 48, 57, 65, 90, 97, 122, 
	0
};

static const char _fx_parser_single_lengths[] = {
	0, 0, 1, 1, 1, 1, 1, 4, 
	1, 1, 2, 1, 2, 0
};

static const char _fx_parser_range_lengths[] = {
	0, 3, 3, 0, 0, 3, 0, 3, 
	0, 3, 3, 3, 3, 0
};

static const char _fx_parser_index_offsets[] = {
	0, 0, 4, 9, 11, 13, 18, 20, 
	28, 30, 35, 41, 46, 52
};

static const char _fx_parser_indicies[] = {
	0, 0, 0, 1, 3, 2, 2, 2, 
	1, 4, 1, 5, 1, 1, 7, 7, 
	7, 6, 9, 8, 9, 10, 12, 13, 
	11, 11, 11, 8, 9, 6, 9, 14, 
	14, 14, 8, 9, 16, 15, 15, 15, 
	8, 9, 17, 17, 17, 8, 9, 10, 
	18, 18, 18, 8, 1, 0
};

static const char _fx_parser_trans_targs[] = {
	2, 0, 2, 3, 4, 5, 6, 7, 
	6, 13, 8, 7, 9, 11, 10, 10, 
	11, 12, 12
};

static const char _fx_parser_trans_actions[] = {
	3, 0, 1, 6, 0, 0, 3, 3, 
	1, 9, 20, 1, 12, 12, 3, 1, 
	16, 3, 1
};

static const int fx_parser_start = 1;
static const int fx_parser_first_final = 13;
static const int fx_parser_error = 0;

static const int fx_parser_en_main = 1;


#line 80 "../../src/browsers/context.rl"

int parse(struct state_t* fsm, browser_context_t* ctx, const char* line){
	assert(fsm);
	assert(ctx);

	fsm->buflen = 0;

	
#line 128 "browsers/context.cpp"
	{
	 fsm->cs = fx_parser_start;
	}

#line 88 "../../src/browsers/context.rl"

	const char* p = line;
	const char* pe = line + (strlen(p) + 1);

	
#line 139 "browsers/context.cpp"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if (  fsm->cs == 0 )
		goto _out;
_resume:
	_keys = _fx_parser_trans_keys + _fx_parser_key_offsets[ fsm->cs];
	_trans = _fx_parser_index_offsets[ fsm->cs];

	_klen = _fx_parser_single_lengths[ fsm->cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _fx_parser_range_lengths[ fsm->cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _fx_parser_indicies[_trans];
	 fsm->cs = _fx_parser_trans_targs[_trans];

	if ( _fx_parser_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _fx_parser_actions + _fx_parser_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 47 "../../src/browsers/context.rl"
	{ if ( fsm->buflen < BUFLEN ) fsm->buffer[fsm->buflen++] = (*p); }
	break;
	case 1:
#line 48 "../../src/browsers/context.rl"
	{ fsm->buflen = 0; }
	break;
	case 2:
#line 49 "../../src/browsers/context.rl"
	{ if ( fsm->buflen < BUFLEN ) fsm->buffer[fsm->buflen++] = 0; }
	break;
	case 3:
#line 51 "../../src/browsers/context.rl"
	{ ctx->provider = strdup(fsm->buffer); }
	break;
	case 4:
#line 52 "../../src/browsers/context.rl"
	{ ctx->user = strdup(fsm->buffer); }
	break;
	case 5:
#line 53 "../../src/browsers/context.rl"
	{ ctx->pass = strdup(fsm->buffer); }
	break;
	case 6:
#line 54 "../../src/browsers/context.rl"
	{ ctx->host = strdup(fsm->buffer); }
	break;
	case 7:
#line 55 "../../src/browsers/context.rl"
	{ ctx->name = strdup(fsm->buffer); }
	break;
#line 245 "browsers/context.cpp"
		}
	}

_again:
	if (  fsm->cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	_out: {}
	}

#line 93 "../../src/browsers/context.rl"

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
