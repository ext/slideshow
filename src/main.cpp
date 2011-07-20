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

#include "Kernel.h"
#include "ForegroundApp.h"

#ifdef BUILD_DAEMON
#	include "DaemonApp.h"
#endif /* BUILD_DAEMON */

#include "Log.h"
#include "exception.h"
#include "module_loader.h"
#include "path.h"
#include <cstring>
#include <cstdlib>
#include <limits.h>
#include <signal.h>
#include <portable/time.h>
#include "backend/platform.h"
#include "Browser.h"

// for getcwd
#ifdef WIN32
#	include <direct.h>
#	define getcwd _getcwd
#	define PATH_MAX _MAX_PATH
#endif

static Kernel* application = NULL;

static void sighandler(int signum){
	switch ( signum ){
	case SIGINT:
		Log::message(Log_Verbose, "IPC: Quit\n");
		application->quit();
		break;
	case SIGHUP:
		Log::message(Log_Verbose, "IPC: Reload browser\n");
		application->reload_browser();
		signal(SIGHUP, sighandler);
		break;
	}
}

int main( int argc, const char* argv[] ){
	try {

		// Default arguments
		Kernel::argument_set_t arguments = {
			Kernel::ForegroundMode, // mode
			Log_Info, 				// loglevel
			false,					// fullscreen
			false,					// have_password
			0, 						// queue_id
			800,					// width
			600,					// height
			3.0f,					// transition_time;
			5.0f,					// switch_time;
			NULL,					// connection_string
			NULL,					// transition_string
			NULL,					// file log
			NULL,					// named pipe log
			NULL,					// unix domain socket log

			NULL,                   // Frontend URL.
			NULL,                   // Instance name.
		};

		// Parse the cli arguments, overriding the defaults
		if ( !Kernel::parse_arguments(arguments, argc, argv) ){
			throw exception("Failed to parse commandline arguments");
		}

		initTime();
		moduleloader_init(pluginpath());
		PlatformBackend::register_all();

		Log::initialize();

		/* only log to stdout if no other destination has been set */
		if ( !(arguments.log_file || arguments.log_fifo || arguments.log_domain) ){
			Log::add_destination(new FileDestination(stdout));
		}

		/* setup file log */
		if ( arguments.log_file ){
			Log::add_destination(new FileDestination(arguments.log_file));
		}

		/* setup named pipe log */
		if ( arguments.log_fifo ){
			Log::add_destination(new FIFODestination(arguments.log_fifo));
		}

		/* setup unix domain socket log */
		if ( arguments.log_domain ){
			UDSServer d(arguments.log_domain);
			d.accept(NULL);
		}

#ifdef HAVE_SYSLOG
		Log::add_destination(new SyslogDestination());
#endif /* HAVE_SYSLOG */
		//Log::set_level( (Log::Severity)arguments.loglevel );

		/* Kernel takes ownership of backend and will release memory when finished */
		const char* backend_name = "sdl";
		PlatformBackend* backend = PlatformBackend::factory(backend_name);
		if ( !backend ){
			throw exception("Failed to create a backend named \"%s\"", backend_name);
		}

		switch ( arguments.mode ){
			case Kernel::ForegroundMode:
				application = new ForegroundApp(arguments, backend);
				break;
			case Kernel::DaemonMode:
#ifdef BUILD_DAEMON
				application = new DaemonApp(arguments, backend); break;
#else /* BUILD_DAEMON */
				throw exception("DaemonMode is not supported on this platform.");
#endif /* BUILD_DAEMON */
			case Kernel::ListTransitionMode:
				Kernel::print_transitions();
				throw ExitException();
			default:
				throw exception("No valid mode. This should not happen, please report this to the maintainer. Modeid: %d\n", arguments.mode);
		}

		signal(SIGINT, sighandler);
		signal(SIGHUP, sighandler);

		application->init();
		application->run();
		application->cleanup();

		delete application;
		application = NULL;

		moduleloader_cleanup();
		PlatformBackend::register_cleanup();

		Log::cleanup();

	} catch ( ExitException &e ){
		printf("exit exception\n");
		return e.code();

	} catch ( exception &e ){
		/* Handle unhandled exceptions, if anythin makes it here it is a fatal
		 * error and it is not possible to continue operation. */

		char cwd[PATH_MAX];

		if ( getcwd(cwd, 256) == NULL ){
			fprintf(stderr, "Failed to get cwd\n");
			cwd[0] = '\0';
		}

		fprintf(stderr, " *** slideshow unhandled exception ***\n");
		fprintf(stderr, "\tcwd:     %s\n", cwd);
		fprintf(stderr, "\tSource:  %s:%d\n", e.file(), e.line());
		fprintf(stderr, "\tMessage: %s\n\n", e.what());
		fprintf(stderr, "Troubleshooting:\n");
		fprintf(stderr, " - Make sure that all required dll's are installed.\n");
		fprintf(stderr, " - Make sure that the cwd is correct.\n\n");
		fprintf(stderr, "If the problem persists report the bug at\n"
				"http://sidvind.com:8000/slideshow/newticket\n"
				"and copy the entire output from the console.\n\n");
		fprintf(stderr, "This is a fatal error, the application will now terminate!\n\n");

		fflush(stderr);

		if ( getenv("SLIDESHOW_NO_ABORT") ) {
			return -1;
		} else {
#ifdef WIN32
			__debugbreak();
#elif defined(__GNUC__)
			__builtin_trap();
#else
			abort();
#endif
		}

		/* silence compiler warnings */
		return -1;
	}

	return 0;
}
