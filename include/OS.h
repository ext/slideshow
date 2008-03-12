#ifndef OS_H
#define OS_H

namespace OS {
	void init_view(int width, int height, bool fullscreen);
	void cleanup();
	void swap_gl_buffers();
	void poll_events(bool& running);
}

#endif // OS_H
