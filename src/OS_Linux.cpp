#include "OS.h"
#include "Log.h"	
//#include <cstdio>
#include <stdexcept>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/extensions/xf86vmode.h>
#include <GL/glx.h>
//#include <GL/glu.h>

static Display*             dpy;
static Window                   win;
static Window                   root;
static GLXFBConfig*         fbConfigs;
static GLXContext               ctx;
static XVisualInfo*         vi;
static Colormap             cmap;
static XSetWindowAttributes swa;
static XF86VidModeModeInfo **vidmodes;
static Atom                     wm_delete_window;
static Atom                     fullScreen;
static Atom                     wmState;
Cursor                          default_cursor = 0;
Cursor                          no_cursor = 0;

GLXDrawable glx_drawable;

enum
{
  _NET_WM_STATE_REMOVE =0,
  _NET_WM_STATE_ADD = 1,
  _NET_WM_STATE_TOGGLE =2

};

int doubleBufferAttributes[] = {
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_RENDER_TYPE,
    GLX_RGBA_BIT,
    GLX_DOUBLEBUFFER,  True,
    GLX_RED_SIZE,      1,
    GLX_BLUE_SIZE,     1,
    None
};

void OS::init_view(int width, int height, bool fullscreen){
    dpy = XOpenDisplay(NULL);

    if( !dpy ) {
    	throw std::runtime_error("Could not connect to an X server\n");
    }

    root = DefaultRootWindow(dpy);

    int numReturned = 0;
    fbConfigs = glXChooseFBConfig( dpy, DefaultScreen(dpy), doubleBufferAttributes, &numReturned );

    if ( !fbConfigs ) {
    	throw std::runtime_error( "No double buffered config available\n" );
    }

    vi = glXGetVisualFromFBConfig(dpy, fbConfigs[0]);

    if( !vi ) {
    	throw std::runtime_error("No appropriate visual found\n");
    }

    unsigned long mask = CWColormap | CWEventMask;
    
    if ( fullscreen ){
    	Log::message(Log::Verbose, "Graphics: Going fullscreen\n");
    	width = DisplayWidth(dpy, DefaultScreen(dpy));
    	height = DisplayHeight(dpy, DefaultScreen(dpy));
    }

    cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask ;

    win = XCreateWindow(dpy, root, 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, mask, &swa);

    XStoreName(dpy, win, "Slideshow");

    XMapWindow(dpy, win);

    ctx = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, ctx);

    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", 0);
    fullScreen = XInternAtom(dpy,"_NET_WM_STATE_FULLSCREEN", False);
    wmState = XInternAtom(dpy, "_NET_WM_STATE", False);

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

    /*XGrabPointer(dpy, win,
             True,
             0,
             GrabModeAsync, GrabModeAsync,
             win,
             cursor,
             CurrentTime);*/

    XDefineCursor(dpy, win, no_cursor);

    if ( fullscreen ){
        XEvent xev;
        xev.xclient.type=ClientMessage;
        xev.xclient.serial = 0;
        xev.xclient.send_event=True;
        xev.xclient.window=win;
        xev.xclient.message_type=wmState;
        xev.xclient.format=32;
        xev.xclient.data.l[0] = (_NET_WM_STATE_ADD);
        xev.xclient.data.l[1] = fullScreen;
        xev.xclient.data.l[2] = 0;

        XSendEvent(dpy, DefaultRootWindow(dpy), False, SubstructureRedirectMask | SubstructureNotifyMask,&xev);
    }

    glx_drawable = glXGetCurrentDrawable();
}

void OS::swap_gl_buffers(){
    glXSwapBuffers(dpy, glx_drawable);
}

void OS::cleanup(){
  XDefineCursor(dpy, win, default_cursor);
  glXDestroyContext(dpy, ctx);
	
  if( win && dpy ){
    XDestroyWindow( dpy, win );
    win = (Window) 0;
  }

  if ( dpy ){
    XCloseDisplay( dpy );
    dpy = 0;
  }
}

void OS::poll_events(bool& running){
    XEvent event;

    while ( XPending(dpy) > 0 ){
        XNextEvent(dpy, &event);
        switch (event.type){
            case KeyPress:
                if ( event.xkey.state == 24 && event.xkey.keycode == 36 ){
                    XEvent xev;
                    xev.xclient.type=ClientMessage;
                    xev.xclient.serial = 0;
                    xev.xclient.send_event=True;
                    xev.xclient.window=win;
                    xev.xclient.message_type=wmState;
                    xev.xclient.format=32;
                    xev.xclient.data.l[0] = (_NET_WM_STATE_TOGGLE);
                    xev.xclient.data.l[1] = fullScreen;
                    xev.xclient.data.l[2] = 0;

                    XSendEvent(dpy, DefaultRootWindow(dpy), False, SubstructureRedirectMask | SubstructureNotifyMask,&xev);
                    
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
