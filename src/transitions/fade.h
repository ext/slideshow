#ifndef FADE_TRANSITION_H
#define FADE_TRANSITION_H

#include "Transition.h"

class FadeTransition: public Transition {
	public:
		FadeTransition();
		virtual ~FadeTransition();
		
		virtual void render(unsigned int texture_0, unsigned int texture_1, float state);
};

#endif // FADE_TRANSITION_H
