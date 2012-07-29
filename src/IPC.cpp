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
#include "Kernel.h"
#include "log.hpp"

IPC::IPC(Kernel* kernel)
	: kernel(kernel) {

}

IPC::~IPC(){

}

void IPC::action_quit(){
	Log::message(Log_Verbose, "IPC: Quit\n");
	kernel->quit();
}

void IPC::action_reload(){
	Log::message(Log_Verbose, "IPC: Reload browser\n");
	kernel->reload_browser();
}

void IPC::action_debug(){
	Log::message(Log_Verbose, "IPC: Debug\n");
	kernel->debug_dumpqueue();
}

void IPC::action_set_queue(int id){
	Log::message(Log_Verbose, "IPC: Changing queue to %d\n", id);
	kernel->queue_set(id);
}
