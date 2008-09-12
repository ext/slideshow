#ifndef VIEWSTATE_H
#define VIEWSTATE_H

#include "State.h"

class ViewState: public State {
	public:
		ViewState(State* state): State(state){}

		virtual State* action();

		static void set_view_time(double t){ view_time = t; }

	private:
		static double view_time;
};

#endif // VIEWSTATE_H
