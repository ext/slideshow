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

// Internal
#include "config.h"
#include "Kernel.h"
#include "Graphics.h"
#include "OS.h"
#include "Log.h"
#include "ErrorCodes.h"
#include "Exceptions.h"

// Transitions
#include "transitions/dummy.h"
#include "transitions/fade.h"
#include "transitions/spin.h"

// Browsers
#include "browsers/mysqlbrowser.h"

// IPC
#include "IPC/dbus.h"

// Argument parser
#include "argument_parser.h"

// FSM
#include "state/State.h"
#include "state/InitialState.h"
#include "state/SwitchState.h"
#include "state/TransitionState.h"
#include "state/ViewState.h"

// libportable
#include <portable/Time.h>
#include <portable/Process.h>
#include <portable/string.h>

// libc
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctype.h>

// Platform
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <X11/Xlib.h>

#ifdef LINUX
#include <sys/time.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

bool* daemon_running = NULL;
static const char* const application_name = "slideshow";

void quit_signal(int){
	Log::message(Log::Verbose, "Caught SIGQUIT\n");
	signal(SIGQUIT, quit_signal);
	*daemon_running = false;
}

Kernel::Kernel(int argc, const char* argv[]):
	_width(800),
	_height(600),
	_frames(0),
	_bin_id(1),
	_fullscreen(0),
	_daemon(0),
	_verbose(1),
	_stdin(0),
	_transition_time(3.0f),
	_switch_time(5.0f),
	_graphics(NULL),
	_browser(NULL),
	_ipc(NULL),
	_db_username(NULL),
	_db_password(NULL),
	_db_name(NULL),
	_db_host(NULL),
	_logfile("slideshow.log"){

	initTime();

	if ( !parse_argv(argc, argv) ){
		exit(ARGUMENT_ERROR);
	}

	if ( !daemon() ){
		print_licence_statement();
	}

	Log::initialize(_logfile);
	Log::set_level( (Log::Severity)_verbose );

	print_cli_arguments(argc, argv);

	Log::flush();

	///@todo HACK! Attempt to connect to an xserver.
	Display* dpy = XOpenDisplay(NULL);
	if( !dpy ) {
		throw NoXConnection("Could not connect to an X server\n");
	}
	XCloseDisplay(dpy);

	Log::message(Log::Info, "Kernel: Starting slideshow\n");

	if ( daemon() ){
		start_daemon();
	}

	init_graphics();
	init_IPC();
	init_browser();
	init_fsm();
}

Kernel::~Kernel(){
	if ( daemon() ){
		Portable::daemon_stop(application_name);
	}

	delete _browser;
	delete _graphics;
	delete _ipc;

	free( _db_username );
	free( _db_password );
	free( _db_name );
	free( _db_host );

	_browser = NULL;
	_graphics = NULL;
	_ipc = NULL;

	Log::deinitialize();
}

void Kernel::init_graphics(){
	_graphics = new Graphics(_width, _height, _fullscreen);
	_graphics->set_transition(new SpinTransition);
}

void Kernel::init_IPC(){
	_ipc = new DBus(this, 50);
}

void Kernel::init_browser(){
	if ( !_db_password ){
		if ( !_stdin ){
			printf("Database password: \n");
		}
		_db_password = (char*)malloc(256);
		scanf("%256s", _db_password);
	}

	if ( _db_host ){
		_browser = new MySQLBrowser(_db_username, _db_password, _db_name, _db_host);
	} else {
		_browser = new MySQLBrowser(_db_username, _db_password, _db_name);
	}

	_browser->change_bin(_bin_id);
	_browser->reload();
}

void Kernel::init_fsm(){
	TransitionState::set_transition_time(_transition_time);
	ViewState::set_view_time(_switch_time);
	_state = new InitialState(_browser, _graphics, _ipc);
}

void Kernel::start_daemon(){
	Portable::daemonize(application_name);

	if ( signal(SIGQUIT, quit_signal) == SIG_ERR ){
		Log::message(Log::Fatal, "Kernel: Could not initialize signal handler!\n");
		exit(3);
	}

	///@ hack
	daemon_running = &_running;
}

void Kernel::print_licence_statement(){
	printf("Slideshow  Copyright (C) 2008 David Sveningsson <ext@sidvind.com>\n");
	printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
	printf("This is free software, and you are welcome to redistribute it\n");
	printf("under certain conditions; see COPYING or <http://www.gnu.org/licenses/>\n");
	printf("for details.\n");
}

void Kernel::print_cli_arguments(int argc, const char* argv[]){
	Log::message_begin(Log::Verbose);
	Log::message_ex("Starting with \"");

	for ( int i = 1; i < argc; i++ ){
		if ( i > 1 ){
			Log::message_ex(" ");
		}
		Log::message_ex_fmt("%s", argv[i]);
	}
	Log::message_ex("\n");
}

void Kernel::run(){
	_running = true;

	_last_switch = getTime();
	_transition_start = 0.0f;

	while ( _running ){
		OS::poll_events(_running);
		_state = _state->action();
	}
}

bool Kernel::parse_argv(int argc, const char* argv[]){
	option_set_t options;
	option_initialize(&options, argc, argv);

	option_set_description(&options, "Slideshow is an application for showing text and images in a loop on monitors and projectors.");

	option_add_flag(&options, "verbose", 'v', "Explain what is being done", &_verbose, 0);
	option_add_flag(&options, "quiet", 'q', "Explain what is being done", &_verbose, 2);
	option_add_flag(&options, "fullscreen", 'f', "Start in fullscreen mode", &_fullscreen, 1);
	option_add_flag(&options, "daemon", 'd', "Run in background", &_daemon, 1);
	option_add_flag(&options, "stdin", 0, "Except the input (e.g database password) to come from stdin", &_stdin, 1);
	option_add_string(&options, "db_user", 0, "Database username", &_db_username);
	option_add_string(&options, "db_pass", 0, "Database password", &_db_password);
	option_add_string(&options, "db_name", 0, "Database name", &_db_name);
	option_add_string(&options, "db_host", 0, "Database host [localhost]", &_db_host);
	option_add_int(&options, "collection-id", 'c', "ID of the collection to display", &_bin_id);
	option_add_format(&options, "resolution", 'r', "Resolution", "WIDTHxHEIGHT", "%dx%d", &_width, &_height);

	int n = option_parse(&options);
	option_finalize(&options);

	if ( n < 0 ){
		return false;
	}

	if ( n != argc ){
		printf("%d %d\n", n, argc);
		printf("%s: unrecognized option '%s'\n", argv[0], argv[n+1]);
		printf("Try `%s --help' for more information.\n", argv[0]);
		return false;
	}

	return true;
}

void Kernel::play_video(const char* fullpath){
	Log::message(Log::Info, "Kernel: Playing video \"%s\"\n", fullpath);

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

void Kernel::change_bin(unsigned int id){
	Log::message(Log::Verbose, "Kernel: Switching to collection %d\n", id);
	_browser->change_bin(id);
	_browser->reload();
}

void Kernel::ipc_quit(){
	delete _ipc;
	_ipc = NULL;
}

void Kernel::debug_dumpqueue(){
	_browser->dump_queue();
}

char* Kernel::real_path(const char* filename){
	const char* datadir = getenv("SLIDESHOW_DATA_DIR");
	if ( !datadir ){
		datadir = DATA_DIR;
	}

	char* dst;
	if ( asprintf(&dst, "%s/%s", datadir, filename) == -1 ){
		Log::message(Log::Fatal, "Memory allocation failed!");
		return NULL;
	}

	return dst;
}
