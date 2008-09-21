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

#ifdef LINUX
//#include <sys/time.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

static char* pidfile = NULL;

Kernel::Kernel(const argument_set_t& arg):
	_arg(arg),
	_password(NULL){
	create_pidpath();
	_password = get_password();
}

Kernel::~Kernel(){

}

void Kernel::init(){
	Log::message(Log::Info, "Kernel: Starting slideshow\n");

	moduleloader_init(pluginpath());

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

	moduleloader_cleanup();

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
	_ipc = new DBus(this, 50);
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

	Log::message(Log::Info, "Getting password\n");
	char* password = (char*)malloc(256);
	scanf("%256s", password);

	Log::message(Log::Info, "Got %s\n", password);
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
	Log::message(Log::Info, "Kernel: Playing video \"%s\"\n", fullpath);

	int status;

	if ( fork() == 0 ){
		execlp("mplayer", "", "-fs", "-really-quiet", fullpath, NULL);
		exit(0);
	}

	::wait(&status);
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
