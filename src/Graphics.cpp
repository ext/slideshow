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

#include "exception.h"
#include "Graphics.h"
#include "Kernel.h"
#include "module_loader.h"
#include "OS.h"
#include "Log.h"
#include "Transition.h"
#include "path.h"
#include "gl.h"
#include "ptr.h"
#include "curl_local.h"

#include <cstdlib>
#include <cstring>
#include <memory>
#include <cassert>

#include <portable/string.h>

/* hack to get opengl functions, does not seem to be used in header even when
 * built with opengl support. */
#define ILUT_USE_OPENGL

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

Graphics::Graphics(int width, int height, bool fullscreen):
	_transition(NULL),
	_width(width),
	_height(height){

	for ( unsigned int i = 0; i < 2; i++ ){
		_texture[i] = 0;
	}

	Log::verbose("Graphics: Using resoultion %dx%d\n", width, height);

	glew_init();
	imageloader_init();
	gl_setup();
	gl_set_matrices();
	gl_init_textures();
}

Graphics::~Graphics(){
	gl_cleanup_textures();
	imageloader_cleanup();
	glew_cleanup();
	module_close(&_transition->base);
}

void Graphics::glew_init(){
	GLenum err = glewInit();

	if (GLEW_OK != err){
		throw exception("Failed to initialize glew");
	}

	if ( !GLEW_VERSION_2_0 ){
		Log::warning("Graphics card does not support OpenGL 2.0+\n");
	}

	if (!GLEW_ARB_texture_non_power_of_two){
		Log::warning("Graphics card does not support ARB_texture_non_power_of_two, performance will suffer\n");
	}
}

void Graphics::glew_cleanup(){

}

void Graphics::imageloader_init(){
	ilInit();

	ILuint devilError = ilGetError();

	if (devilError != IL_NO_ERROR) {
		throw exception("Devil Error (ilInit: %s)", iluErrorString(devilError));
	}

	iluInit();
	ilutRenderer(ILUT_OPENGL);

	/* init curl */
	curl = curl_easy_init();
}

void Graphics::imageloader_cleanup(){
	curl_easy_cleanup(curl);
}

void Graphics::gl_setup(){
	glShadeModel( GL_FLAT );
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST );
	glClearColor(0, 0, 0, 1);
	glColor4f(1, 1, 1, 1);

	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );
	glDisable(GL_ALPHA_TEST);

	glEnable( GL_TEXTURE_2D );
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClear( GL_COLOR_BUFFER_BIT );
}

void Graphics::gl_set_matrices(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0, 1, 0, 1, -1.0, 1.0);
	glScalef(1, -1, 1);
	glTranslated(0, -1, 0);

	glEnable(GL_CULL_FACE);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void Graphics::gl_init_textures(){
	glGenTextures(2, _texture);

	for ( unsigned int i = 0; i < 2; i++ ){
		glBindTexture(GL_TEXTURE_2D, _texture[i]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}

void Graphics::gl_cleanup_textures(){
	glDeleteTextures(2, _texture);
}

void Graphics::render(float state){
	transition_context_t context;

	context.texture[0] = _texture[0];
	context.texture[1] = _texture[1];
	context.state = state;

	if ( _transition ){
		_transition->render(&context);
	}
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

void Graphics::load_file(const char* filename, unsigned int dst){
	assert(filename);

	Log::debug("Loading '%s' as local file.\n", filename);

	Ptr<char> real_name(strdup(filename));
	if ( is_slide(filename) ){
		real_name.reset(asprintf2("%s/raster/%dx%d.png", filename, _width, _height));
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
		throw exception("Failed to load image '%s' (ilLoadImage: %s)", path.get(), error);
	}
}

void Graphics::load_url(const char* url, unsigned int dst){
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
	}
}

void Graphics::load_blank(){
	Log::debug("Loading blank image.\n");

	static const unsigned char black[] = {
		0, 0, 0
	};

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, black);
}

/**
 * Add a {letter,pillar}box if necessary.
 * Leaves the new image bound.
 */
void Graphics::apply_letterbox(unsigned int src, unsigned int dst){
	ilBindImage(src);

	/* skip if the size is already correct (slides from frontend is already rendered correct) */
	if ( ilGetInteger(IL_IMAGE_WIDTH) == _width && ilGetInteger(IL_IMAGE_HEIGHT) == _height ){
		return;
	}

	const float old_width  = static_cast<float>(ilGetInteger(IL_IMAGE_WIDTH));
	const float old_height = static_cast<float>(ilGetInteger(IL_IMAGE_HEIGHT));
	const ILuint depth = ilGetInteger(IL_IMAGE_DEPTH);
	const ILubyte bpp = static_cast<ILubyte>(ilGetInteger(IL_IMAGE_BPP));
	const ILenum format = ilGetInteger(IL_IMAGE_FORMAT);
	const ILenum type = ilGetInteger(IL_IMAGE_TYPE);
	const float old_aspect = old_width / old_height;
	
	float new_width  = static_cast<float>(_width);
	float new_height = static_cast<float>(_height);
	const float new_aspect = new_width / new_height;
	
	if ( old_aspect > new_aspect ){
		new_height = new_width * (old_height / old_width);
	} else {
		new_width = new_height * (old_width / old_height);
	}
	
	Log::debug("  Letterboxed resolution: %dx%d\n", (int)new_width, (int)new_height);
	
	iluImageParameter(ILU_FILTER, ILU_BILINEAR);
	iluScale(static_cast<ILuint>(new_width), static_cast<ILuint>(new_height), depth);
	
	const int offset_x = (_width  - static_cast<int>(new_width )) / 2;
	const int offset_y = (_height - static_cast<int>(new_height)) / 2;
	ilBindImage(dst);
	ilTexImage(_width, _height, depth, bpp, format, type, NULL);
	ilOverlayImage(src, offset_x, offset_y, 0);
	iluFlipImage();
}

void Graphics::load_image(const char* name, bool letterbox){
	swap_textures();

	glBindTexture(GL_TEXTURE_2D, _texture[0]);

	/* null is passed when the screen should go blank (e.g. queue is empty) */
	if ( !name ){
		load_blank();
		return;
	}

	ILuint image[2];
	ilGenImages(2, image);

	if ( is_url(name) ) {
		load_url(name, image[0]);
	} else {
		load_file(name, image[0]);
	}

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
}

void Graphics::swap_textures(){
	unsigned int tmp = _texture[0];
	_texture[0] = _texture[1];
	_texture[1] = tmp;
}

void Graphics::set_transition(const char* name){
	assert(name);
	module_close(&_transition->base);
	_transition = (transition_module_t*)module_open(name, TRANSITION_MODULE, 0);
}
