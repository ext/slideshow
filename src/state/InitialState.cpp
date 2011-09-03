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

#include "InitialState.h"
#include "TransitionState.h"
#include "exception.h"
#include "Log.h"

State* InitialState::action(bool &flip){
	try {
		gfx()->load_image(NULL);
		gfx()->load_image("resources/splash.png", false);
	} catch ( exception &e ){
		Log::warning("%s\n", e.what());
		Log::warning("Failed to load initial resources, check your configuration\n");
	}

	gfx()->render(0.0);
	flip = true;

	return new TransitionState(this);
}
