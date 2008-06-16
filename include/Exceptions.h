#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>

class NoXConnection: public std::runtime_error {
	public:
		NoXConnection(const char* str): std::runtime_error(str){}
		virtual ~NoXConnection() throw(){}
};

#endif // EXCEPTIONS_H
