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

#include "Kernel.h"
#include "Exceptions.h"
#include <cstring>

int main( int argc, const char* argv[] ){
	try {
		Kernel kernel(argc, argv);
		kernel.run();

	} catch ( ExitException &e ){
		return 0;

	} catch ( FatalException &e ){

		// Only display message if there is one available.
		// Some exceptions like ArgumentException usually
		// print the error messages before throwing the
		// exception.
		if ( strlen(e.what()) > 0 ){
			fprintf(stderr, "%d %s\n", e.code(), e.what());
		}
		return e.code();

	} catch ( BaseException &e ){
		fprintf(stderr, "Uhh, unhandled exception, recovery not possible. The message was: %s\n", e.what());
		return UNHANDLED_ERROR;
	}

	return 0;
}
