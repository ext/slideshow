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
#include "kernel.hpp"
#include "log.hpp"

extern Kernel* global_fubar_kernel;

ipc_module_t* IPC::factory(const std::string& name){
	ipc_module_t* module = (ipc_module_t*)module_open(name.c_str(), IPC_MODULE, 0);
	if ( !module ) Log::warning("Failed to load IPC module `%s'.\n", name.c_str());
	return module;
}

void action_quit(){
	Log::message(Log_Verbose, "IPC: Quit\n");
	global_fubar_kernel->quit();
}

void action_reload(){
	Log::message(Log_Verbose, "IPC: Reload browser\n");
	global_fubar_kernel->reload_browser();
}

void action_debug(){
	Log::message(Log_Verbose, "IPC: Debug\n");
	global_fubar_kernel->debug_dumpqueue();
}

void action_set_queue(int id){
	Log::message(Log_Verbose, "IPC: Changing queue to %d\n", id);
	global_fubar_kernel->queue_set(id);
}
