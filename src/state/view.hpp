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

#ifndef STATE_VIEW_H
#define STATE_VIEW_H

#include "state/state.hpp"

class ViewState: public State {
public:
	ViewState(State* state): State(state){}
	virtual ~ViewState(){}

	virtual State* action(bool &flip);

	static void set_view_time(double t){ view_time = t; }

private:
	static double view_time;
};

#endif /* STATE_VIEW_HPP */
