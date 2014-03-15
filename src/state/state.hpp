/**
 * This file is part of Slideshow.
 * Copyright (C) 2008-2012 David Sveningsson <ext@sidvind.com>
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

#ifndef STATE_STATE_HPP
#define STATE_STATE_HPP

#include "browsers/browser.h"
#include <portable/time.h>

class State {
public:
	State(browser_module_t* browser): _browser(browser) {
		_created = getTimef();
	}
	virtual ~State(){}

	State(State* state): _browser(state->_browser) {
		_created = getTimef();
		delete state;
	}

	virtual State* action(bool &flip) = 0;

	slide_context_t next_slide(){
		return _browser->next_slide(_browser);
	}

	browser_module_t* browser(){ return _browser; }

	float age(){ return getTimef() - _created; }

private:
	browser_module_t* _browser;
	float _created;
};

#endif /* STATE_STATE_HPP */
