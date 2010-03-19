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

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "Exceptions.h"
#include "Graphics.h"
#include "Kernel.h"
#include "OS.h"
#include "Log.h"
#include "Transition.h"
#include "path.h"
#include "gl.h"

#include <cstdlib>
#include <memory>

#include <portable/string.h>
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

	Log::message(Log::Verbose, "Graphics: Using resoultion %dx%d\n", width, height);

	imageloader_init();
	gl_setup();
	gl_set_matrices();
	gl_init_textures();
}

Graphics::~Graphics(){
	gl_cleanup_textures();
	imageloader_cleanup();

	if ( _transition && _transition->cleanup ){
		_transition->cleanup();
	}

	free(_transition);
}

void Graphics::imageloader_init(){
	ilInit();

	ILuint devilError = ilGetError();

	if (devilError != IL_NO_ERROR) {
		throw GraphicsException("Devil Error (ilInit: %s", iluErrorString(devilError));
	}

	ilutRenderer(ILUT_OPENGL);
}

void Graphics::imageloader_cleanup(){

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

void Graphics::load_image(const char* name){
	swap_textures();

	glBindTexture(GL_TEXTURE_2D, _texture[0]);

	if ( name ){
#ifdef UNICODE
		char* tmp = real_path(name);
		std::auto_ptr<wchar_t> path(to_tchar(tmp));
		free(tmp);
#else /* UNICODE */
		std::auto_ptr<char> path(real_path(name));
#endif /* UNICODE */

		ILuint devilID;

		ilGenImages(1, &devilID);
		ilBindImage(devilID);
		ilLoadImage(path.get());
		ILuint devilError = ilGetError();

		if( devilError != IL_NO_ERROR ){
#ifdef UNICODE
			const wchar_t* asdf = iluErrorString (devilError);
			const char* error = from_tchar(asdf);
#else /* UNICODE */
			const char* error = iluErrorString (devilError);
#endif /* UNICODE */
			throw GraphicsException("Failed to load image '%s' (ilLoadImage: %s)", path.get(), error);
		}

		ilutGLTexImage(0);
	} else {
		static const unsigned char black[] = {
			0, 0, 0
		};

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, black);
	}
}

void Graphics::swap_textures(){
	unsigned int tmp = _texture[0];
	_texture[0] = _texture[1];
	_texture[1] = tmp;
}

void Graphics::set_transition(transition_module_t* module){
	if ( _transition && _transition->cleanup ){
		_transition->cleanup();
	}

	_transition = module;

	if ( _transition && _transition->init ){
		_transition->init();
	}
}
