#include "Exceptions.h"
#include <cstdlib>

BaseException::BaseException(const char* message): std::exception(), _msg(NULL) {
	if ( message ){
		asprintf(&_msg, message);
	}
}

BaseException::~BaseException() throw() {
	free(_msg);
}

const char* BaseException::what() const throw() {
	return _msg;
}

void BaseException::set_message(const char* fmt, va_list va){
	if ( _msg ){
		free(_msg);
	}

	vasprintf(&_msg, fmt, va);
}
