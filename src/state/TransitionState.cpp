#include "TransitionState.h"
#include "ViewState.h"

double TransitionState::transition_time = 1.0;

State* TransitionState::action(bool &flip){
	double s = age() / transition_time;

	gfx()->render( s );
	flip = true;

	if ( s > 1.0f ){
		return new ViewState(this);
	}

	return this;
}
