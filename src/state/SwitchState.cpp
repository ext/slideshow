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
#include "VideoState.h"
#include "ViewState.h"
#include "exception.h"
#include "Log.h"
#include <cstring>

State* SwitchState::action(bool &flip){
	if ( !browser() ){
		return new ViewState(this);
	}

	/* get next slide */
	slide_context_t slide = next_slide();

	struct autofree_t {
		autofree_t(slide_context_t& s): s(s){}
		~autofree_t(){
			free(s.filename);
			free(s.assembler);
		}
		slide_context_t& s;
	};

	autofree_t container(slide);

	if ( !(slide.filename && slide.assembler) ){
		/* The current queue is empty, load a blank screen instead of keeping the
		 * current slide. It makes more sense that the screen goes blank when removing 
		 * all the slides from the queue. */
		Log::warning("Kernel: Queue is empty\n");
		gfx()->load_image(NULL); /* blank screen */
		return new TransitionState(this);
	}

	/* @todo make something factory-like */
	if ( strcmp("image", slide.assembler) == 0 || strcmp("text", slide.assembler) == 0 ){
		Log::verbose("Kernel: Switching to image \"%s\"\n", slide.filename);

		try {
			gfx()->load_image( slide.filename );
		} catch ( exception& e ) {
			Log::warning("Kernel: Failed to load image '%s': %s\n", slide.filename, e.what());
			return new ViewState(this);
		}

		return new TransitionState(this);
	} else if ( strcmp("video", slide.assembler) == 0 ){
		Log::debug("Kernel: Playing video \"%s\"\n", slide.filename);
		return new VideoState(this, slide.filename);
	} else {
		Log::warning("Unhandled assembler \"%s\" for \"%s\"\n", slide.assembler, slide.filename);
		return new ViewState(this);
	}
}
