/**
 * This file is part of Slideshow.
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

#include "Kernel.h"
#include "Graphics.h"
#include "OS.h"
#include "Log.h"
#include "transitions/fade.h"
#include "browsers/mysqlbrowser.h"
#include "IPC/dbus.h"
#include <portable/Time.h>
#include <portable/Process.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctype.h>

#include <syscall.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#ifdef LINUX
#include <sys/time.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#ifndef NULL
#define NULL 0
#endif

bool* daemon_running = NULL;
const char* application_name = "slideshow";

void quit_signal(int){
	Log::message(Log::Verbose, "Caught SIGQUIT\n");
	signal(SIGQUIT, quit_signal);
	*daemon_running = false;
}

Kernel::Kernel(int argc, char* argv[]):
	_width(800),
	_height(600),
	_frames(0),
	_fullscreen(false),
	_daemon(false),
	_transition_time(5.0f),
	_switch_time(15.0f),
	_graphics(NULL),
	_browser(NULL),
	_ipc(NULL),
	_db_username(NULL),
	_db_password(NULL),
	_db_name(NULL),
	_logfile("slideshow.log"){

	parse_argv(argc, argv);

	initTime();
	Log::initialize(_logfile, "slideshow.debug.log");
	
	if ( _daemon ){
		Log::message(Log::Verbose, "Kernel: Starting slideshow daemon\n");
		
		Portable::daemonize(application_name);
		
		if ( signal(SIGQUIT, quit_signal) == SIG_ERR ){
			Log::message(Log::Fatal, "Kernel: Could not initialize signal handler!\n");
			exit(3);
		}
		
		///@ hack
		daemon_running = &_running;
	} else {
		Log::message(Log::Verbose, "Kernel: Starting slideshow\n");
	}
	
	_graphics = new Graphics(_width, _height, _fullscreen);
	_graphics->set_transition(new FadeTransition);
	
	_ipc = new DBus(this, 50);
	
	_browser = new MySQLBrowser(_db_username, _db_password, _db_name);
	_browser->reload();
	
	_state = SWITCH;
}

Kernel::~Kernel(){
	if ( _daemon ){
		Portable::daemon_stop(application_name);
	}
	
	delete _browser;
	delete _graphics;
	delete _ipc;
	
	free( _db_username );
	free( _db_password );
	free( _db_name );
	
	_browser = NULL;
	_graphics = NULL;
	_ipc = NULL;
	
	Log::deinitialize();
}

void Kernel::run(){
	_running = true;
	
	_last_switch = getTime();
	_transition_start = 0.0f;
	
	while ( _running ){
		OS::poll_events(_running);
		
		double t = getTime();
		
		// Simple statemachine
		switch ( _state ){
			case VIEW:
				view_state(t);
				break;

			case TRANSITION:
				transition_state(t);
				break;
				
			case SWITCH:
				switch_state(t);
				break;
		}
	}
}

void Kernel::parse_argv(int argc, char* argv[]){
	for ( int i = 0; i < argc; i++ ){
		if ( strcmp(argv[i], "--fullscreen") == 0 ){
			_fullscreen = true;
			continue;
		}
		if ( strcmp(argv[i], "--daemon") == 0 ){
			_daemon = true;
			continue;
		}
		if ( strcmp(argv[i], "--db_user") == 0 ){
			i++;
			
			const char* user = argv[i];
			_db_username = (char*)malloc(strlen(user)+1);
			strcpy(_db_username, user);
			
			continue;
		}
		if ( strcmp(argv[i], "--db_pass") == 0 ){
			i++;
			
			const char* pass = argv[i];
			_db_password = (char*)malloc(strlen(pass)+1);
			strcpy(_db_password, pass);
			
			continue;
		}
		if ( strcmp(argv[i], "--db_name") == 0 ){
			i++;
			
			const char* name = argv[i];
			_db_name = (char*)malloc(strlen(name)+1);
			strcpy(_db_name, name);
			
			continue;
		}
		if ( strcmp(argv[i], "--resolution") == 0 ){
			i++;
			
			sscanf(argv[i], "%dx%d", &_width, &_height);
			continue;
		}
	}
}

void Kernel::view_state(double t){
	if ( t - _last_switch > _switch_time ){
		_state = SWITCH;
		return;
	}
	
	if ( _ipc ){
		_ipc->poll();
	}
	
	// Sleep for a while
	wait( 0.1f );
}

// The transition state calculates how long of the
// transition has come and calls the renderer
void Kernel::transition_state(double t){
	double s = (t - _transition_start) / _transition_time;

	_frames++;	
	_graphics->render( s );
	
	// If the transition is complete the state changes to VIEW
	if ( s > 1.0f ){
		if ( !_daemon ){
			printf("Frames: %d\nFPS: %f\n\n", _frames, (float)_frames/_transition_time);
		}

		_state = VIEW;
		_last_switch = t;
	}
}

void Kernel::switch_state(double t){
	const char* filename = _browser->get_next_file();
	
	if ( !filename ){
		Log::message(Log::Warning, "Kernel: Queue is empty\n", filename);
		//wait( _switch_time * 0.9f );
		return;
	}
	
	Log::message(Log::Debug, "Kernel: Switching to image \"%s\"\n", filename);
	
	try {
		_graphics->load_image( filename );
	} catch ( ... ) {
		Log::message(Log::Warning, "Kernel: Failed to load image '%s'\n", filename);
		return;
	}
	
	_state = TRANSITION;
	_frames = 0;
	_transition_start = getTime();
}

void Kernel::play_video(const char* fullpath){
	Log::message(Log::Verbose, "Kernel: Playing video \"%s\"\n", fullpath);
	
	int status;
	
	if ( fork() == 0 ){
		execlp("mplayer", "", "-fs", "-really-quiet", fullpath, NULL);
		exit(0);
	}
	
	::wait(&status);
}

void Kernel::quit(){
	_running = false;
}

void Kernel::reload_browser(){
	_browser->reload();
}

void Kernel::ipc_quit(){
	delete _ipc;
	_ipc = NULL;
}

void Kernel::debug_dumpqueue(){
	_browser->dump_queue();
}
