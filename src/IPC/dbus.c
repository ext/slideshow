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

#include "IPC.hpp"
#include "core/log.h"
#include <dbus/dbus.h>

MODULE_INFO("dbus", IPC_MODULE, "David Sveningsson");

static const char* dbus_rule = "type='signal',interface='com.slideshow.dbus.Signal'";
static DBusConnection* bus;
static DBusError error;

static void poll(struct ipc_module_t* module, int timeout){
	dbus_connection_read_write_dispatch(bus, timeout);
}

static void handle_quit(DBusMessage* message){
	action_quit();
}

static void handle_reload(DBusMessage* message){
	action_reload();
}

static void handle_debug(DBusMessage* message){
	action_debug();
}

static void handle_set_queue(DBusMessage* message){
	unsigned int queue_id;
	dbus_bool_t args_ok = dbus_message_get_args (message, &error,
	                                             DBUS_TYPE_UINT32, &queue_id,
	                                             DBUS_TYPE_INVALID);
	if ( !args_ok ) {
		log_message(Log_Verbose, "D-Bus: Malformed `ChangeQueue' command: %s\n", error.message);
		return;
	}

	action_set_queue((int)queue_id);
}

static DBusHandlerResult signal_filter (DBusConnection* bus, DBusMessage* message, void* user_data){
	if (dbus_message_is_signal(message, DBUS_INTERFACE_LOCAL, "Disconnected")) {
		log_message(Log_Warning, "D-Bus: Bus got disconnected.\n");
		return DBUS_HANDLER_RESULT_HANDLED;
	}

	typedef void (*callback_t)(DBusMessage* message);
	struct command_t {
		callback_t callback;
		const char* name;
	};

	static struct command_t commands[] = {
		{ &handle_quit, "Quit" },
		{ &handle_reload, "Reload" },
		{ &handle_debug, "Debug_DumpQueue" },
		{ &handle_set_queue, "ChangeQueue" },
		{ NULL, NULL }
	};

	struct command_t* current_command = commands;
	while ( current_command->name != NULL ){
		if ( dbus_message_is_signal (message, "com.slideshow.dbus.Signal", current_command->name) ) {
			const callback_t func = current_command->callback;
			func(message);
			return DBUS_HANDLER_RESULT_HANDLED;
		}

		current_command++;
	}

	log_message(Log_Verbose, "D-Bus: Unhandled command: %s\n", dbus_message_get_member(message));
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void* module_alloc(){
	return malloc(sizeof(struct ipc_module_t));
}

int module_init(struct ipc_module_t* module){
	log_message(Log_Verbose, "D-Bus: Starting\n");
	module->poll = poll;

	dbus_error_init (&error);
	bus = dbus_bus_get (DBUS_BUS_SYSTEM, &error);

	if ( !bus ) {
		log_message(Log_Fatal, "D-Bus: %s\n", error.message);
		return -1;
	}

	dbus_bus_add_match (bus, dbus_rule, &error);
	dbus_connection_add_filter(bus, signal_filter, NULL, NULL);
	return 0;
}

int module_cleanup(struct ipc_module_t* module){
	dbus_bus_remove_match(bus, dbus_rule, &error);
	dbus_connection_remove_filter(bus, signal_filter, NULL);
	dbus_error_free(&error);
	return 0;
}
