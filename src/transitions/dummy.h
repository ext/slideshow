#ifndef DUMMY_TRANSITION_H
#define DUMMY_TRANSITION_H

#include "Transition.h"

class DummyTransition: public Transition {
	public:
		DummyTransition();
		virtual ~DummyTransition();
		
		virtual void render(unsigned int texture_0, unsigned int texture_1, float state);
};

#endif // DUMMY_TRANSITION_H
