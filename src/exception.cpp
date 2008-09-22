#include "Exceptions.h"
#include <cstdlib>

BaseException::BaseException(const char* message): std::exception(), _msg(NULL) {
	if ( message ){
		asprintf(&_msg, "%s", message);
	}
}

BaseException::BaseException(const BaseException& e): std::exception(), _msg(NULL) {
	if ( e._msg ){
		asprintf(&_msg, "%s", e._msg);
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

FatalException::FatalException(ErrorCode code, const char* message): BaseException(message), _code(code){

}

FatalException::FatalException(const FatalException& e): BaseException(e), _code(e._code) {
}

FatalException::~FatalException() throw() {

}

ErrorCode FatalException::code(){
	return _code;
}

ADD_EXCEPTION_IMPLEMENTATION(XlibException, XLIB_ERROR);
ADD_EXCEPTION_IMPLEMENTATION(KernelException, KERNEL_ERROR);
ADD_EXCEPTION_IMPLEMENTATION(ArgumentException, ARGUMENT_ERROR);
ADD_EXCEPTION_IMPLEMENTATION(BrowserException, BROWSER_ERROR);
ADD_EXCEPTION_IMPLEMENTATION(IPCException, IPC_ERROR);
ADD_EXCEPTION_IMPLEMENTATION(GraphicsException, GRAPHICS_ERROR);
ADD_EXCEPTION_IMPLEMENTATION(DaemonException, DAEMON_ERROR);
