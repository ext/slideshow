/**
 * This file is part of Slideshow.
 * Copyright (C) 2008 David Sveningsson <ext@sidvind.com>
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

#include "dbus.h"
#include "Log.h"
#include "Kernel.h"
#include <stdexcept>

static const char* dbus_rule = "type='signal',interface='com.slideshow.dbus.Signal'";

DBus::DBus(Kernel* kernel, int timeout):
	IPC(kernel),
	_timeout(timeout){

	Log::message(Log::Verbose, "D-Bus: Starting\n");

	dbus_error_init (&_error);
	_bus = dbus_bus_get (DBUS_BUS_SYSTEM, &_error);

	if (!_bus) {
		Log::message(Log::Fatal, "D-Bus: Failed to connect to the daemon: %s", _error.message);
		throw std::runtime_error("Failed to start D-Bus");
	}

	dbus_bus_add_match (_bus, dbus_rule, &_error);
	dbus_connection_add_filter (_bus, signal_filter, kernel, NULL);
}

DBus::~DBus(){
	dbus_bus_remove_match (_bus, dbus_rule, &_error);
	dbus_connection_remove_filter (_bus, signal_filter, kernel());
	dbus_error_free (&_error);
}

void DBus::poll(){
	dbus_connection_read_write_dispatch ( _bus, _timeout );
}

DBusHandlerResult DBus::signal_filter (DBusConnection* bus, DBusMessage* message, void* user_data){
	Kernel* kernel = static_cast<Kernel*>(user_data);

	if (dbus_message_is_signal(message, DBUS_INTERFACE_LOCAL, "Disconnected")) {
		Log::message(Log::Verbose, "D-Bus: Disconnected\n");
		kernel->ipc_quit();
		return DBUS_HANDLER_RESULT_HANDLED;
	}

	typedef void (*callback_t)(DBusMessage* message, Kernel* kernel);

	struct command_t {
		callback_t callback;
		const char* name;
	};

	static command_t commands[] = {
		{ handle_quit, "Quit" },
		{ handle_reload, "Reload" },
		{ handle_ping, "Ping" },
		{ handle_playvideo, "PlayVideo" },
		{ handle_debug_dumpqueue, "Debug_DumpQueue" },
		{ handle_change_bin, "ChangeBin" },
		{ NULL, NULL }
	};

	command_t* current_command = commands;
	while ( current_command->name != NULL ){
		if ( dbus_message_is_signal (message, "com.slideshow.dbus.Signal", current_command->name) ) {
			current_command->callback(message, kernel);
			return DBUS_HANDLER_RESULT_HANDLED;
		}

		current_command++;
	}

	Log::message(Log::Verbose, "D-Bus: Unhandled command: %s\n", dbus_message_get_member(message));

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void DBus::handle_quit(DBusMessage* message, Kernel* kernel){
	Log::message(Log::Verbose, "IPC: Quit\n");
	kernel->quit();
}

void DBus::handle_reload(DBusMessage* message, Kernel* kernel){
	Log::message(Log::Verbose, "IPC: Reload browser\n");
	kernel->reload_browser();
}

void DBus::handle_playvideo(DBusMessage* message, Kernel* kernel){
	DBusError error;
	dbus_error_init (&error);

	char* fullpath;

	dbus_bool_t args_ok = dbus_message_get_args (message, &error,
		DBUS_TYPE_STRING, &fullpath,
		DBUS_TYPE_INVALID
	);

	if ( !args_ok ) {
		Log::message(Log::Verbose, "D-Bus: Malformed PlayVideo command: %s\n", error.message);
		return;
	}

	kernel->play_video(fullpath);
}

void DBus::handle_ping(DBusMessage* message, Kernel* kernel){
	Log::message(Log::Verbose, "IPC: Ping (reloading)\n");
	kernel->reload_browser();
}

void DBus::handle_debug_dumpqueue(DBusMessage* message, Kernel* kernel){
	Log::message(Log::Verbose, "IPC: Debug DumpQueue\n");
	kernel->debug_dumpqueue();
}

void DBus::handle_change_bin(DBusMessage* message, Kernel* kernel){
	DBusError error;
	dbus_error_init (&error);

	unsigned int bin_id;

	dbus_bool_t args_ok = dbus_message_get_args (message, &error,
		DBUS_TYPE_UINT32, &bin_id,
		DBUS_TYPE_INVALID
	);

	if ( !args_ok ) {
		Log::message(Log::Verbose, "D-Bus: Malformed ChangeBin command: %s\n", error.message);
		return;
	}

	Log::message(Log::Verbose, "IPC: Changing bin to %d\n", bin_id);
	kernel->change_bin(bin_id);
}
