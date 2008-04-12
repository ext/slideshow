/**
 * This file is part of Slideshow.
 * 
 * Slideshow is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Slideshow is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Slideshow.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SLIDESHOW_DBUS_IPC_H
#define SLIDESHOW_DBUS_IPC_H

#include "IPC.h"
#include <dbus/dbus.h>

class DBus: public IPC {
	public:
		DBus(Kernel* kernel, int timeout_ms);
		virtual ~DBus();
		
		virtual void poll();
		
	private:
		static DBusHandlerResult signal_filter (DBusConnection* bus, DBusMessage* message, void* user_data);
		static void handle_quit(DBusMessage* message, Kernel* kernel);
		static void handle_reload(DBusMessage* message, Kernel* kernel);
		static void handle_ping(DBusMessage* message, Kernel* kernel);
		static void handle_playvideo(DBusMessage* message, Kernel* kernel);
		static void handle_debug_dumpqueue(DBusMessage* message, Kernel* kernel);
		
		DBusConnection* _bus;
		DBusError _error;
		const int _timeout;
};

#endif // SLIDESHOW_DBUS_IPC_H
