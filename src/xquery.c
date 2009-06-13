#include <stdio.h>
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
int main(int argc, const char* argv[]){
	Display* dpy = XOpenDisplay(NULL);
	assert(dpy);

	int screen_count = XScreenCount(dpy);
	printf("%d available screens\n", screen_count);

	for ( int screen_num = 0; screen_num < screen_count; screen_num++ ){
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

	XCloseDisplay(dpy);

	return 0;
}
