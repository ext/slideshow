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

#ifndef LOG_H
#define LOG_H

#include "log_base.h"
#include <cstdio>
#include <vector>

class Destination {
	public:
		virtual ~Destination(){}

		/**
		 * Write to destination
		 * @param content Original content.
		 * @param decorated Decorated content (timestamp etc)
		 */
		virtual void write(const char* content, const char* decorated) const = 0;
};

class FileDestination: public Destination {
	public:
		/**
		 * Open by filename.
		 */
		FileDestination(const char* filename);

		/**
		 * Open by file stream, caller must call fclose if needed.
		 */
		FileDestination(FILE* fp);

		virtual ~FileDestination();
		virtual void write(const char* content, const char* decorated) const;

	private:
		FILE* _fp;
		bool _autoclose;
};

class FIFODestination: public Destination {
	public:
		/**
		 * Named pipe
		 */
		FIFODestination(const char* filename);

		virtual ~FIFODestination();
		virtual void write(const char* content, const char* decorated) const;

	private:
		char* _filename;
		FILE* _fp;
};

class SocketDestination: public Destination {
	public:
		SocketDestination(int socket);
		virtual ~SocketDestination();

		virtual void write(const char* content, const char* decorated) const;

	private:
		int _socket;
};

/**
 * Unix Doman Sockets Server
 */
class UDSServer {
	public:
		UDSServer(const char* filename);
		~UDSServer();

		bool accept(struct timeval *timeout) const;

	private:
		char* _filename;
		int _socket;
};

#ifdef HAVE_SYSLOG
class SyslogDestination: public Destination {
	public:
		SyslogDestination();
		virtual ~SyslogDestination()
		virtual void write(const char* content, const char* decorated) const;
};
#endif /* HAVE_SYSLOG */

namespace Log {
	
	void initialize();
	void cleanup();
	
	/**
	 * Add a logging destination.
	 * @param dst Memory will be release using delete.
	 */
	void add_destination(Destination* dst);
	
	void  message(Severity severity, const char* fmt, ...) __attribute__ ((format (printf, 2, 3)));
	void vmessage(Severity severity, const char* fmt, va_list ap);
	
	void debug(const char* fmt, ...)   __attribute__ ((format (printf, 1, 2)));  /** Short for message(Log_Debug) */
	void verbose(const char* fmt, ...) __attribute__ ((format (printf, 1, 2)));  /** Short for message(Log_Verbose) */
	void info(const char* fmt, ...)    __attribute__ ((format (printf, 1, 2)));  /** Short for message(Log_Info) */
	void warning(const char* fmt, ...) __attribute__ ((format (printf, 1, 2)));  /** Short for message(Log_Warning) */
	void fatal(const char* fmt, ...)   __attribute__ ((format (printf, 1, 2)));  /** Short for message(Log_Fatal) */
}

#endif // LOG_H
