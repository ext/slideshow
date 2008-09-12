#include "InitialState.h"
#include "TransitionState.h"

State* InitialState::action(){
	gfx()->load_image(NULL);
	gfx()->load_image(NULL);
	gfx()->render(1.0);

	return new TransitionState(this);
}
