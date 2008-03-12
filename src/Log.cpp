#include "Log.h"
#include <stdarg.h>
#include <cstdlib>
#include <time.h>

FILE* Log::_file = NULL;
FILE* Log::_dfile = NULL;

void Log::initialize(const char* filename, const char* debugfilename){
	_file = fopen(filename, "a");
	_dfile = fopen(debugfilename, "a");
	
	if ( !_file ){
		fprintf(stderr, "Failed to open logfile '%s' ! Fatal error!\n", filename);
		exit(1);
	}
	
	if ( !_dfile ){
		fprintf(stderr, "Failed to open logfile '%s' ! Fatal error!\n", debugfilename);
		exit(1);
	}
}

void Log::deinitialize(){
	fclose(_file);
	fclose(_dfile);
}

void Log::message(Severity severity, const char* fmt, ...){
#ifdef _NDEBUG
	if ( severity == Debug ){
		return;
	}
#endif
	
	va_list arg;
	va_start(arg, fmt);
	
	char buf[255];
	
	char* line;
	vasprintf(&line, fmt, arg);

	if ( severity != Debug ){
		fprintf(_file, "(%s) [%s] %s", severity_string(severity), timestring(buf, 255), line);
		fflush(_file);
	}
	
	fprintf(_dfile, "(%s) [%s] %s", severity_string(severity), timestring(buf, 255), line);
	fflush(_dfile);
	
	if ( severity == Fatal ){
		vfprintf(stderr, fmt, arg);
	}
	
	free(line);
	va_end(arg);
}

char* Log::timestring(char *buffer, int bufferlen) {
    time_t t = time(NULL);
    struct tm* nt;
#ifdef WIN32
    nt = new struct tm;
    localtime_s(nt, &t);
#else
    nt = localtime(&t);
#endif
    strftime(buffer, bufferlen, "%Y-%m-%d %H:%M:%S", nt);
#ifdef WIN32
    delete nt;
#endif   
    return buffer;
}

const char* Log::severity_string(Severity severity){
	switch ( severity ){
		case Debug: return "DD";
		case Verbose: return "--";
		case Warning: return "WW";
		case Fatal: return "!!";
	}
	return NULL;
}
