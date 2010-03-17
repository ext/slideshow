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

#ifndef KERNEL_H
#define KERNEL_H

class Graphics;
class Browser;
class IPC;
class State;

class Kernel {
	public:
		typedef struct argument_set_t {
			int mode;
			int loglevel;
			int fullscreen;
			int have_password;
			int collection_id;
			int width;
			int height;
			float transition_time;
			float switch_time;
			char* connection_string;
			char* transition_string;
		} argument_set_t;

		enum Mode {
			InvalidMode,
			ForegroundMode,
			DaemonMode,
			HelpMode,
			ListTransitionMode
		};

		static void print_transitions();

		Kernel(const argument_set_t& arg);
		virtual ~Kernel();

		virtual void init();
		virtual void cleanup();
		virtual void run() = 0;
		virtual void poll();
		virtual void action();

		bool running(){ return _running; }

		void start();
		void quit();

		void reload_browser();
		void ipc_quit();
		void play_video(const char* fullpath);
		void change_bin(unsigned int id);

		void debug_dumpqueue();

		static bool parse_arguments(argument_set_t& arg, int argc, const char* argv[]);

	protected:
		static const char* pidpath();

	private:

		void create_pidpath();

		void view_state(double t);
		void transition_state(double t);
		void switch_state(double t);

		void print_licence_statement();
		void print_cli_arguments(int argc, const char* argv[]);

		Browser* browser(){ return _browser; }

		void load_transition(const char* name);

		char* get_password();

		void init_graphics();
		void init_IPC();
		void init_browser();
		void init_fsm();

		argument_set_t _arg;

		char* _password;

		State* _state;

		Graphics* _graphics;
		Browser* _browser;
		IPC* _ipc;

		bool _running;
};

#endif // KERNEL_H
