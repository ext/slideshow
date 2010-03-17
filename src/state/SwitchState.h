#ifndef SWITCHSTATE_H
#define SWITCHSTATE_H

#include "State.h"

class SwitchState: public State {
	public:
		SwitchState(State* state): State(state){}
		virtual ~SwitchState(){}

		virtual State* action(bool &flip);
};

#endif // SWITCHSTATE_H
