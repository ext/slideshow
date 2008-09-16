#ifndef INITIALSTATE_H
#define INITIALSTATE_H

#include "State.h"

class InitialState: public State {
	public:
		InitialState(Browser* browser, Graphics* gfx, IPC* ipc): State(browser, gfx, ipc){}
		virtual ~InitialState(){}

		virtual State* action();
};

#endif // INITIALSTATE_H
