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

#include "IPC.hpp"
#include <signal.h>

MODULE_INFO("signal", IPC_MODULE, "David Sveningsson");

static void sighandler(int signum){
	switch ( signum ){
	case SIGINT:
	case SIGTERM:
		action_quit();
		break;
	case SIGHUP:
		action_reload();
		break;
	case SIGUSR1:
		action_debug();
	}
}

void* module_alloc(){
	return malloc(sizeof(struct ipc_module_t));
}

int module_init(struct ipc_module_t* module){
	module->poll = NULL;

	signal(SIGINT, sighandler);
	signal(SIGHUP, sighandler);
	signal(SIGTERM, sighandler);
	signal(SIGUSR1, sighandler);

	return 0;
}

int module_cleanup(struct ipc_module_t* module){
	signal(SIGINT, SIG_DFL);
	signal(SIGHUP, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGUSR1, SIG_DFL);

	return 0;
}
