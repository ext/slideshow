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

#include "OS.h"
#include "Log.h"
//#include <cstdio>
#include "Exceptions.h"
#include <X11/Xlib.h>
#include <X11/cursorfont.h>

#if HAVE_XRANDR
#	include <X11/extensions/Xrandr.h>
#endif /* HAVE_XRANDR */

#if HAVE_XF86VIDMODE
#	include <X11/extensions/xf86vmode.h>
#endif /* HAVE_XF86VIDMODE */

#include <GL/glx.h>

typedef struct __glx_state {
	Display* dpy;
	Window win;
	Window root;
	GLXContext ctx;
	GLXDrawable glx_drawable;
} glx_state;

/* @todo Remove usage of this global state */
static glx_state g;

#if HAVE_XF86VIDMODE
static XF86VidModeModeInfo **vidmodes;
#endif /* HAVE_XF86VIDMODE */

static Atom wm_delete_window;
static Atom wm_fullscreen;
static Atom wm_state;

Cursor default_cursor = 0;
Cursor no_cursor = 0;

enum
{
  _NET_WM_STATE_REMOVE =0,
  _NET_WM_STATE_ADD = 1,
  _NET_WM_STATE_TOGGLE =2

};

int doubleBufferAttributes[] = {
	GLX_DRAWABLE_TYPE,	GLX_WINDOW_BIT,
	GLX_RENDER_TYPE,	GLX_RGBA_BIT,
	GLX_DOUBLEBUFFER,	True,
	None
};

/**
 * Return a glXVisualInfo from a FBConfig attribute list.
 * @param dpy Specifies the connection to the X server.
 * @param screen  Specifies the screen number.
 * @param attribList Specifies a list of boolean attributes and integer attribute/value pairs.  The last attribute must be None.
 */
XVisualInfo* glXVisualFromFBConfigAttributes(Display* dpy, int screen, int* attribList){
	int configs = 0;

	GLXFBConfig* fbConfigs = glXChooseFBConfig(dpy, screen, attribList, &configs);
	if ( !fbConfigs ) {
		throw XlibException( "No double buffered config available\n" );
	}

	XVisualInfo* visual = glXGetVisualFromFBConfig(dpy, fbConfigs[0]);
	if ( !visual ){
		throw XlibException("No appropriate visual found\n");
	}

	return visual;
}

enum fullscreen_state_t {
	ENABLE = (_NET_WM_STATE_ADD),
	DISABLE = (_NET_WM_STATE_REMOVE),
	TOGGLE = (_NET_WM_STATE_TOGGLE)
};

void set_fullscreen(Display* dpy, Window win, fullscreen_state_t status){
	XEvent xev;

	xev.xclient.type=ClientMessage;
	xev.xclient.serial = 0;
	xev.xclient.send_event=True;
	xev.xclient.window=win;
	xev.xclient.message_type=wm_state;
	xev.xclient.format=32;

	xev.xclient.data.l[0] = status;
	xev.xclient.data.l[1] = wm_fullscreen;
	xev.xclient.data.l[2] = 0;

	XSendEvent(dpy, DefaultRootWindow(dpy), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}

void OS::init_view(int width, int height, bool fullscreen){
	Display* dpy = XOpenDisplay(NULL);

	if( !dpy ) {
		throw XlibException("Could not connect to an X server");
	}

	Window root = DefaultRootWindow(dpy);

	XVisualInfo* vi = glXVisualFromFBConfigAttributes(dpy, DefaultScreen(dpy), doubleBufferAttributes);

	unsigned long mask = CWColormap | CWEventMask;

	if ( fullscreen ){
		Log::message(Log::Verbose, "Graphics: Going fullscreen\n");
		width = DisplayWidth(dpy, DefaultScreen(dpy));
		height = DisplayHeight(dpy, DefaultScreen(dpy));
	}

	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask ;

	Window win = XCreateWindow(dpy, root, 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, mask, &swa);

	XStoreName(dpy, win, "Slideshow");

	XMapWindow(dpy, win);

	GLXContext ctx = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, ctx);

	wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wm_fullscreen = XInternAtom(dpy,"_NET_WM_STATE_FULLSCREEN", False);
	wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);

	XSetWMProtocols(dpy, win, &wm_delete_window, 1);


	Pixmap blank;
	XColor dummy;
	char data[1] = {0};
	//Cursor cursor;

	/* make a blank cursor */
	blank = XCreateBitmapFromData (dpy, win, data, 1, 1);
	//if(blank == None) fprintf(stderr, "error: out of memory.\n");
	no_cursor = XCreatePixmapCursor(dpy, blank, blank, &dummy, &dummy, 0, 0);
	XFreePixmap (dpy, blank);

	default_cursor = XCreateFontCursor(dpy, XC_left_ptr);

    XDefineCursor(dpy, win, no_cursor);

	if ( fullscreen ){
		set_fullscreen(dpy, root, ENABLE);
	}

	GLXDrawable glx_drawable = glXGetCurrentDrawable();

	g.dpy = dpy;
	g.win = win;
	g.root = root;
	g.ctx = ctx;
	g.glx_drawable = glx_drawable;
}

void OS::swap_gl_buffers(){
	glXSwapBuffers(g.dpy, g.glx_drawable);
}

void OS::cleanup(){
  XDefineCursor(g.dpy, g.win, default_cursor);
  glXDestroyContext(g.dpy, g.ctx);

  if( g.win && g.dpy ){
	XDestroyWindow(g.dpy, g.win );
	g.win = (Window) 0;
  }

  if ( g.dpy ){
	XCloseDisplay( g.dpy );
	g.dpy = 0;
  }
}

void OS::poll_events(bool& running){
	XEvent event;

	while ( XPending(g.dpy) > 0 ){
		XNextEvent(g.dpy, &event);
		switch (event.type){
			case KeyPress:
				if ( event.xkey.state == 24 && event.xkey.keycode == 36 ){
					set_fullscreen(g.dpy, g.win, TOGGLE);
					continue;
				}

				if ( event.xkey.keycode == 9 ){
					running = false;

					continue;
				}

				break;

			case ClientMessage:
				if( (unsigned int)event.xclient.data.l[0] == wm_delete_window ){
					running = false;
					continue;
				}
				break;
		}
	}
}
