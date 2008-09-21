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

#include "argument_parser.h"
#include "module.h"
#include "module_loader.h"
#include "Kernel.h"
#include "Graphics.h"
#include "OS.h"
#include "Log.h"
#include "Exceptions.h"
#include "Transition.h"

// IPC
#include "IPC/dbus.h"

// FSM
#include "state/State.h"
#include "state/InitialState.h"
#include "state/SwitchState.h"
#include "state/TransitionState.h"
#include "state/ViewState.h"

// libportable
#include <portable/Time.h>
#include <portable/string.h>

// libdaemon
#include <libdaemon/daemon.h>

// libc
#include <cstdlib>
#include <cstdio>
#include <cstring>

// Platform
#include <sys/wait.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <dirent.h>
#include <fnmatch.h>
#include <errno.h>

#ifdef LINUX
//#include <sys/time.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

bool* daemon_running = NULL;

static const int mode_normal = 0;
static const int mode_list_transitions = 1;

static char* pidfile = NULL;

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
	_mode(mode_normal),
	_transition_name(NULL),
	_transition_time(3.0f),
	_switch_time(5.0f),
	_graphics(NULL),
	_browser(NULL),
	_ipc(NULL),
	_browser_string(NULL),
	_logfile("slideshow.log"){

	initTime();
	moduleloader_init(pluginpath());

	if ( !parse_argv(argc, argv) ){
		throw ArgumentException("");
	}

	switch ( _mode ){
		case mode_list_transitions:
			print_transitions();
			throw ExitException();
	}

	if ( !daemon() ){
		print_licence_statement();
	}

	Log::initialize(_logfile);
	Log::set_level( (Log::Severity)_verbose );

	print_cli_arguments(argc, argv);

	Log::flush();

	///@todo HACK! Attempt to connect to an xserver.
	/*Display* dpy = XOpenDisplay(NULL);
	if( !dpy ) {
		throw XlibException("Could not connect to an X server\n");
	}
	XCloseDisplay(dpy);*/

	Log::message(Log::Info, "Kernel: Starting slideshow\n");

	create_pidpath();

	if ( daemon() ){
		daemon_start();
	}

	init_graphics();
	init_IPC();
	init_browser();
	init_fsm();

	if ( daemon() ){
		daemon_ready();
	}
}

Kernel::~Kernel(){
	if ( daemon() ){
		daemon_stop();
	}

	delete _state;
	delete _browser;
	delete _graphics;
	delete _ipc;
	free(pidfile);

	moduleloader_cleanup();

	_state = NULL;
	_browser = NULL;
	_graphics = NULL;
	_ipc = NULL;
	pidfile = NULL;

	Log::deinitialize();
}

void Kernel::init_graphics(){
	_graphics = new Graphics(_width, _height, _fullscreen);
	load_transition( _transition_name ? _transition_name : "fade" );
}

void Kernel::init_IPC(){
	_ipc = new DBus(this, 50);
}

void Kernel::init_browser(){
	char* password = get_password();

	_browser = Browser::factory(_browser_string, password);

	free(_browser_string);
	free(password);

	_browser_string = NULL;

	if ( browser() ){
		change_bin(_bin_id);
	} else {
		Log::message(Log::Warning, "No browser selected, you will not see any slides\n");
	}
}

char* Kernel::get_password(){
	if ( !_stdin ){
		return NULL;
	}

	char* password = (char*)malloc(256);
	scanf("%256s", password);

	return password;
}

void Kernel::init_fsm(){
	TransitionState::set_transition_time(_transition_time);
	ViewState::set_view_time(_switch_time);
	_state = new InitialState(_browser, _graphics, _ipc);
}

void Kernel::load_transition(const char* name){
	Log::message(Log::Warning, "Loading %s\n", name);
	struct module_context_t* context = module_open(name);

	if ( !context ){
		Log::message(Log::Info, "Transition plugin not found\n");
		return;
	}

	if ( module_type(context) != TRANSITION_MODULE ){
		Log::message(Log::Info, "Plugin is not a transition module not found\n");
		return;
	}

	transition_module_t* m = (transition_module_t*)module_get(context);
	_graphics->set_transition(m);
}

void Kernel::daemon_start(){
    pid_t pid;

    /* Reset signal handlers */
    if (daemon_reset_sigs(-1) < 0) {
    	throw KernelException("Kernel: failed to reset all signal handlers: %s\n", strerror(errno));
    }

    /* Unblock signals */
    if (daemon_unblock_sigs(-1) < 0) {
    	throw KernelException("Kernel: failed to unblock all signals: %s\n", strerror(errno));
    }

    /* Set indetification string for the daemon for both syslog and PID file */
    daemon_pid_file_proc = &Kernel::pidpath;
	daemon_log_ident = PACKAGE_NAME;

    /* Check that the daemon is not rung twice a the same time */
	if ((pid = daemon_pid_file_is_running()) >= 0) {
		throw KernelException("Kernel: daemon already running on PID file %u\n", pid);
	}

	/* Prepare for return value passing from the initialization procedure of the daemon process */
	daemon_retval_init();

	/* Do the fork */
	if ((pid = daemon_fork()) < 0) {

		/* Exit on error */
		daemon_retval_done();
		throw KernelException("Kernel: fork failed.\n");

	} else if (pid) { /* The parent */
		int ret;

		/* Wait for 20 seconds for the return value passed from the daemon process */
		if ((ret = daemon_retval_wait(20)) < 0) {
			throw KernelException("Kernel: could not receive return value from daemon process: %s\n", strerror(errno));
		}

		if ( ret != 0 ){
			throw KernelException("Kernel: daemon returned %i as return value.\n", ret);
		} else {
			throw ExitException();
		}

	} else { /* The daemon */
		/* Close FDs */
		if (daemon_close_all(-1) < 0) {
			Log::message(Log::Fatal, "Failed to close all file descriptors: %s\n", strerror(errno));

			/* Send the error condition to the parent process */
			daemon_retval_send(1);
			daemon_stop();
			exit(0);
		}

		/* Create the PID file */
		if (daemon_pid_file_create() < 0) {
			Log::message(Log::Fatal, "Could not create PID file (%s).\n", strerror(errno));
			daemon_retval_send(2);
			daemon_stop();
			exit(0);
		}

		/* Initialize signal handling */
		if (daemon_signal_init(SIGINT, SIGTERM, SIGQUIT, SIGHUP, 0) < 0) {
			Log::message(Log::Fatal, "Could not register signal handlers (%s).\n", strerror(errno));
			daemon_retval_send(3);
			daemon_stop();
			exit(0);
		}
	}
}

