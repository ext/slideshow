#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <Python.h>

static Display* dpy = NULL;

static float mode_refresh (XRRModeInfo *mode_info){
	if (mode_info->hTotal && mode_info->vTotal){
		return ((float) mode_info->dotClock / ((float) mode_info->hTotal * (float) mode_info->vTotal));
	} else {
		return 0;
	}
}

/**
 * Parse a display string to a screen number.
 *
 * N -> N
 * :X.N -> N
 *
 * @param display_string
 */
static int parse_screen(const char* display_string){
	int screen = 0;

	if ( !display_string ){
		return DefaultScreen(dpy);
	} else if ( display_string[0] == ':' ){
		sscanf(display_string, ":%*d.%d", &screen);
	} else {
		sscanf(display_string, "%d", &screen);
	}

	return screen;
}

static PyObject* xquery_screens(PyObject *self, PyObject *args) {
	PyObject* l = NULL;
	int n = 0, i = 0;
	char buf[16];

	if ( !dpy ){
		/* @todo set exception object */
		return NULL;
	}

	n = XScreenCount(dpy);
	l = PyList_New(n);

	for ( i = 0; i < n; i++ ){
		snprintf(buf, 16, ":0.%d", i);
		PyList_SET_ITEM(l, i, Py_BuildValue("s", buf));
	}

	return l;
}

static PyObject* xquery_resolution(PyObject *self, PyObject *args) {
	char* string = NULL;
	int screen, o;
	Window root;
	XRRScreenResources* res;
	PyObject* l;

	int min_width;
	int min_height;
	int max_width;
	int max_height;

	if ( !PyArg_ParseTuple(args, "|s", &string) ){
		return NULL;
	}

	if ( !dpy ){
		/* @todo set exception object */
		return NULL;
	}

	screen = parse_screen(string);
	root = XRootWindow(dpy, screen);

	XRRGetScreenSizeRange(dpy, root, &min_width, &min_height, &max_width, &max_height);
	res = XRRGetScreenResources (dpy, root);

	l = PyList_New(res->nmode);

	for ( o = 0; o < res->nmode; o++ ){
		PyList_SET_ITEM(l, o, Py_BuildValue("(iif)", res->modes[o].width, res->modes[o].height, mode_refresh(&res->modes[o])));
	}

	XRRFreeScreenResources(res);

	return l;
}

//Method table
static PyMethodDef XQueryMethods[] = {
	{"screens", xquery_screens, METH_VARARGS, "List all screens"},
	{"resolutions", xquery_resolution, METH_VARARGS, "List all resolutions for a screen. Defaults to current screen"},
	{NULL, NULL, 0, NULL}
};


PyMODINIT_FUNC initxquery(void) {
	PyObject* m;

	m = Py_InitModule("xquery", XQueryMethods);
	if ( !m ){
		return;
	}

	//dpy = XOpenDisplay(NULL);
}
