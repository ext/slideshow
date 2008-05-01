/**
 * This file is part of Slideshow.
 * Copyright (C) 2008 David Sveningsson <ext@sidvind.com>
 * 
 * Slideshow is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Slideshow is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Slideshow.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Graphics.h"
#include "OS.h"
#include "Log.h"
#include "Transition.h"
#include <GL/gl.h>
#include <cstdio>
#include <stdexcept>

#ifndef __MACH__
#   include <FreeImage.h>
#else
#   include <FreeImage/FreeImage.h>
#endif

static void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message) {
	Log::message(Log::Debug, "FreeImage: An error occured while loading an image\n");
    if(fif != FIF_UNKNOWN) {
        Log::message(Log::Debug, "FreeImage: %s Format\n", FreeImage_GetFormatFromFIF(fif));
    }
    Log::message(Log::Debug, "FreeImage: %s\n", message);
}

static FIBITMAP* GenericLoader(const char* lpszPathName, int flag) {
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

    fif = FreeImage_GetFileType(lpszPathName, 0);
    if(fif == FIF_UNKNOWN) {
        // no signature ?
        // try to guess the file format from the file extension
        fif = FreeImage_GetFIFFromFilename(lpszPathName);
    }

    // check that the plugin has reading capabilities ...
    if((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
        // ok, let's load the file
        FIBITMAP *dib = FreeImage_Load(fif, lpszPathName, flag);
        // unless a bad file format, we are done !
        return dib;
    }

    return NULL;
}
Graphics::Graphics(int width, int height, bool fullscreen):
	_transition(NULL),
	texture_0(0),
	texture_1(0){
	
	OS::init_view(width, height, fullscreen);
	Log::message(Log::Verbose, "Graphics: Using resoultion %dx%d\n", width, height);
	
	FreeImage_Initialise();
    FreeImage_SetOutputMessage(FreeImageErrorHandler);

    glShadeModel( GL_FLAT );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST );
	glClearColor(1, 0, 0, 1);
	glColor4f(1, 1, 1, 1);
	
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );
	glDisable(GL_ALPHA_TEST);
	
	glEnable( GL_TEXTURE_2D );
	glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, 1, 0, 1, -1.0, 1.0);
    glScalef(1, -1, 1);
    glTranslated(0, -1, 0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glGenTextures(1, &texture_0);
    glGenTextures(1, &texture_1);
    
    glClear( GL_COLOR_BUFFER_BIT );
}

Graphics::~Graphics(){
	glDeleteTextures(1, &texture_0);
	glDeleteTextures(1, &texture_1);
	set_transition(NULL);
	
	OS::cleanup();
}

void Graphics::render(float state){
	_transition->render(texture_0, texture_1, state);
	
	OS::swap_gl_buffers();
}

void Graphics::load_image(const char* name){
	swap_textures();
	
    FIBITMAP* dib = GenericLoader(name, 0);

    if( !dib ){
    	//@todo Is this safe?
    	char buf[512];
    	snprintf(buf, 512, "Failed to load image (%s)", name);
        throw std::runtime_error(buf);
    }

    FreeImage_FlipVertical(dib);

    glBindTexture(GL_TEXTURE_2D, texture_0);

    FIBITMAP* dib32 = FreeImage_ConvertTo24Bits(dib);
    FIBITMAP* dib_resized = FreeImage_Rescale(dib32, 1024, 1024, FILTER_BILINEAR);

    BYTE *pixels = (BYTE*)FreeImage_GetBits(dib_resized);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);

    FreeImage_Unload(dib);
    FreeImage_Unload(dib32);
    FreeImage_Unload(dib_resized);
}

void Graphics::swap_textures(){
	unsigned int tmp = texture_0;
	texture_0 = texture_1;
	texture_1 = tmp;
}

void Graphics::set_transition(Transition* transition){
	delete _transition;
	_transition = transition;
}
