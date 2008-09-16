#ifndef TRANSITIONSTATE_H
#define TRANSITIONSTATE_H

#include "State.h"

class TransitionState: public State {
	public:
		TransitionState(State* state): State(state){}
		virtual ~TransitionState(){}

		virtual State* action();

		static void set_transition_time(double t){ transition_time = t; }

	private:
		static double transition_time;
};

#endif // TRANSITIONSTATE_H
