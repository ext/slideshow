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

#include "core/module_loader.h"
#include "core/opengl.h"
#include "core/graphics.h"
#include "core/log.hpp"
#include "core/path.h"
#include "transitions/transition.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <cmath>
#include <unistd.h>
#include <getopt.h>
#include <ftw.h>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
typedef Bool (*glXMakeContextCurrentARBProc)(Display*, GLXDrawable, GLXDrawable, GLXContext);
static glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
static glXMakeContextCurrentARBProc glXMakeContextCurrentARB = 0;

enum Mode {
	MODE_PREVIEW = 0,
	MODE_LIST,
	MODE_GIF,
};

static const char* program_name;
static enum Mode mode = MODE_PREVIEW;
static bool fullscreen = false;
static int width = 0;
static int height = 0;
static bool running = true;
static const char* name = "fade";
static transition_module_t transition = NULL;
static bool automatic = true;
static float s = 0.0f;
static enum Severity severity = Log_Info;
static char* gif_dst = NULL;
static bool gif_update = false;

static float min(float a, float b){
	return a < b ? a : b;
}

static float max(float a, float b){
	return a > b ? a : b;
}

static float clamp(float x, float lo, float hi){
	if ( x > hi ) return hi;
	if ( x < lo ) return lo;
	return x;
}

static void init_common(){
	graphics_init(width, height);
	graphics_load_image("resources/transition_a.png", 1);
	graphics_load_image("resources/transition_b.png", 1);

	/* load transition module */
	moduleloader_init(pluginpath());
	graphics_set_transition(name, &transition);

	if ( !transition ){
		exit(1);
	}
}

static void init_window(){
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

	init_common();
}

static void cleanup_window(){
	SDL_Quit();
}

/**
 * Initialize windowless OpenGL (requires width and height to be set)
 */
static void init_windowless(){
	static int visual_attribs[] = {
		None
	};
	int context_attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 0,
		None
	};

	Display* dpy = XOpenDisplay(0);
	int fbcount = 0;
	GLXFBConfig* fbc = NULL;
	GLXContext ctx;
	GLXPbuffer pbuf;

	/* open display */
	if ( ! (dpy = XOpenDisplay(0)) ){
		fprintf(stderr, "%s: Failed to open display\n", program_name);
		exit(1);
	}

	/* get framebuffer configs, any is usable (might want to add proper attribs) */
	if ( !(fbc = glXChooseFBConfig(dpy, DefaultScreen(dpy), visual_attribs, &fbcount) ) ){
		fprintf(stderr, "%s: Failed to get FBConfig\n", program_name);
		exit(1);
	}

	/* get the required extensions */
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB");
	glXMakeContextCurrentARB = (glXMakeContextCurrentARBProc)glXGetProcAddressARB( (const GLubyte *) "glXMakeContextCurrent");
	if ( !(glXCreateContextAttribsARB && glXMakeContextCurrentARB) ){
		fprintf(stderr, "%s: missing support for GLX_ARB_create_context\n", program_name);
		XFree(fbc);
		exit(1);
	}

	/* create a context using glXCreateContextAttribsARB */
	if ( !( ctx = glXCreateContextAttribsARB(dpy, fbc[0], 0, True, context_attribs)) ){
		fprintf(stderr, "%s: Failed to create opengl context\n", program_name);
		XFree(fbc);
		exit(1);
	}

	/* create temporary pbuffer */
	int pbuffer_attribs[] = {
		GLX_PBUFFER_WIDTH, width,
		GLX_PBUFFER_HEIGHT, height,
		None
	};
	pbuf = glXCreatePbuffer(dpy, fbc[0], pbuffer_attribs);

	XFree(fbc);
	XSync(dpy, False);

	/* try to make it the current context */
	if ( !glXMakeContextCurrent(dpy, pbuf, pbuf, ctx) ){
		/* some drivers does not support context without default framebuffer, so fallback on
		 * using the default window.
		 */
		if ( !glXMakeContextCurrent(dpy, DefaultRootWindow(dpy), DefaultRootWindow(dpy), ctx) ){
			fprintf(stderr, "%s: failed to make current\n", program_name);
			exit(1);
		}
	}

	init_common();
}

static void cleanup_windowless(){

}

static void poll(){
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
				automatic = false;
				s = min(s + 0.02f, 1.0f);
			}
			break;

		case SDL_QUIT:
			running = false;
			break;
		}
	}
}

static void update(){
	Uint8* keys = SDL_GetKeyState(NULL);
	if ( keys[SDLK_LEFT]  ) s = max(s - 0.01f, 0.0f);
	if ( keys[SDLK_RIGHT] ) s = min(s + 0.01f, 1.0f);

	if ( automatic ){
		static float prev = 0.0f;
		const Uint32 t = SDL_GetTicks();
		s = fmodf(static_cast<float>(t) / 1000.0f, 2.0f);
		if ( s < prev ) graphics_swap_textures();
		prev = s;
	}
}

static void render(){
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	graphics_render(clamp(s, 0.0f, 1.0f));
}

static int preview(){
	init_window();
	while ( running ){
		poll();
		update();
		render();
		SDL_GL_SwapBuffers();
	}
	cleanup_window();
	return 0;
}

static int list_transitions(){
	moduleloader_init(pluginpath());
	module_enumerate(TRANSITION_MODULE, [](const char* name, const module_handle mod){
		printf("%s:%s\n", name, module_get_name(mod));
	});
	return 0;
}

