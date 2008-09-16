#ifndef STATE_H
#define STATE_H

#include "Browser.h"
#include "Graphics.h"
#include "IPC.h"
#include "portable/Time.h"

class State {
	public:
		State(Browser* browser, Graphics* gfx, IPC* ipc): _browser(browser), _gfx(gfx), _ipc(ipc){
			_created = getTime();
		}
		virtual ~State(){}

		State(State* state): _browser(state->_browser), _gfx(state->_gfx), _ipc(state->_ipc){
			_created = getTime();
			delete state;
		}

		virtual State* action() = 0;

		Browser* browser(){ return _browser; }
		Graphics* gfx(){ return _gfx; }
		IPC* ipc(){ return _ipc; }

		double age(){ return getTime() - _created; }

	private:
		Browser* _browser;
		Graphics* _gfx;
		IPC* _ipc;
		double _created;
};

#endif // STATE_H