void Kernel::daemon_ready(){
	/* Send OK to parent process */
	daemon_retval_send(0);

	Log::message(Log::Info, "Sucessfully started\n");
	daemon_log(LOG_INFO, "Sucessfully started\n");

	/* Prepare for select() on the signal fd */
	FD_ZERO(&fds);
	fd = daemon_signal_fd();
	FD_SET(fd, &fds);
}

void Kernel::daemon_poll(bool& running){
	fd_set fds2 = fds;

	/* Wait for an incoming signal */
	if (select(FD_SETSIZE, &fds2, 0, 0, 0) < 0) {

		/* If we've been interrupted by an incoming signal, continue */
		if (errno == EINTR)
			return;

		Log::message(Log::Warning, "select(): %s\n", strerror(errno));
		return;
	}

	/* Check if a signal has been recieved */
	if (FD_ISSET(fd, &fds)) {
		int sig;

		/* Get signal */
		if ((sig = daemon_signal_next()) <= 0) {
			Log::message(Log::Warning, "daemon_signal_next() failed: %s\n", strerror(errno));
			return;
		}

		/* Dispatch signal */
		switch (sig) {

			case SIGINT:
			case SIGQUIT:
			case SIGTERM:
				Log::message(Log::Info, "Got SIGINT, SIGQUIT or SIGTERM\n");
				running = false;
				break;
		}
	}
}

void Kernel::daemon_stop(){
	Log::message(Log::Info, "Exiting\n");
	daemon_log(LOG_INFO, "Exiting");

	daemon_retval_send(255);
	daemon_signal_done();
	daemon_pid_file_remove();
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

static int filter(const struct dirent* el){
	return fnmatch("*.la", el->d_name, 0) == 0;
}

void Kernel::print_transitions(){
	struct dirent **namelist;
	int n;

	n = scandir(pluginpath(), &namelist, filter, alphasort);
	if (n < 0){
		perror("scandir");
	} else {
		printf("Available transitions: \n");
		for ( int i = 0; i < n; i++ ){
			char* path;
			asprintf(&path, "%s/%s", pluginpath(), namelist[i]->d_name);
			free(namelist[i]);

			struct module_context_t* context = module_open(path);

			if ( !context ){
				continue;
			}

			if ( module_type(context) != TRANSITION_MODULE ){
				continue;
			}

			printf("%s\n", module_get_name(context));

			module_close(context);
		}
		free(namelist);
	}
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

	option_add_flag(&options,	"verbose",			'v', "Explain what is being done", &_verbose, 0);
	option_add_flag(&options,	"quiet",			'q', "Explain what is being done", &_verbose, 2);
	option_add_flag(&options,	"fullscreen",		'f', "Start in fullscreen mode", &_fullscreen, 1);
	option_add_flag(&options,	"daemon",			'd', "Run in background", &_daemon, 1);
	option_add_flag(&options,	"list-transitions",	 0,  "List available transitions", &_mode, mode_list_transitions);
	option_add_flag(&options,	"stdin-password",	 0,  "Except the input (e.g database password) to come from stdin", &_stdin, 1);
	option_add_string(&options,	"browser",			 0,  "Browser connection string. provider://user[:pass]@host[:port]/name", &_browser_string);
	option_add_string(&options,	"transition",		't', "Set slide transition plugin [fade]", &_transition_name);
	option_add_int(&options,	"collection-id",	'c', "ID of the collection to display", &_bin_id);
	option_add_format(&options,	"resolution",		'r', "Resolution", "WIDTHxHEIGHT", "%dx%d", &_width, &_height);

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
	char* dst;

	if ( filename[0] == '/' ){
		dst = (char*)malloc(strlen(filename)+1);
		strcpy(dst, filename);
	} else {
		if ( asprintf(&dst, "%s/%s", datapath(), filename) == -1 ){
			Log::message(Log::Fatal, "Memory allocation failed!");
			return NULL;
		}
	}

	return dst;
}

const char* Kernel::datapath(){
	const char* path = getenv("SLIDESHOW_DATA_DIR");
	if ( !path ){
		path = DATA_DIR;
	}
	return path;
}

const char* Kernel::pluginpath(){
	const char* path = getenv("SLIDESHOW_PLUGIN_DIR");
	if ( !path ){
		path = PLUGIN_DIR;
	}
	return path;
}
void Kernel::create_pidpath(){
	char* cwd = get_current_dir_name();
	asprintf(&pidfile, "%s/slideshow.pid", cwd);
	free(cwd);
}

const char* Kernel::pidpath(){
	return pidfile;
}
