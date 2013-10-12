/**
 * This file is part of Slideshow.
 * Copyright (C) 2008-2012 David Sveningsson <ext@sidvind.com>
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

#include "graphics.h"
#include "exception.h"
#include "Kernel.h"
#include "module_loader.h"
#include "log.hpp"
#include "Transition.h"
#include "path.h"
#include "gl.h"
#include "opengl.h"
#include "ptr.h"
#include "curl_local.h"

#include <cstdlib>
#include <cstring>
#include <memory>
#include <cassert>
#include <cerrno>

#include <IL/il.h>
#include <IL/ilu.h>

static CURL* curl = NULL;
static transition_module_t* transition = NULL;
static unsigned int texture[2] = {0,0};
static int width;
static int height;

int graphics_init(int w, int h){
	width = w;
	height = h;
	gl_setup();

	Log::verbose("Graphics: Using resoultion %dx%d\n", width, height);

	/* Initialize GLEW */
	GLenum err = glewInit();
	if (GLEW_OK != err){
		Log::fatal("Failed to initialize GLEW\n");
		return EINVAL;
	}
	if ( !GLEW_VERSION_2_0 ){
		Log::warning("Graphics card does not support OpenGL 2.0+\n");
	}
	if (!GLEW_ARB_texture_non_power_of_two){
		Log::warning("Graphics card does not support ARB_texture_non_power_of_two, performance will suffer\n");
	}

	/* Initialize DevIL */
	ilInit();
	ILuint devilError = ilGetError();
	if (devilError != IL_NO_ERROR) {
		Log::fatal("Devil Error (ilInit: %s)\n", iluErrorString(devilError));
		return EINVAL;
	}
	iluInit();

	/* init curl */
	curl = curl_easy_init();
	if ( !curl ){
		Log::fatal("Failed to initialize curl\n");
		return EINVAL;
	}

	/* initialize textures */
	glGenTextures(2, texture);
	for ( unsigned int i = 0; i < 2; i++ ){
		glBindTexture(GL_TEXTURE_2D, texture[i]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	return 0;
}

int graphics_cleanup(){
	glDeleteTextures(2, texture);
	curl_easy_cleanup(curl);
	module_close(&transition->base);
	return 0;
}

void graphics_render(float state){
	if ( !transition ) return;

	transition_context_t context = {
		.texture = {texture[0], texture[1]},
		.state = state,
	};
	transition->render(&context);
}

#ifdef WIN32
/* convert to LPCTSTR, release memory with free */
static TCHAR* to_tchar(const char* src){
	assert(src);
	size_t len = 0;
	TCHAR* buf = NULL;

	if ( mbstowcs_s(&len, NULL, 0, src, _TRUNCATE) != 0 ){
		return NULL;
	}

	buf = (TCHAR*)malloc(sizeof(TCHAR) * (len+1));

	if ( mbstowcs_s(&len, buf, len+1, src, _TRUNCATE) != 0 ){
		return NULL;
	}

	return buf;
}

/* convert from LPCTSTR, release memory with free */
static char* from_tchar(const TCHAR* src){
	assert(src);
	size_t len = 0;
	char* buf = NULL;

	if ( wcstombs_s(&len, NULL, 0, src, _TRUNCATE) != 0 ){
		return NULL;
	}

	buf = (char*)malloc(sizeof(char) * (len+1));

	if ( wcstombs_s(&len, buf, len+1, src, _TRUNCATE) != 0 ){
		return NULL;
	}

	return buf;
}
#endif /* WIN32 */

/**
 * Check if a filename has the .slide-extension.
 */
static bool __attribute__ ((pure))  __attribute__((nonnull)) is_slide(const char* name){
	if ( !name ) return false;
	static const char ext[] = ".slide";
	const ssize_t offset = strlen(name) - sizeof(ext) + 1; /* +1 for null terminator */

	/* name is shorter than extension */
	if ( offset < 0 ){
		return false;
	}

	return strcmp(name + offset, ext) == 0;
}

/**
 * Test if given name is a URL.
 */
static bool __attribute__((pure)) __attribute__((nonnull)) is_url(const char* name){
	if ( !name ) return false;
	static const char prefix[] = "http://";
	return strncmp(name, prefix, strlen(prefix)) == 0;
}

static int load_file(const char* filename, unsigned int dst){
	assert(filename);

	Log::debug("Loading '%s' as local file.\n", filename);

	Ptr<char> real_name(strdup(filename));
	if ( is_slide(filename) ){
		char* tmp;
		asprintf(&tmp, "%s/raster/%dx%d.png", filename, width, height);
		real_name.reset(tmp);
	}

#ifdef UNICODE
	char* tmp = real_path(real_name.get());
	Ptr<wchar_t> path(to_tchar(tmp));
	free(tmp);
#else /* UNICODE */
	Ptr<char> path(real_path(real_name.get()));
#endif /* UNICODE */

	ilBindImage(dst);
	ilLoadImage(path.get());
	ILuint devilError = ilGetError();

	if( devilError != IL_NO_ERROR ){
#ifdef UNICODE
		const wchar_t* asdf = iluErrorString (devilError);
		const char* error = from_tchar(asdf);
#else /* UNICODE */
		const char* error = iluErrorString (devilError);
#endif /* UNICODE */

		Log::warning("Failed to load image '%s' (ilLoadImage: %s)\n", path.get(), error);
		return -1;
	}

	return 0;
}

static int load_url(const char* url, unsigned int dst){
	assert(url);
	assert(dst >= 1);
	Log::debug("Loading '%s' as remote image.\n", url);

	struct MemoryStruct chunk = {
		(char*)malloc(1), 0
	};
	long response = -1;

	/* get image data */
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_local_resize);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_perform(curl);
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response);

	if ( response != 200 ){ /* HTTP OK */
		throw exception("Failed to load url, server replied with code %ld\n", response);
		return -1;
	}

	char* content_type;
	double content_length;
	curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type);
	curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &content_length);

	Log::debug("  Content-type: %s\n", content_type);
	Log::debug("  Content-length: %d bytes (chunk size %zd)\n", static_cast<int>(content_length), chunk.size);

	ilBindImage(dst);
	ilLoadL(IL_TYPE_UNKNOWN, chunk.memory, static_cast<ILuint>(chunk.size));
	ILuint devilError = ilGetError();

	if( devilError != IL_NO_ERROR ){
#ifdef UNICODE
		const wchar_t* asdf = iluErrorString (devilError);
		const char* error = from_tchar(asdf);
#else /* UNICODE */
		const char* error = iluErrorString (devilError);
#endif /* UNICODE */
		throw exception("Failed to load image '%s' (ilLoadImage: %s)", url, error);
		return -1;
	}

	return 0;
}

