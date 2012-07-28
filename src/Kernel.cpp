/**
 * This file is part of Slideshow.
 * Copyright (C) 2008-2010 David Sveningsson <ext@sidvind.com>
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
#include "graphics.h"
#include "OS.h"
#include "path.h"
#include "Log.h"
#include "exception.h"
#include "Transition.h"
#include "state/VideoState.h" /* must be initialized */

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

// Backend
#include "backend/platform.h"

// libportable
#include <portable/time.h>
#include <portable/asprintf.h>
#include <portable/scandir.h>
#include <portable/cwd.h>

// libc
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>

// Loading settings
#include <curl/curl.h>
#include <json/json.h>
#include "curl_local.h"

// Platform
#ifdef __GNUC__
#	include <sys/wait.h>
#endif

#ifdef WIN32
#	include "win32.h"
#	include <direct.h>
#	define getcwd _getcwd
#	define PATH_MAX _MAX_PATH
#endif

static char* pidfile = NULL;
static CURL* curl_handle_settings = NULL;
static struct curl_httppost* settings_formpost = NULL;

Kernel::Kernel(const argument_set_t& arg, PlatformBackend* backend)
	: _arg(arg)
	, _password(NULL)
	, _state(NULL)
	, _browser(NULL)
	, _ipc(NULL)
	, _backend(backend)
	, _running(false) {

	verify(_backend);

	create_pidpath();
	_password = get_password();
}

Kernel::~Kernel(){
	delete _backend;

	free( _arg.connection_string );
	free( _arg.transition_string );
	free( _arg.url );
}

void Kernel::init(){
	Log::info("Kernel: Starting slideshow\n");

	init_backend();
	init_graphics();
	init_IPC();
	init_browser();
	init_fsm();
	VideoState::init();
}

void Kernel::cleanup(){
	VideoState::cleanup();
	delete _state;
	module_close(&_browser->module);
	graphics_cleanup();
	free(pidfile);
	free(_password);

	cleanup_IPC();
	cleanup_backend();

	_state = NULL;
	_browser = NULL;
	pidfile = NULL;
}

void Kernel::init_backend(){
	_backend->init(Vector2ui(_arg.width, _arg.height), _arg.fullscreen > 0);
}

void Kernel::cleanup_backend(){
	_backend->cleanup();
}

void Kernel::init_graphics(){
	graphics_init(_arg.width, _arg.height);
	graphics_set_transition( _arg.transition_string ? _arg.transition_string : "fade" );
}

void Kernel::init_IPC(){
#ifdef HAVE_DBUS
	_ipc = new DBus(this, 50);
#endif /* HAVE_DBUS */

	if ( _arg.url ){
		char* settings_url = asprintf2("%s/instance/settings", _arg.url);

		/* get json data */
		curl_handle_settings = curl_easy_init();
		struct curl_httppost *lastptr = NULL;
		curl_formadd(&settings_formpost, &lastptr, CURLFORM_COPYNAME, "name", CURLFORM_COPYCONTENTS, _arg.instance, CURLFORM_END);
		curl_easy_setopt(curl_handle_settings, CURLOPT_URL, settings_url);
		curl_easy_setopt(curl_handle_settings, CURLOPT_HTTPPOST, settings_formpost);
		curl_easy_setopt(curl_handle_settings, CURLOPT_WRITEFUNCTION, curl_local_resize);
		free(settings_url);
	}
}

void Kernel::cleanup_IPC(){
	delete _ipc;
	_ipc = NULL;

	curl_easy_cleanup(curl_handle_settings);
	curl_formfree(settings_formpost);
}

void Kernel::init_browser(){
	browser_context_t context;

	if ( _arg.connection_string ){
		context = get_context(_arg.connection_string);
	} else if ( _arg.url ) {
		/* default to frontend browser */
		context.provider = strdup("frontend");
		context.user     = NULL;
		context.pass     = NULL;
		context.host     = strdup(_arg.url);
		context.name     = strdup(_arg.instance);
	} else {
		Log::fatal("No browser provided.\n");
		return;
	}

	// If the contex doesn't contain a password and a password was passed from stdin (arg password)
	// we set that as the password in the context.
	if ( !context.pass && _password ){
		context.pass = strdup(_password);
	}

	_browser = (struct browser_module_t*)module_open(context.provider, BROWSER_MODULE, MODULE_CALLER_INIT);

	if ( !_browser ){
		Log::warning("Failed to load browser plugin '%s': %s\n", context.provider, module_error_string());
		return;
	}

	/* setup defalts */
	_browser->context = context;
	_browser->next_slide = NULL;
	_browser->queue_reload = browser_default_queue_reload;
	_browser->queue_dump = browser_default_queue_dump;
	_browser->queue_set = browser_default_queue_set;

	/* initialize browser */
	if ( _browser->module.init ){
		_browser->module.init((module_handle)_browser);
	}

	if ( !_browser->next_slide ){
		Log::warning("Browser plugin `%s' does not implement next_slide function (no slides can be retrieved).\n", context.provider);
		_browser = NULL;
		return;
	}

	/* assertions */
	assert(_browser->next_slide);
	assert(_browser->queue_reload);
	assert(_browser->queue_dump);
	assert(_browser->queue_set);

	change_bin(_arg.queue_id);
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
	_state = new InitialState(_browser, _ipc);
}

