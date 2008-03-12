#ifndef LOG_H
#define LOG_H

#include <cstdio>

class Log {
	public:
		static void initialize(const char* filename, const char* debugfilename);
		static void deinitialize();
		
		enum Severity {
			Debug,
			Verbose,
			Warning,
			Fatal
		};
		
		static void message(Severity severity, const char* fmt, ...);
		
	private:
		Log(){}
		
		static char *timestring(char *buffer, int bufferlen);
		static const char* severity_string(Severity severity);
		
		static FILE* _file;
		static FILE* _dfile;
};

#endif // LOG_H
