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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "state/state.hpp"
#include <time.h>
#include <sys/time.h>

static unsigned long utime(){
	struct timeval cur;
	gettimeofday(&cur, NULL);
	return (unsigned long)(cur.tv_sec * 1000000 + cur.tv_usec);
}

State::State(browser_module_t* browser)
	: _browser(browser) {
	_created = utime();
}

State::State(State* state)
	: _browser(state->_browser) {
	_created = utime();
	delete state;
}

State::~State(){

}

slide_context_t State::next_slide() const {
	return _browser->next_slide(_browser);
}

browser_module_t* State::browser() const {
	return _browser;
}

float State::age() const {
	return static_cast<float>(utime() - _created) / 1e6;
}