void Kernel::load_transition(const char* name){
	graphics_set_transition(name);
}

void Kernel::poll(){
	_backend->poll(_running);
	VideoState::poll();
}

void Kernel::action(){
	if ( !_state ){
		return;
	}

	bool flip = false;

	try {
		_state = _state->action(flip);
	} catch ( exception& e ){
		Log::warning("State exception: %s\n", e.what());
		_state = NULL;
	}

	if ( flip ){
		_backend->swap_buffers();
	}
}

void Kernel::print_config() const {
	char* cwd = get_current_dir_name();

	Log::info("Slideshow configuration\n");
	Log::info("  cwd: %s\n", cwd);
	Log::info("  pidfile: %s\n", pidfile);
	Log::info("  datapath: %s\n", datapath());
	Log::info("  pluginpath: %s\n", pluginpath());
	Log::info("  resolution: %dx%d (%s)\n", _arg.width, _arg.height, _arg.fullscreen ? "fullscreen" : "windowed");
	Log::info("  transition time: %0.3fs\n", _arg.transition_time);
	Log::info("  switch time: %0.3fs\n", _arg.switch_time);
	Log::info("  connection string: %s\n", _arg.connection_string);
	Log::info("  transition: %s\n", _arg.transition_string);
	Log::info("\n");

	free(cwd);
}

void Kernel::print_licence_statement() const {
	Log::info("Slideshow  Copyright (C) 2008-2012 David Sveningsson <ext@sidvind.com>\n");
	Log::info("This program comes with ABSOLUTELY NO WARRANTY.\n");
	Log::info("This is free software, and you are welcome to redistribute it\n");
	Log::info("under certain conditions; see COPYING or <http://www.gnu.org/licenses/>\n");
	Log::info("for details.\n");
	Log::info("\n");
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
	Log::info("Available transitions: \n");

	struct dirent **namelist;
	int n;

	char* path_list = strdup(pluginpath());
	char* ctx;
	char* path = strtok_r(path_list, ":", &ctx);
	while ( path ){
		n = scandir(path, &namelist, filter, NULL);
		if (n < 0){
			perror("scandir");
		} else {
			for ( int i = 0; i < n; i++ ){
				module_handle context = module_open(namelist[i]->d_name, ANY_MODULE, MODULE_CALLEE_INIT);
				free(namelist[i]);

				if ( !context ){
					continue;
				}

				if ( module_type(context) != TRANSITION_MODULE ){
					continue;
				}

				Log::info(" * %s\n", module_get_name(context));

				module_close(context);
			}
			free(namelist);
		}

		path = strtok_r(NULL, ":", &ctx);
	}

	free(path_list);
}

