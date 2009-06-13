#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

static float mode_refresh (XRRModeInfo *mode_info){
	if (mode_info->hTotal && mode_info->vTotal){
		return ((float) mode_info->dotClock / ((float) mode_info->hTotal * (float) mode_info->vTotal));
	} else {
		return 0;
	}
}

static void resolutions_for_screen(Display* dpy, int screen_num){
	Window root = XRootWindow(dpy, screen_num);

	int minWidth;
	int minHeight;
	int maxWidth;
	int maxHeight;

	XRRGetScreenSizeRange(dpy, root, &minWidth, &minHeight, &maxWidth, &maxHeight);
	XRRScreenResources* res = XRRGetScreenResources (dpy, root);
	assert(res);

	printf("screen :0.%d\n", screen_num);

	for ( int o = 0; o < res->nmode; o++ ){
		printf("\t%d %ux%u@%.1f\n", o, res->modes[o].width, res->modes[o].height, mode_refresh(&res->modes[o]));
	}

	XRRFreeScreenResources(res);
}

/**
 * Parse a display string to a screen number.
 *
 * N -> N
 * :X.N -> N
 *
 * @param display_string
 */
int parse_screen(const char* display_string){
	int screen = 0;

	if ( display_string[0] == ':' ){
		int dummy;
		sscanf(display_string, ":%d.%d", &dummy, &screen);
	} else {
		sscanf(display_string, "%d", &screen);
	}

	return screen;
}

int main(int argc, const char* argv[]){
	Display* dpy = XOpenDisplay(NULL);
	assert(dpy);

	if ( argc == 1 ){
		int screen_count = XScreenCount(dpy);
		printf("%d available screens\n", screen_count);

		for ( int screen_num = 0; screen_num < screen_count; screen_num++ ){
			resolutions_for_screen(dpy, screen_num);
		}

		return 0;
	}

	for ( int i = 1; i < argc; i++ ){
		if ( strcmp(argv[i], "--screen") == 0 ){
			if ( ++i<argc ){
				int screen = parse_screen(argv[i]);
				resolutions_for_screen(dpy, screen);
			} else {
				fprintf(stderr, "--screen takes one argument\n");
			}
			continue;
		}

		if ( strcmp(argv[i], "--get-screens") == 0 ){
			int screen_count = XScreenCount(dpy);
			for ( int screen_num = 0; screen_num < screen_count; screen_num++ ){
				printf(":0.%d\n", screen_num);
			}
			continue;
		}
	}

	XCloseDisplay(dpy);

	return 0;
}
