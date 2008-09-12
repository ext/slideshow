#include "TransitionState.h"
#include "ViewState.h"

double TransitionState::transition_time = 1.0;

State* TransitionState::action(){
	double s = age() / transition_time;

	gfx()->render( s );

	if ( s > 1.0f ){
		return new ViewState(this);
	}

	return this;
}
