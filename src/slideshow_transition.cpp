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
	MODE_PREVIEW,
	MODE_LIST,
};

static const char* program_name;
static enum Mode mode = MODE_PREVIEW;
static bool fullscreen = false;
static int width = 0;
static int height = 0;
static bool running = true;
static GLuint texture[2];
static const char* name = "fade";
static transition_module_t* transition;
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

static GLuint load_texture(const char* filename){
	char* fullpath = real_path(filename);
	ILuint image;
	GLuint texture;

	ilGenImages(1, &image);
	ilBindImage(image);
	ilLoadImage(fullpath);

	ILuint devilError = ilGetError();
	if( devilError != IL_NO_ERROR ){
		fprintf(stderr, "Failed to load image '%s' (ilLoadImage: %s)\n", fullpath, iluErrorString(devilError));
		free(fullpath);
		return 0;
	}
	free(fullpath);

	const ILubyte* pixels = ilGetData();
	const ILint width  = ilGetInteger(IL_IMAGE_WIDTH);
	const ILint height = ilGetInteger(IL_IMAGE_HEIGHT);
	const ILint format = ilGetInteger(IL_IMAGE_FORMAT);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, (GLenum)format, GL_UNSIGNED_BYTE, pixels);

	return texture;
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
	}
	if ( SDL_SetVideoMode(width, height, 0, (fullscreen ? SDL_FULLSCREEN : 0) | SDL_OPENGL) == NULL ){
		fprintf(stderr, "%s: Faeiled to initialize SDL: %s", program_name, SDL_GetError());
	}
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
	gl_setup();

	/* Initialize GLEW */
	GLenum err = glewInit();
	if (GLEW_OK != err){
		fprintf(stderr, "%s: Failed to initialize GLEW\n", program_name);
		exit(1);
	}

	/* Initialize DevIL */
	ilInit();
	ILuint devilError = ilGetError();
	if (devilError != IL_NO_ERROR) {
		fprintf(stderr, "%s: Failed to initialize DevIL: %s\n", program_name, iluErrorString(devilError));
		exit(1);
	}
	iluInit();

	/* Load images */
	texture[0] = load_texture("resources/transition_a.png");
	texture[1] = load_texture("resources/transition_b.png");

	/* load transition module */
	moduleloader_init(pluginpath());
	transition = (transition_module_t*)module_open(name, TRANSITION_MODULE, 0);
	if ( !transition ){
		fprintf(stderr, "%s: Failed to load transition plugin `%s'.\n", program_name, name);
		exit(1);
	}
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
	transition_context_t context = {
		.texture = {texture[0], texture[1]},
		.state = 1.0f - s,
	};

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	transition->render(&context);
}

static void run(){
	while ( running ){
		update();
		render();
		SDL_GL_SwapBuffers();
	}
}

static int list_transitions(){
	moduleloader_init(pluginpath());
	module_enumerate(TRANSITION_MODULE, [](const module_handle mod){
		printf("%s\n", module_get_name(mod));
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

	switch ( mode ){
	case MODE_PREVIEW:
		init();
		run();
		cleanup();
		break;

	case MODE_LIST:
		list_transitions();
		break;
	}

	return 0;
}
