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
