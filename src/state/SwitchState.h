#ifndef SWITCHSTATE_H
#define SWITCHSTATE_H

#include "State.h"

class SwitchState: public State {
	public:
		SwitchState(State* state): State(state){}

		virtual State* action();
};

#endif // SWITCHSTATE_H
