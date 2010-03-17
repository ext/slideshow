#include "InitialState.h"
#include "TransitionState.h"

State* InitialState::action(bool &flip){
	gfx()->load_image(NULL);
	gfx()->load_image("resources/splash.png");
	gfx()->render(0.0);
	flip = true;

	return new TransitionState(this);
}
