#include "backend/SDLbackend.h"
#include "Exceptions.h"

#ifdef WIN32
#	include "win32.h"
#endif

SDLBackend::SDLBackend()
	: PlatformBackend()
	, _lock(false) {

}

SDLBackend::~SDLBackend(){

}

int SDLBackend::init(const Vector2ui &resolution){
	set_resolution(resolution.width, resolution.height);

	SDL_Init(SDL_INIT_VIDEO);
	SDL_SetVideoMode(resolution.width, resolution.height, 0, SDL_OPENGL);
	SDL_EnableUNICODE(1);

#ifdef WIN32
	SetConsoleOutputCP(65001);
#endif

	return 0;
}

void SDLBackend::cleanup(){
	SDL_Quit();
}

void SDLBackend::poll(){
	SDL_Event event;
		while(SDL_PollEvent(&event) ){
			switch(event.type){
				case SDL_VIDEORESIZE:
					printf("video resize\n");
					set_resolution(event.resize.w, event.resize.h);
					break;

				case SDL_QUIT:
					break;
			}
		}
}

void SDLBackend::swap_buffers() const {
	SDL_GL_SwapBuffers();
}

void SDLBackend::lock_mouse(bool state){
	_lock = state;

	if ( _lock ){
		SDL_WarpMouse(center().x, center().y);
	}
}
