/**
 * This file is part of Slideshow.
 * Copyright (C) 2008-2012 David Sveningsson <ext@sidvind.com>
 *
 * Slideshow is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Slideshow is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Slideshow.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "signal.hpp"
#include "Kernel.h"
#include "log.hpp"
#include <signal.h>

static SignalIPC* ipc = NULL;

static void sighandler(int signum){
	switch ( signum ){
	case SIGINT:
	case SIGTERM:
		ipc->action_quit();
		break;
	case SIGHUP:
		ipc->action_reload();
		break;
	case SIGUSR1:
		ipc->action_debug();
	}
}

SignalIPC::SignalIPC(Kernel* kernel)
	: IPC(kernel) {
	ipc = this;

	signal(SIGINT, sighandler);
	signal(SIGHUP, sighandler);
	signal(SIGTERM, sighandler);
	signal(SIGUSR1, sighandler);
}

SignalIPC::~SignalIPC(){
	ipc = NULL;
}

void SignalIPC::poll(int timeout){
	/* do nothing */
}
