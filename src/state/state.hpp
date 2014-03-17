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

class State {
public:
	State(browser_module_t* browser);
	State(State* state);
	virtual ~State();

	virtual State* action(bool &flip) = 0;

	slide_context_t next_slide() const;
	browser_module_t* browser() const;

	float age() const;

private:
	browser_module_t* _browser;
	unsigned long _created;
};

#endif /* STATE_STATE_HPP */