static int load_blank(){
	Log::debug("Loading blank image.\n");

	static const unsigned char black[] = {
		0, 0, 0
	};

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, black);
	return 0;
}

/**
 * Add a {letter,pillar}box if necessary.
 * Leaves the new image bound.
 */
static void apply_letterbox(unsigned int src, unsigned int dst){
	ilBindImage(src);

	/* skip if the size is already correct (slides from frontend is already rendered correct) */
	if ( ilGetInteger(IL_IMAGE_WIDTH) == width && ilGetInteger(IL_IMAGE_HEIGHT) == height ){
		return;
	}

	const float old_width  = static_cast<float>(ilGetInteger(IL_IMAGE_WIDTH));
	const float old_height = static_cast<float>(ilGetInteger(IL_IMAGE_HEIGHT));
	const ILuint depth = ilGetInteger(IL_IMAGE_DEPTH);
	const ILubyte bpp = static_cast<ILubyte>(ilGetInteger(IL_IMAGE_BPP));
	const ILenum format = ilGetInteger(IL_IMAGE_FORMAT);
	const ILenum type = ilGetInteger(IL_IMAGE_TYPE);
	const float old_aspect = old_width / old_height;

	float new_width  = static_cast<float>(width);
	float new_height = static_cast<float>(height);
	const float new_aspect = new_width / new_height;

	if ( old_aspect > new_aspect ){
		new_height = new_width * (old_height / old_width);
	} else {
		new_width = new_height * (old_width / old_height);
	}

	Log::debug("  Letterboxed resolution: %dx%d\n", (int)new_width, (int)new_height);

	iluImageParameter(ILU_FILTER, ILU_BILINEAR);
	iluScale(static_cast<ILuint>(new_width), static_cast<ILuint>(new_height), depth);

	const int offset_x = (width  - static_cast<int>(new_width )) / 2;
	const int offset_y = (height - static_cast<int>(new_height)) / 2;
	ilBindImage(dst);
	ilTexImage(width, height, depth, bpp, format, type, NULL);

	/* clear image */
	unsigned char black[width*4];
	memset(black, 0, width*4);
	for ( int i = 0; i < height; i++ ){
		ilSetPixels(0, i, 0, width, 1, 0, format, type, black);
	}

	/* render image centered */
	ilOverlayImage(src, offset_x, offset_y, 0);
	iluFlipImage();
}

void graphics_swap_textures(){
	const unsigned int tmp = texture[0];
	texture[0] = texture[1];
	texture[1] = tmp;
}

int graphics_load_image(const char* name, int letterbox){
	graphics_swap_textures();
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	/* null is passed when the screen should go blank (e.g. queue is empty) */
	if ( !name ){
		return load_blank();
	}

	ILuint image[2];
	ilGenImages(2, image);

	int ret;
	if ( is_url(name) ) {
		ret = load_url(name, image[0]);
	} else {
		ret = load_file(name, image[0]);
	}
	if ( ret == -1 ) return ret;

	if ( letterbox ){
		apply_letterbox(image[0], image[1]);
	}

	/* copy data to texture */
	const ILubyte* pixels = ilGetData();
	const ILuint width  = ilGetInteger(IL_IMAGE_WIDTH);
	const ILuint height = ilGetInteger(IL_IMAGE_HEIGHT);
	const ILuint format = ilGetInteger(IL_IMAGE_FORMAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);

	/* free buffer */
	ilDeleteImages(2, image);

	return 0;
}

int graphics_set_transition(const char* name, transition_module_t** mod){
	if ( mod ) *mod = NULL;

	if ( !name ){
		return EINVAL;
	}

	/* discard old transition */
	if ( transition ){
		module_close(&transition->base);
	}

	/* load new */
	transition = (transition_module_t*)module_open(name, TRANSITION_MODULE, 0);
	if ( !transition ){
		Log::fatal("Failed to load transition plugin `%s'.\n", name);
		return EINVAL;
	}

	if ( mod ) *mod = transition;
	return 0;
}
