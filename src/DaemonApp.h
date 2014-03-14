/**
 * This file is part of Slideshow.
 * Copyright (C) 2008-2010 David Sveningsson <ext@sidvind.com>
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

#ifndef DAEMON_APP_H
#define DAEMON_APP_H

#include "kernel.hpp"
#include "exception.h"
#include <sys/select.h>

class DaemonApp: public Kernel {
	public:
		DaemonApp(const argument_set_t& arg, PlatformBackend* backend);
		~DaemonApp();

		virtual void init();
		virtual void cleanup();

		void daemon_start();
		void daemon_ready();
		void daemon_poll();
		void daemon_stop();

		void pass_exception(const exception &e);
		void pass_exception(const ExitException &e);

	private:
		int fd;
		fd_set fds;

		int _readfd;
		int _writefd;
};

#endif // DAEMON_APP_H
