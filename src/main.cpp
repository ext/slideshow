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

#include "Kernel.h"
#include "ForegroundApp.h"

#ifdef BUILD_DAEMON
#	include "DaemonApp.h"
#endif /* BUILD_DAEMON */

#include "Log.h"
#include "Exceptions.h"
#include "module_loader.h"
#include "path.h"
#include <cstring>
#include <portable/Time.h>
#include "backend/platform.h"

int main( int argc, const char* argv[] ){
	try {

		// Default arguments
		Kernel::argument_set_t arguments = {
			Kernel::ForegroundMode, // mode
			Log::Info, 				// loglevel
			false,					// fullscreen
			false,					// have_password
			0, 						// collection_id
			800,					// width
			600,					// height
			3.0f,					// transition_time;
			5.0f,					// switch_time;
			NULL,					// connection_string
			NULL					// transition_string
		};

		// Parse the cli arguments, overriding the defaults
		if ( !Kernel::parse_arguments(arguments, argc, argv) ){
			throw ArgumentException("");
		}

		initTime();
		moduleloader_init(pluginpath());
		PlatformBackend::register_all();

		Log::initialize("slideshow.log");
		Log::set_level( (Log::Severity)arguments.loglevel );

		Kernel* application = NULL;
		PlatformBackend* backend = PlatformBackend::factory("sdl");

		switch ( arguments.mode ){
			case Kernel::ForegroundMode:
				application = new ForegroundApp(arguments, backend);
				break;
			case Kernel::DaemonMode: 
#ifdef BUILD_DAEMON
				application = new DaemonApp(arguments, backend); break;
#else /* BUILD_DAEMON */
				throw KernelException("DaemonMode is not supported.");
#endif /* BUILD_DAEMON */
			case Kernel::ListTransitionMode:
				Kernel::print_transitions();
				throw ExitException();
			default:
				throw KernelException("No valid mode. This should not happen, please report this to the maintainer. Modeid: %d\n", arguments.mode);
		}

		application->init();
		application->run();
		application->cleanup();

		delete application;

		moduleloader_cleanup();

		Log::deinitialize();

	} catch ( ExitException ){
		return 0;

	} catch ( FatalException &e ){

		// Only display message if there is one available.
		// Some exceptions like ArgumentException usually
		// print the error messages before throwing the
		// exception.
		if ( e.what() && strlen(e.what()) > 0 ){
			fprintf(stderr, "\nError 0x%02x:\n%s\n", e.code(), e.what());
		}
		return e.code();

	} catch ( BaseException &e ){
		fprintf(stderr, "Uhh, unhandled exception, recovery not possible. The message was: %s\n", e.what());
		return UNHANDLED_ERROR;
	}

	return 0;
}
