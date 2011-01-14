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

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "Log.h"
#include "ptr.h"
#include "exception.h"
#include <stdarg.h>
#include <cstdlib>
#include <cstring>
#include <memory> /* for auto_ptr */
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <portable/asprintf.h>
#include <portable/file.h>

#ifdef WIN32
#	include "win32.h"
#endif

#ifdef HAVE_SYSLOG
#	include <syslog.h>
int syslog_severity[5] = {
	LOG_DEBUG,
	LOG_INFO,
	LOG_NOTICE,
	LOG_WARNING,
	LOG_ERR
};
#endif /* HAVE_SYSLOG */

Log::vector Log::_dst;

FileDestination::FileDestination(const char* filename)
	: _fp(NULL)
	, _autoclose(true) {

	if ( fopen_s(&_fp, filename, "a") != 0 ){
		fprintf(stderr, "Failed to open logfile '%s' ! Fatal error!\n", filename);
		exit(1);
	}
}
FileDestination::FileDestination(FILE* fp)
	: _fp(fp)
	, _autoclose(false) {

	if ( !fp ){
		fprintf(stderr, "Failed to read fp! Fatal error!\n");
		exit(1);
	}
}

FileDestination::~FileDestination(){
	if ( _autoclose ){
		fclose(_fp);
	}
}
void FileDestination::write(const char* content, const char* decorated) const {
	fputs(decorated, _fp);
	fflush(_fp);
}

FIFODestination::FIFODestination(const char* filename)
	: _filename(NULL)
	, _fp(NULL) {

	_filename = strdup(filename);
	mkfifo(filename, 0600);

	if ( fopen_s(&_fp, filename, "w") != 0 ){
		fprintf(stderr, "Failed to open logfile '%s' ! Fatal error!\n", filename);
		exit(1);
	}
}

FIFODestination::~FIFODestination(){
	fclose(_fp);
	unlink(_filename);
	free(_filename);
}
void FIFODestination::write(const char* content, const char* decorated) const {
	fputs(decorated, _fp);
	fflush(_fp);
}

SocketDestination::SocketDestination(int socket)
	: _socket(socket) {

}

SocketDestination::~SocketDestination(){
	close(_socket);
}

void SocketDestination::write(const char* content, const char* decorated) const {
	send(_socket, decorated, strlen(decorated), 0);
}

#ifdef HAVE_SYSLOG
SyslogDestination::SyslogDestination(){
	/* @todo only a single instance of syslog may be opened */
	openlog(PACKAGE, 0, LOG_DAEMON);
}
SyslogDestination::~SyslogDestination(){
	closelog();
}
void SyslogDestination::write(const char* content, const char* decorated) const {
	syslog(syslog_severity[severity], content);
}
#endif /* HAVE_SYSLOG */

UDSServer::UDSServer(const char* filename)
	: _filename(NULL)
	, _socket(-1) {

	_filename = strdup(filename);

	struct sockaddr_un address;
	socklen_t address_length;

	_socket = socket(PF_UNIX, SOCK_STREAM, 0);

	unlink(filename);
	address.sun_family = AF_UNIX;
	address_length = (socklen_t)sizeof(address.sun_family) +
		(socklen_t)sprintf(address.sun_path, "%s", filename);

	bind(_socket, (struct sockaddr *) &address, address_length);
	listen(_socket, 5);
}

UDSServer::~UDSServer(){
	unlink(_filename);
	free(_filename);
}

bool UDSServer::accept(struct timeval *timeout) const {
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(_socket, &rfds);

	if ( select(_socket+1, &rfds, NULL, NULL, timeout) <= 0 ){
		return false;
	}

	int client = ::accept(_socket, NULL, NULL);
	Log::add_destination(new SocketDestination(client));

	return true;
}

void Log::initialize(){

}

void Log::cleanup(){
	for ( iterator it = _dst.begin(); it != _dst.end(); ++it ){
		delete *it;
	}
	_dst.clear();
}

void Log::add_destination(Destination* dst){
	_dst.push_back(dst);
}

void Log::message(Severity severity, const char* fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	vmessage(severity, fmt, ap);
	va_end(ap);
}

void Log::vmessage(Severity severity, const char* fmt, va_list ap){
	static char buf[255]; /* this isn't thread-safe anyway, might as well make it static */

	Ptr<char> content( vasprintf2(fmt, ap) );
	Ptr<char> decorated( asprintf2("(%s) [%s] %s", severity_string(severity), timestring(buf, 255), content.get()) );

	for ( iterator it = _dst.begin(); it != _dst.end(); ++it ){
		Destination* dst = *it;
		dst->write(content.get(), decorated.get());
	}
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
		case Log_Debug: return "DD";
		case Log_Verbose: return "--";
		case Log_Info: return "  ";
		case Log_Warning: return "WW";
		case Log_Fatal: return "!!";
	}
	return NULL;
}
