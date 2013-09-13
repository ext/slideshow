/**
 * This file is part of Slideshow.
 * Copyright (C) 2013 David Sveningsson <ext@sidvind.com>
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
#include "config.h"
#endif

#include "module_loader.h"
#include "opengl.h"
#include "graphics.h"
#include "log.hpp"
#include "path.h"
#include "Transition.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <cmath>
#include <getopt.h>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>

enum Mode {
	MODE_PREVIEW = 0,
	MODE_LIST,
};

static const char* program_name;
static enum Mode mode = MODE_PREVIEW;
static bool fullscreen = false;
static int width = 0;
static int height = 0;
static bool running = true;
static const char* name = "fade";
static bool automatic = true;
static float s = 0.0f;

static float min(float a, float b){
	return a < b ? a : b;
}

static float max(float a, float b){
	return a > b ? a : b;
}

static float clamp(float x, float hi, float lo){
	if ( x > hi ) return hi;
	if ( x < lo ) return lo;
	return x;
}

static void init(){
	/* use default resolution 0x0 (native) for fullscreen and 800x600 for
	   windowed */
	if ( !fullscreen && width == 0 ){
		width = 800;
		height = 600;
	}

	/* Initialize SDL */
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ){
		fprintf(stderr, "%s: Failed to initialize SDL: %s", program_name, SDL_GetError());
		exit(1);
	}
	if ( SDL_SetVideoMode(width, height, 0, (fullscreen ? SDL_FULLSCREEN : 0) | SDL_OPENGL) == NULL ){
		fprintf(stderr, "%s: Failed to initialize SDL: %s", program_name, SDL_GetError());
		exit(1);
	}
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);

	/* read actual resolution */
	const SDL_VideoInfo* vi = SDL_GetVideoInfo();
	if ( !vi ){
		fprintf(stderr, "%s: SDL_GetVideoInfo() failed\n", program_name);
		exit(1);
	}
	width = vi->current_w;
	height = vi->current_h;

	graphics_init(width, height);
	graphics_load_image("resources/transition_a.png", 1);
	graphics_load_image("resources/transition_b.png", 1);

	/* load transition module */
	moduleloader_init(pluginpath());
	graphics_set_transition(name);
}

static void cleanup(){
	SDL_Quit();
}

static void update(){
	SDL_Event event;
	while(SDL_PollEvent(&event) ){
		switch(event.type){
		case SDL_KEYDOWN:
			switch ( event.key.keysym.sym ){
			case SDLK_ESCAPE:
				running = false;
				break;

			case SDLK_LEFT:
			case SDLK_RIGHT:
				automatic = false;
				break;

			case SDLK_SPACE:
				automatic = true;
				break;

			default:
				break;
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			if ( event.button.button == 4){
				automatic = false;
				s = max(s - 0.02f, 0.0f);
			} else if ( event.button.button == 5){
				automatic = 0;
				s = min(s + 0.02f, 1.0f);
			}
			break;

		case SDL_QUIT:
			running = false;
			break;
		}
	}

	Uint8* keys = SDL_GetKeyState(NULL);
	if ( keys[SDLK_LEFT]  ) s = max(s - 0.01f, 0.0f);
	if ( keys[SDLK_RIGHT] ) s = min(s + 0.01f, 1.0f);

	if ( automatic ){
		const Uint32 t = SDL_GetTicks();
		const float x = (float)t / 1000.0f;
		s = clamp(asinf(sinf(x)) / (float)M_PI_2, 0.5f, -0.5f) + 0.5f;
	}
}

static void render(){
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	graphics_render(s);
	SDL_GL_SwapBuffers();
}

static int preview(){
	init();
	while ( running ){
		update();
		render();
	}
	cleanup();
	return 0;
}

static int list_transitions(){
	moduleloader_init(pluginpath());
	module_enumerate(TRANSITION_MODULE, [](const char* name, const module_handle mod){
		printf("%s:%s\n", name, module_get_name(mod));
	});
	return 0;
}

static const char* shortopts = "lfbh";
static struct option longopts[] = {
	{"list",        no_argument, 0, 'l'},
	{"fullscreen",  no_argument, 0, 'f'},
	{"help",        no_argument, 0, 'h'},
	{0, 0, 0, 0}, /* sentinel */
};

static void show_usage(void){
	printf("%s-%s\n", program_name, VERSION);
	printf("(c) 2013 David Sveningsson <ext@sidvind.com>\n\n");
	printf("Preview and manage slideshow transitions.\n");
	printf("Usage: %s [OPTIONS] [TRANSITION]\n\n"
	       "  -l, --list                 List available transitions\n"
	       "  -f, --fullscreen           Run in fullscreen mode\n"
	       "  -b                         Format output as machine-parsable text\n"
	       "  -h, --help                 Show this text.\n",
	       program_name);
}

int main(int argc, char* argv[]){
	/* extract program name from path. e.g. /path/to/foo -> foo */
	const char* separator = strrchr(argv[0], '/');
	if ( separator ){
		program_name = separator + 1;
	} else {
		program_name = argv[0];
	}

	int option_index = 0;
	int op;

	/* parse arguments */
	while ( (op=getopt_long(argc, argv, shortopts, longopts, &option_index)) != -1 ){
		switch ( op ){
		case 'l': /* --list */
			mode = MODE_LIST;
			break;

		case 'f': /* --fullscreen */
			fullscreen = true;
			break;

		case 'b':
			/* not implemented as of now as the output is already machine-parsable but
			   in case it changes this should be implemented */
			break;

		case 'h': /* --help */
			show_usage();
			return 0;
		}
	}

	if ( optind < argc ){
		name = argv[optind];
	}

	int (*func[])() = {
		preview,
		list_transitions,
	};

	Log::add_destination(new FileDestination(stdout));
	return func[mode]();
}
