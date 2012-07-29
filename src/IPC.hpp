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

class Kernel;

class IPC {
	public:
		IPC(Kernel* kernel): _kernel(kernel){}
		virtual ~IPC(){}

		/**
		 * Poll IPC implementation.
		 *
		 * @param timeout Time in ms it can block, 0 means no blocking.
		 */
		virtual void poll(int timeout) = 0;

	protected:
		Kernel* kernel(){ return _kernel; }

	private:
		Kernel* _kernel;
};

#endif // SLIDESHOW_IPC_H
