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

#ifndef TRANSITIONSTATE_H
#define TRANSITIONSTATE_H

#include "State.h"

class TransitionState: public State {
	public:
		TransitionState(State* state): State(state){}
		virtual ~TransitionState(){}

		virtual State* action(bool &flip);

		static void set_transition_time(double t){ transition_time = t; }

	private:
		static double transition_time;
};

#endif // TRANSITIONSTATE_H
