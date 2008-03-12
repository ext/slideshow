#ifndef KERNEL_H
#define KERNEL_H

class Graphics;
class Browser;
class IPC;

class Kernel {
	public:
		Kernel(int argc, char* argv[]);
		~Kernel();
		
		void run();
		void parse_argv(int argc, char* argv[]);
		
		void quit();
		void reload_browser();
		void ipc_quit();
		void play_video(const char* fullpath);
		
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
		
		char* get_file_ext(const char* filename);
		
		///@todo fulhack
		bool is_movie_ext(const char* ext);
		
		int _width;
		int _height;
		int _frames;
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
