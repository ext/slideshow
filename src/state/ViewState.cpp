#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "ViewState.h"
#include "SwitchState.h"

double ViewState::view_time = 1.0;

State* ViewState::action(bool &flip){
	if ( age() > view_time ){
		return new SwitchState(this);
	}

	if ( ipc() ){
		ipc()->poll();
	}

	// Sleep for a while
	wait( 0.1f );

	return this;
}
