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

#include "config.h"
#include "DaemonApp.h"
#include "Log.h"
#include <libdaemon/daemon.h>
#include <cstring>
#include <cstdlib>
#include <errno.h>
#include <csignal>

DaemonApp::DaemonApp(const argument_set_t& arg, PlatformBackend* backend):
	Kernel(arg, backend){

}

DaemonApp::~DaemonApp(){
}

void DaemonApp::init(){
	daemon_start();

	try {
		Kernel::init();
	} catch ( ExitException& e ){
		Log::message(Log_Fatal, "Error 0x%02x: %s\n", e.code());

		pass_exception(e);
		daemon_retval_send(e.code());

		daemon_stop();
		exit(0);
	} catch ( exception& e ){
		Log::message(Log_Fatal, "Unhandled exception: %s\n", e.what());

		pass_exception(e);
		daemon_retval_send(DAEMON_UNHANDLED_EXCEPTION);

		daemon_stop();
		exit(0);
	}

	daemon_ready();
}

void DaemonApp::cleanup(){
	daemon_stop();
	Kernel::cleanup();
}

void DaemonApp::pass_exception(const exception &e){
	size_t size = 0;

	verify( write(_writefd, &size, sizeof(size)) >= 0 );
}

void DaemonApp::pass_exception(const ExitException &e){
	size_t size = strlen(e.what());

	// @todo Errors should be handled properly
	verify( write(_writefd, &size, sizeof(size)) >= 0 );
	verify( write(_writefd, e.what(), size) >= 0 );
}

void DaemonApp::run(){
	start();

	while ( running() ){
		poll();
		action();
	}
}

void DaemonApp::daemon_start(){
    pid_t pid;

    /* Reset signal handlers */
    if (daemon_reset_sigs(-1) < 0) {
    	throw exception("Kernel: failed to reset all signal handlers: %s\n", strerror(errno));
    }

    /* Unblock signals */
    if (daemon_unblock_sigs(-1) < 0) {
    	throw exception("Kernel: failed to unblock all signals: %s\n", strerror(errno));
    }

    /* Set indetification string for the daemon for both syslog and PID file */
    daemon_pid_file_proc = &Kernel::pidpath;
	daemon_log_ident = PACKAGE_NAME;

    /* Check that the daemon is not rung twice a the same time */
	if ((pid = daemon_pid_file_is_running()) >= 0) {
		throw exception("Kernel: daemon already running on PID file %u\n", pid);
	}

	/* Prepare for return value passing from the initialization procedure of the daemon process */
	daemon_retval_init();

	/* Create a pipe to write messages from child to parent */
	{
		int pipefd[2];
		if ( pipe(pipefd) == -1 ){
			throw exception("Kernel: failed to create pipe: %s\n", strerror(errno));
		}
		_readfd = pipefd[0];
		_writefd = pipefd[1];
	}

	/* Do the fork */
	if ((pid = daemon_fork()) < 0) {

		/* Exit on error */
		daemon_retval_done();
		throw exception("Kernel: fork failed.\n");

	} else if (pid) { /* The parent */
		close(_writefd);

		int ret;

		/* Wait for 20 seconds for the return value passed from the daemon process */
		if ((ret = daemon_retval_wait(20)) < 0) {
			throw exception("Kernel: could not receive return value from daemon process: %s\n", strerror(errno));
		}

		if ( ret != 0 ){
			ssize_t size;

			// @todo Errors should be handled properly
			verify( read(_readfd, &size, sizeof(size_t)) == sizeof(size_t) );

			if ( size > 0 ){
				char buf[size+1];
				// @todo Errors should be handled properly
				verify( read(_readfd, buf, size) == size );
				close(_readfd);
				buf[size] = '\0';
				printf("DaemonApp exception: %s\n", buf);
			}

			throw ExitException((ErrorCode)ret);
		} else {
			close(_readfd);
			throw ExitException();
		}

	} else { /* The daemon */
		/* Close FDs */
		if (daemon_close_all(_writefd, -1) < 0) {
			Log::message(Log_Fatal, "Failed to close all file descriptors: %s\n", strerror(errno));

			/* Send the error condition to the parent process */
			daemon_retval_send(DAEMON_FILE_ERROR);
			daemon_stop();
			exit(0);
		}

		/* Create the PID file */
		if (daemon_pid_file_create() < 0) {
			Log::message(Log_Fatal, "Could not create PID file (%s).\n", strerror(errno));
			daemon_retval_send(DAEMON_PID_ERROR);
			daemon_stop();
			exit(0);
		}

		/* Initialize signal handling */
		if (daemon_signal_init(SIGINT, SIGTERM, SIGQUIT, SIGHUP, 0) < 0) {
			Log::message(Log_Fatal, "Could not register signal handlers (%s).\n", strerror(errno));
			daemon_retval_send(DAEMON_SIGNAL_ERROR);
			daemon_stop();
			exit(0);
		}
	}
}

void DaemonApp::daemon_ready(){
	/* Send OK to parent process */
	daemon_retval_send(0);

	Log::message(Log_Info, "Sucessfully started\n");
	daemon_log(LOG_INFO, "Sucessfully started\n");

	/* Prepare for select() on the signal fd */
	FD_ZERO(&fds);
	fd = daemon_signal_fd();
	FD_SET(fd, &fds);
}

void DaemonApp::daemon_poll(){
	fd_set fds2 = fds;

	/* Wait for an incoming signal */
	if (select(FD_SETSIZE, &fds2, 0, 0, 0) < 0) {

		/* If we've been interrupted by an incoming signal, continue */
		if (errno == EINTR)
			return;

		Log::message(Log_Warning, "select(): %s\n", strerror(errno));
		return;
	}

	/* Check if a signal has been recieved */
	if (FD_ISSET(fd, &fds)) {
		int sig;

		/* Get signal */
		if ((sig = daemon_signal_next()) <= 0) {
			Log::message(Log_Warning, "daemon_signal_next() failed: %s\n", strerror(errno));
			return;
		}

		/* Dispatch signal */
		switch (sig) {

			case SIGINT:
			case SIGQUIT:
			case SIGTERM:
				Log::message(Log_Info, "Got SIGINT, SIGQUIT or SIGTERM\n");
				quit();
				break;
		}
	}
}

void DaemonApp::daemon_stop(){
	Log::message(Log_Info, "Exiting\n");
	daemon_log(LOG_INFO, "Exiting");

	close(_writefd);

	daemon_retval_send(255);
	daemon_signal_done();
	daemon_pid_file_remove();
}
