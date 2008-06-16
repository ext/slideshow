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

#ifndef KERNEL_H
#define KERNEL_H

class Graphics;
class Browser;
class IPC;

class Kernel {
	public:
		Kernel(int argc, const char* argv[]);
		~Kernel();

		void run();
		void parse_argv(int argc, const char* argv[]);

		void quit();
		void reload_browser();
		void ipc_quit();
		void play_video(const char* fullpath);
		void change_bin(unsigned int id);

		void debug_dumpqueue();

	private:
		enum State {
			VIEW,
			TRANSITION,
			SWITCH
		};

		void view_state(double t);
		void transition_state(double t);
		void switch_state(double t);

		void print_licence_statement();

		int _width;
		int _height;
		int _frames;
		unsigned int _bin_id;
		bool _fullscreen;
		bool _daemon;
		double _transition_time;
		double _switch_time;
		double _last_switch;
		double _transition_start;
		State _state;

		Graphics* _graphics;
		Browser* _browser;
		IPC* _ipc;

		char* _db_username;
		char* _db_password;
		char* _db_name;

		const char* _logfile;
		bool _running;
};

#endif // KERNEL_H
