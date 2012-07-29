/**
 * This file is part of Slideshow.
 * Copyright (C) 2008 David Sveningsson <ext@sidvind.com>
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

#include "dbus.h"
#include "log.hpp"
#include "exception.h"
#include "Kernel.h"

static const char* dbus_rule = "type='signal',interface='com.slideshow.dbus.Signal'";

DBus::DBus(Kernel* kernel)
	: IPC(kernel) {

	Log::message(Log_Verbose, "D-Bus: Starting\n");

	dbus_error_init (&_error);
	_bus = dbus_bus_get (DBUS_BUS_SYSTEM, &_error);

	if (!_bus) {
		Log::message(Log_Fatal, "D-Bus: %s\n", _error.message);
		throw exception("D-Bus: %s", _error.message);
	}

	dbus_bus_add_match (_bus, dbus_rule, &_error);
	dbus_connection_add_filter(_bus, signal_filter, this, NULL);
}

DBus::~DBus(){
	dbus_bus_remove_match (_bus, dbus_rule, &_error);
	dbus_connection_remove_filter (_bus, signal_filter, this);
	dbus_error_free (&_error);
}

void DBus::poll(int timeout){
	dbus_connection_read_write_dispatch(_bus, timeout);
}

DBusHandlerResult DBus::signal_filter (DBusConnection* bus, DBusMessage* message, void* user_data){
	DBus* dbus = static_cast<DBus*>(user_data);

	if (dbus_message_is_signal(message, DBUS_INTERFACE_LOCAL, "Disconnected")) {
		Log::message(Log_Verbose, "D-Bus: Disconnected\n");
		return DBUS_HANDLER_RESULT_HANDLED;
	}

	typedef void (DBus::*callback_t)(DBusMessage* message);

	struct command_t {
		callback_t callback;
		const char* name;
	};

	static command_t commands[] = {
		{ &DBus::handle_quit, "Quit" },
		{ &DBus::handle_reload, "Reload" },
		{ &DBus::handle_debug, "Debug_DumpQueue" },
		{ &DBus::handle_set_queue, "ChangeQueue" },
		{ NULL, NULL }
	};

	command_t* current_command = commands;
	while ( current_command->name != NULL ){
		if ( dbus_message_is_signal (message, "com.slideshow.dbus.Signal", current_command->name) ) {
			const callback_t func = current_command->callback;
			(dbus->*func)(message);
			return DBUS_HANDLER_RESULT_HANDLED;
		}

		current_command++;
	}

	Log::message(Log_Verbose, "D-Bus: Unhandled command: %s\n", dbus_message_get_member(message));

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void DBus::handle_quit(DBusMessage* message){
	action_quit();
}

void DBus::handle_reload(DBusMessage* message){
	action_reload();
}

void DBus::handle_debug(DBusMessage* message){
	action_debug();
}

void DBus::handle_set_queue(DBusMessage* message){
	DBusError error;
	dbus_error_init (&error);

	unsigned int queue_id;

	dbus_bool_t args_ok = dbus_message_get_args (message, &error,
		DBUS_TYPE_UINT32, &queue_id,
		DBUS_TYPE_INVALID
	);

	if ( !args_ok ) {
		Log::message(Log_Verbose, "D-Bus: Malformed ChangeBin command: %s\n", error.message);
		return;
	}

	action_set_queue(queue_id);
}