bool Kernel::parse_arguments(argument_set_t& arg, int argc, const char* argv[]){
	option_set_t options;
	char* directory = NULL;

	option_initialize(&options, argc, argv);
	option_set_description(&options, "Slideshow is an application for showing text and images in a loop on monitors and projectors.");
	option_set_helpline(&options, "FRONTEND-URL");

	option_add_flag(&options,   "verbose",          'v', "Include debugging messages in log.", &arg.loglevel, Log_Debug);
	option_add_flag(&options,   "quiet",            'q', "Show only warnings and errors in log.", &arg.loglevel, Log_Warning);
	option_add_flag(&options,   "fullscreen",       'f', "Start in fullscreen mode", &arg.fullscreen, true);
	option_add_flag(&options,   "window",           'w', "Start in windowed mode [default]", &arg.fullscreen, false);
	option_add_flag(&options,   "daemon",           'd', "Run in background mode", &arg.mode, DaemonMode);
	option_add_flag(&options,   "foreground",       'd', "Run in foreground mode", &arg.mode, ForegroundMode);
	option_add_flag(&options,   "list-transitions",  0,  "List available transitions", &arg.mode, ListTransitionMode);
	option_add_flag(&options,   "stdin-password",    0,  "Except the input (e.g database password) to come from stdin", &arg.have_password, true);
	option_add_string(&options, "browser",           0,  "Browser connection string. provider://user[:pass]@host[:port]/name [use frontend]", &arg.connection_string);
	option_add_string(&options, "directory",         0,  "Use directory browser (short for directory://PATH) (--browser has precedence)", &directory);
	option_add_string(&options, "transition",       't', "Set slide transition plugin [fade]", &arg.transition_string);
	option_add_int(&options,    "collection-id",    'c', "ID of the queue to display (deprecated, use `--queue-id')",  &arg.queue_id);
	option_add_int(&options,    "queue-id",         'c', "ID of the queue to display", &arg.queue_id);
	option_add_format(&options, "resolution",       'r', "Resolution", "WIDTHxHEIGHT", "%dx%d", &arg.width, &arg.height);
	option_add_string(&options, "name",             'n', "Instance name [machine hostname]", &arg.instance);

	/* logging options */
	option_add_string(&options, "file-log",          0,  "Log to regular file (appending)", &arg.log_file);
	option_add_string(&options, "fifo-log",          0,  "Log to a named pipe", &arg.log_fifo);
	option_add_string(&options, "uds-log",           0,  "Log to a unix domain socket", &arg.log_domain);

	int n = option_parse(&options);
	option_finalize(&options);

	/* If no browser was provided but the --directory option was given construct a browser-string from it */
	if ( !arg.connection_string && directory ){
		arg.connection_string = asprintf2("directory://%s", directory);
		free(directory);
	}

	if ( n < 0 ){
		return false;
	}

	if ( n == argc ){
		printf("%s: Warning: No frontend url given.\n", argv[0]);
		printf("Try `%s --help' for more information.\n", argv[0]);
	} else {
		arg.url = strdup(argv[n+1]);
	}

	/* use machine hostname by default */
	if ( !arg.instance ){
		arg.instance = (char*)malloc(1024);
		gethostname(arg.instance, 1024);
	}

	return true;
}

void Kernel::play_video(const char* fullpath){
#ifndef WIN32
	Log::verbose("Kernel: Playing video \"%s\"\n", fullpath);

	int status;

	if ( fork() == 0 ){
		execlp("mplayer", "", "-fs", "-really-quiet", fullpath, NULL);
		exit(0);
	}

	::wait(&status);
#else /* WIN32 */
	Log::warning("Kernel: Video playback is not supported on this platform (skipping \"%s\")\n", fullpath);
#endif /* WIN32 */
}

void Kernel::start(){
	_running = true;
}

void Kernel::quit(){
	_running = false;
}

void Kernel::reload_browser(){
	if ( _browser ){
		_browser->queue_reload(_browser);
	}

	if ( curl_handle_settings ){
		struct MemoryStruct chunk;
		long response;

		chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
		chunk.size = 0;    /* no data at this point */

		/* get json data */
		curl_easy_setopt(curl_handle_settings, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_perform(curl_handle_settings);
		curl_easy_getinfo(curl_handle_settings, CURLINFO_RESPONSE_CODE, &response);

		/* get json data */
		curl_easy_setopt(curl_handle_settings, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_perform(curl_handle_settings);
		curl_easy_getinfo(curl_handle_settings, CURLINFO_RESPONSE_CODE, &response);

		if ( response != 200 ){ /* HTTP OK */
			Log::warning("Server replied with code %ld\n", response);
			return;
		}

		/* parse */
		json_object* settings = json_tokener_parse(chunk.memory);
		if ( !settings ){
			Log::warning("Failed to parse settings");
			return;
		}

		json_object_object_foreach(settings, key, value) {
			/* @todo map */
			if ( strcasecmp(key, "queue") == 0 ){
				change_bin(json_object_get_int(value));
				continue;
			}

			if ( strcasecmp(key, "transitiontime") == 0 ){
				TransitionState::set_transition_time((float)json_object_get_double(value));
				continue;
			}

			if ( strcasecmp(key, "switchtime") == 0 ){
				ViewState::set_view_time(json_object_get_double(value));
				continue;
			}

			Log::warning("Unhandled setting %s: %s\n", key, json_object_to_json_string(value));
		}

		json_object_put(settings);
	}
}

void Kernel::change_bin(unsigned int id){
	Log::verbose("Kernel: Switching to queue %d\n", id);
	if ( _browser ){
		_browser->queue_set(_browser, id);
		_browser->queue_reload(_browser);
	}
}

void Kernel::ipc_quit(){
	delete _ipc;
	_ipc = NULL;
}

void Kernel::debug_dumpqueue(){
	if ( _browser ){
		_browser->queue_dump(_browser);
	}
}

void Kernel::create_pidpath(){
	char* cwd = get_current_dir_name();
	verify( asprintf(&pidfile, "%s/slideshow.pid", cwd) >= 0 );
	free(cwd);
}

const char* Kernel::pidpath(){
	return pidfile;
}