static void write_gif_frame(char* buffer, const char* dir, int n){
	char filename[128];
	snprintf(filename, sizeof(filename), "%s/frame_%03d.ppm", dir, n);
	FILE* fp = fopen(filename, "w");
	if ( !fp ){
		fprintf(stderr, "%s: failed to write to %s: %s\n", program_name, filename, strerror(errno));
		exit(1);
	}

	fprintf(fp, "P6\n%d %d\n255\n", width, height);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	for ( int h = height-1; h >= 0; h-- ){
		fwrite(&buffer[h*width*3], width*3, 1, fp);
	}

	fclose(fp);
}

static bool gif_is_uptodate(){
	if ( !gif_update ) return false;

	const char* src = module_get_filename(&transition->base);
	const char* dst = gif_dst;
	struct stat src_st;
	struct stat dst_st;

	int ret = stat(dst, &dst_st);
	if ( ret == -1 && errno == ENOENT ){
		return false;
	} else if ( ret == -1 ){
		fprintf(stderr, "%s: failed to stat %s: %s\n", program_name, dst, strerror(errno));
		exit(1);
	}

	if ( stat(src, &src_st) == -1 ){
		fprintf(stderr, "%s: failed to stat %s: %s\n", program_name, src, strerror(errno));
		exit(1);
	}

	return dst_st.st_mtime >= src_st.st_mtime;
}

static int create_gif(){
	width = 320;
	height = 240;
	init_windowless();

	if ( gif_is_uptodate() ){
		fprintf(stderr, "%s: %s is already up-to-date.\n", program_name, gif_dst);
		cleanup_windowless();
		return 0;
	}

	/* Calculate steps and delta.
	 *
	 * For 3 steps (delta 0.5):
	 *   0        1        2
	 * 0.0      0.5       1.0
	 */
	static const int steps = 60;
	static const float delta = 1.0f / (steps-1);

	/* create temp directory */
	char dir[] = "/tmp/slideshow.XXXXXX";
	if ( !mkdtemp(dir) ){
		fprintf(stderr, "%s: failed to create temporary directory: %s\n", program_name, strerror(errno));
		exit(1);
	}

	/* generate frames */
	int frame = 1;
	char buffer[width*height*3];
	for ( int j = 0; j < 2; j++ ){
		for ( int i = 0; i < steps; i++ ){
			s = i * delta;
			render();
			write_gif_frame(buffer, dir, frame++);
		}

		/* repeat for a few frames */
		for ( int i = 0; i < steps/3; i++){
			write_gif_frame(buffer, dir, frame++);
		}

		graphics_swap_textures();
	}

	/* encode gif */
	pid_t pid;
	if ( (pid=fork()) == 0 ){
		snprintf(buffer, sizeof(buffer), "%s/frame_???.ppm", dir);
		execl("/usr/bin/convert",
		      "convert",
		      "-delay", "3",
		      "-dispose", "Background",
		      "-coalesce",
		      "-layers", "Optimize",
		      buffer,
		      "-resize", "228",
		      "-loop", "0",
		      gif_dst,
		      NULL);
		fprintf(stderr, "%s: convert error: %s\n", program_name, strerror(errno));
	} else {
		waitpid(pid, NULL, 0);
	}

	/* remove frames and temp directory */
	nftw(dir, [](const char* path, const struct stat*, int, FTW*) -> int {
		int rv = remove(path);
		if (rv){ perror(path); }
		return rv;
	}, 64, FTW_DEPTH | FTW_PHYS);
	rmdir(dir);

	cleanup_windowless();
	return 0;
}

static const char* shortopts = "lg:G:fbvh";
static struct option longopts[] = {
	{"list",        no_argument,       0, 'l'},
	{"gif",         required_argument, 0, 'g'},
	{"update-gif",  required_argument, 0, 'G'},
	{"fullscreen",  no_argument,       0, 'f'},
	{"vebose",      no_argument,       0, 'v'},
	{"help",        no_argument,       0, 'h'},
	{0, 0, 0, 0}, /* sentinel */
};

static void show_usage(void){
	printf("%s-%s\n", program_name, VERSION);
	printf("(c) 2013 David Sveningsson <ext@sidvind.com>\n\n");
	printf("Preview and manage slideshow transitions.\n");
	printf("Usage: %s [OPTIONS] [TRANSITION]\n\n"
	       "  -l, --list                 List available transitions\n"
	       "  -f, --fullscreen           Run in fullscreen mode\n"
	       "  -g, --gif=FILENAME         Create an animated gif\n"
	       "  -G, --update-gif=FILENAME  Same as --gif but only update file if source is newer.\n"
	       "  -b                         Format output as machine-parsable text\n"
	       "  -v, --verbose              Verbose output\n"
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

		case 'g': /* --gif */
			gif_dst = optarg;
			gif_update = false;
			mode = MODE_GIF;
			break;

		case 'G': /* --update-gif */
			gif_dst = optarg;
			gif_update = true;
			mode = MODE_GIF;
			break;

		case 'f': /* --fullscreen */
			fullscreen = true;
			break;

		case 'b':
			/* not implemented as of now as the output is already machine-parsable but
			   in case it changes this should be implemented */
			break;

		case 'v':
			severity = Log_Debug;
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
		create_gif,
	};

	Log::add_destination(new FileDestination(stdout), severity);
	return func[mode]();
}
