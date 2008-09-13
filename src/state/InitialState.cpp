#include "InitialState.h"
#include "TransitionState.h"

State* InitialState::action(){
	gfx()->load_image(NULL);
	gfx()->load_image("resources/splash.png");
	gfx()->render(0.0);

	return new TransitionState(this);
}
