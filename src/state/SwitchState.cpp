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

#include "SwitchState.h"
#include "TransitionState.h"
#include "ViewState.h"
#include "exception.h"
#include "Log.h"

State* SwitchState::action(bool &flip){
	if ( !browser() ){
		return new ViewState(this);
	}

	struct autofree {
		autofree(char* ptr):
			ptr(ptr){}
		~autofree(){
			free(ptr);
		}
		char* ptr;
	};

	/* get next filename */
	autofree filename(browser()->get_next_file());

	if ( !filename.ptr ){
		Log::message(Log::Warning, "Kernel: Queue is empty\n", filename.ptr);
	} else {
		Log::message(Log::Debug, "Kernel: Switching to image \"%s\"\n", filename.ptr);
	}

	try {
		gfx()->load_image( filename.ptr );
	} catch ( exception& e ) {
		Log::message(Log::Warning, "Kernel: Failed to load image '%s': %s\n", filename.ptr, e.what());
		return new ViewState(this);
	}

	return new TransitionState(this);
}
