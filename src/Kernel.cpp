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

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "argument_parser.h"
#include "module.h"
#include "module_loader.h"
#include "Kernel.h"
#include "Graphics.h"
#include "OS.h"
#include "path.h"
#include "Log.h"
#include "Exceptions.h"
#include "Transition.h"

// IPC
#ifdef HAVE_DBUS
#	include "IPC/dbus.h"
#endif /* HAVE_DBUS */

// FSM
#include "state/State.h"
#include "state/InitialState.h"
#include "state/SwitchState.h"
#include "state/TransitionState.h"
#include "state/ViewState.h"

// libportable
#include <portable/Time.h>
#include <portable/asprintf.h>
#include <portable/scandir.h>
#include <portable/cwd.h>

// libc
#include <cstdlib>
#include <cstdio>
#include <cstring>

// Platform
#ifdef LINUX
//#include <sys/time.h>
#	include <sys/wait.h>
#	include <signal.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

static char* pidfile = NULL;

Kernel::Kernel(const argument_set_t& arg)
	: _arg(arg)
	, _password(NULL)
	, _state(NULL)
	, _graphics(NULL)
	, _browser(NULL)
	, _ipc(NULL)
	, _running(false) {

	create_pidpath();
	_password = get_password();
}

Kernel::~Kernel(){

}

void Kernel::init(){
	Log::message(Log::Info, "Kernel: Starting slideshow\n");

	init_graphics();
	init_IPC();
	init_browser();
	init_fsm();
}

void Kernel::cleanup(){
	delete _state;
	delete _browser;
	delete _graphics;
	delete _ipc;
	free(pidfile);
	free(_password);

	_state = NULL;
	_browser = NULL;
	_graphics = NULL;
	_ipc = NULL;
	pidfile = NULL;
}

void Kernel::init_graphics(){
	_graphics = new Graphics(_arg.width, _arg.height, _arg.fullscreen);
	load_transition( _arg.transition_string ? _arg.transition_string : "fade" );
}

void Kernel::init_IPC(){
#ifdef HAVE_DBUS
	_ipc = new DBus(this, 50);
#endif /* HAVE_DBUS */
}

void Kernel::init_browser(){
	_browser = Browser::factory(_arg.connection_string, _password);

	if ( browser() ){
		change_bin(_arg.collection_id);
	} else {
		Log::message(Log::Warning, "No browser selected, you will not see any slides\n");
	}
}

char* Kernel::get_password(){
	if ( !_arg.have_password ){
		return NULL;
	}

	char* password = (char*)malloc(256);
	verify( scanf("%256s", password) == 1 );

	return password;
}

void Kernel::init_fsm(){
	TransitionState::set_transition_time(_arg.transition_time);
	ViewState::set_view_time(_arg.switch_time);
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

void Kernel::poll(){
	OS::poll_events(_running);
}

void Kernel::action(){
	_state = _state->action();
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

#ifdef WIN32
#	define SO_SUFFIX ".dll"
#else
#	define SO_SUFFIX ".la"
#endif

static int filter(const struct dirent* el){
	return fnmatch("*" SO_SUFFIX , el->d_name, 0) == 0;
}

void Kernel::print_transitions(){
	printf("Available transitions: \n");

	struct dirent **namelist;
	int n;

	char* path_list = strdup(pluginpath());
	char* path = strtok(path_list, ":");
	while ( path ){
		n = scandir(path, &namelist, filter, NULL);
		if (n < 0){
			perror("scandir");
		} else {
			for ( int i = 0; i < n; i++ ){
				struct module_context_t* context = module_open(namelist[i]->d_name);
				free(namelist[i]);

				if ( !context ){
					continue;
				}

				if ( module_type(context) != TRANSITION_MODULE ){
					continue;
				}

				printf(" * %s\n", module_get_name(context));

				module_close(context);
			}
			free(namelist);
		}

		path = strtok(NULL, ":");
	}

	free(path_list);
}

bool Kernel::parse_arguments(argument_set_t& arg, int argc, const char* argv[]){
	option_set_t options;

	option_initialize(&options, argc, argv);
	option_set_description(&options, "Slideshow is an application for showing text and images in a loop on monitors and projectors.");

	option_add_flag(&options,	"verbose",			'v', "Explain what is being done", &arg.loglevel, Log::Debug);
	option_add_flag(&options,	"quiet",			'q', "Explain what is being done", &arg.loglevel, Log::Warning);
	option_add_flag(&options,	"fullscreen",		'f', "Start in fullscreen mode", &arg.fullscreen, true);
	option_add_flag(&options,	"daemon",			'd', "Run in background", &arg.mode, DaemonMode);
	option_add_flag(&options,	"list-transitions",	 0,  "List available transitions", &arg.mode, ListTransitionMode);
	option_add_flag(&options,	"stdin-password",	 0,  "Except the input (e.g database password) to come from stdin", &arg.have_password, true);
	option_add_string(&options,	"browser",			 0,  "Browser connection string. provider://user[:pass]@host[:port]/name", &arg.connection_string);
	option_add_string(&options,	"transition",		't', "Set slide transition plugin [fade]", &arg.transition_string);
	option_add_int(&options,	"collection-id",	'c', "ID of the collection to display", &arg.collection_id);
	option_add_format(&options,	"resolution",		'r', "Resolution", "WIDTHxHEIGHT", "%dx%d", &arg.width, &arg.height);

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
#ifndef WIN32
	Log::message(Log::Info, "Kernel: Playing video \"%s\"\n", fullpath);

	int status;

	if ( fork() == 0 ){
		execlp("mplayer", "", "-fs", "-really-quiet", fullpath, NULL);
		exit(0);
	}

	::wait(&status);
#else /* WIN32 */
	Log::message(Log::Warning, "Kernel: Video playback is not supported on this platform (skipping \"%s\")\n", fullpath);
#endif /* WIN32 */
}

void Kernel::start(){
	_running = true;
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

void Kernel::create_pidpath(){
	char* cwd = get_current_dir_name();
	verify( asprintf(&pidfile, "%s/slideshow.pid", cwd) >= 0 );
	free(cwd);
}

const char* Kernel::pidpath(){
	return pidfile;
}
