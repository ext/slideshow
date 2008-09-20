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

#include "config.h"
#include "Exceptions.h"
#include "Graphics.h"
#include "Kernel.h"
#include "OS.h"
#include "Log.h"
#include "Transition.h"
#include <GL/gl.h>
#include <cstdlib>

#include <portable/string.h>
#include <FreeImage.h>

static void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message) {
	const char* format = fif != FIF_UNKNOWN ? FreeImage_GetFormatFromFIF(fif) : "Unknown";
	Log::message(Log::Verbose, "FreeImage: An error occured while loading an image\n");
    Log::message(Log::Debug, "FreeImage: Format: %s Message: %s\n", format, message);
}

static FIBITMAP* GenericLoader(const char* lpszPathName, int flag = 0) {
    FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(lpszPathName, 0);

    if ( fif == FIF_UNKNOWN ) {
    	fif = FreeImage_GetFIFFromFilename(lpszPathName);
	}

    if ( fif == FIF_UNKNOWN ) {
    	throw GraphicsException("FreeImage: unknown format, or FreeImage does not handle it.");
    }

    if ( !FreeImage_FIFSupportsReading(fif) ){
    	throw GraphicsException("FreeImage: cannot read this format.");
    }

    return FreeImage_Load(fif, lpszPathName, flag);
}
Graphics::Graphics(int width, int height, bool fullscreen):
	_transition(NULL),
	texture_0(0),
	texture_1(0){

	OS::init_view(width, height, fullscreen);
	Log::message(Log::Verbose, "Graphics: Using resoultion %dx%d\n", width, height);

	freeimage_init();
	gl_setup();
	gl_set_matrices();
	gl_init_textures();
}

Graphics::~Graphics(){
	gl_cleanup_textures();
	freeimage_cleanup();

	if ( _transition && _transition->cleanup ){
		_transition->cleanup();
	}

	free(_transition);

	OS::cleanup();
}

void Graphics::freeimage_init(){
	FreeImage_Initialise();
	FreeImage_SetOutputMessage(FreeImageErrorHandler);
}

void Graphics::freeimage_cleanup(){
	FreeImage_DeInitialise();
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
	glGenTextures(1, &texture_0);
	glGenTextures(1, &texture_1);
}

void Graphics::gl_cleanup_textures(){
	glDeleteTextures(1, &texture_0);
	glDeleteTextures(1, &texture_1);
}

void Graphics::render(float state){
	transition_context_t context;

	context.texture[0] = texture_0;
	context.texture[1] = texture_1;
	context.state = state;

	_transition->render(&context);

	OS::swap_gl_buffers();
}

void Graphics::load_image(const char* name){
	swap_textures();

	BYTE black[] = {
		0, 0, 0
	};
	BYTE *pixels = black;
	FIBITMAP* dib_resized = NULL;

	int width = 1;
	int height = 1;

	glBindTexture(GL_TEXTURE_2D, texture_0);

	if ( name ){
		char* path = Kernel::real_path(name);

		FIBITMAP* dib = GenericLoader(path);

		if( !dib ){
			GraphicsException e = GraphicsException("Failed to load image '%s'", path);
			free(path);
			throw e;
		}

		free(path);

		FreeImage_FlipVertical(dib);

		FIBITMAP* dib32 = FreeImage_ConvertTo24Bits(dib);

		width = 1024;
		height = 1024;

		dib_resized = FreeImage_Rescale(dib32, width, height, FILTER_BILINEAR);

		pixels = (BYTE*)FreeImage_GetBits(dib_resized);

		FreeImage_Unload(dib);
		FreeImage_Unload(dib32);
	}

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);

    if ( name ){
    	FreeImage_Unload(dib_resized);
    }
}

void Graphics::swap_textures(){
	unsigned int tmp = texture_0;
	texture_0 = texture_1;
	texture_1 = tmp;
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
