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

#ifndef SLIDESHOW_IPC_H
#define SLIDESHOW_IPC_H

#include "module_loader.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Quit application.
 */
void action_quit();

/**
 * Reload configuration.
 */
void action_reload();

/**
 * Output debugging information to log.
 */
void action_debug();

/**
 * Set active queue.
 */
void action_set_queue(int id);

struct ipc_module_t {
	struct module_t module;

	/**
	 * Poll IPC implementation. If non-null it will be called periodically by
	 * kernel.
	 *
	 * @param timeout Time in ms it can block, 0 means no blocking.
	 */
	void (*poll)(struct ipc_module_t* self, int timeout);
};

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <string>
namespace IPC {
	ipc_module_t* factory(const std::string& name);
}
#endif

#endif // SLIDESHOW_IPC_H
