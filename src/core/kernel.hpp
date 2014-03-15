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

#ifndef KERNEL_H
#define KERNEL_H

class State;
class PlatformBackend;
class UDSServer;

#include "browsers/browser.h"
#include <vector>

class Kernel {
	public:
		typedef struct argument_set_t {
			int mode;
			int loglevel;
			int fullscreen;
			int have_password;
			int queue_id;
			int width;
			int height;
			float transition_time;
			float switch_time;
			char* connection_string;
			char* transition_string;
			char* log_file;     /* log: file */
			char* log_fifo;     /* log: named pipe */
			char* log_domain;   /* log: unix domain socket */

			/* frontend settings */
			char* url;
			char* instance;
		} argument_set_t;

		enum Mode {
			InvalidMode,
			ForegroundMode,
			DaemonMode,
			HelpMode,
			ListTransitionMode
		};

		static void print_transitions();

		Kernel(const argument_set_t& arg, PlatformBackend* backend);
		virtual ~Kernel();

		virtual void init();
		virtual void cleanup();
		virtual void run();
		virtual void poll();
		virtual void action();

		bool running(){ return _running; }

		void start();
		void quit();

		void reload_browser();
		void play_video(const char* fullpath);
		void queue_set(unsigned int id);

		void debug_dumpqueue();

		static bool parse_arguments(argument_set_t& arg, int argc, const char* argv[]);

	protected:
		static const char* pidpath();

		void print_config() const;
		void print_licence_statement() const;

	private:

		void create_pidpath();

		void view_state(double t);
		void transition_state(double t);
		void switch_state(double t);

		browser_module_t* browser(){ return _browser; }

		void load_transition(const char* name);

		char* get_password();

		void init_backend();
		void cleanup_backend();
		void init_graphics();
		void init_IPC();
		void cleanup_IPC();
		void init_browser();
		void init_fsm();



		argument_set_t _arg;

		char* _password;

		State* _state;

		browser_module_t* _browser;
		PlatformBackend* _backend;
		std::vector<struct ipc_module_t*> _ipc;

		bool _running;
};

#endif // KERNEL_H
