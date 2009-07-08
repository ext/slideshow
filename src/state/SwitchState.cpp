#include "SwitchState.h"
#include "TransitionState.h"
#include "ViewState.h"
#include "Log.h"

State* SwitchState::action(){
	if ( !browser() ){
		return new ViewState(this);
	}

	const char* filename = browser()->get_next_file();

	if ( !filename ){
		Log::message(Log::Warning, "Kernel: Queue is empty\n", filename);
	} else {
		Log::message(Log::Debug, "Kernel: Switching to image \"%s\"\n", filename);
	}

	try {
		gfx()->load_image( filename );
	} catch ( ... ) {
		Log::message(Log::Warning, "Kernel: Failed to load image '%s'\n", filename);
		return new ViewState(this);
	}

	return new TransitionState(this);
}
