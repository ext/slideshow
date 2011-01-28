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
#include "VideoState.h"
#include "SwitchState.h"
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cassert>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <portable/asprintf.h>

static char read_buffer[1024];
static int child_pid = -1;
static int child_stdin = -1;
static int child_stdout = -1;

static const char* read2(int fd){
	while ( read(fd, read_buffer, 1024) > 0 ){
		if ( strncmp(read_buffer, "ANS_", 4) == 0 ){
			return read_buffer;
		}

		Log::message(Log_Verbose, "mplayer: %s", read_buffer);
	}

	switch ( errno ){
		case EWOULDBLOCK:
			break;
		default:
			Log::message(Log_Warning, "mplayer: read failed with code %d\n", errno);
	}

	return NULL;
}

void VideoState::command(const char* fmt, ...){
	assert(child_pid > 0);

	va_list ap;
	va_start(ap, fmt);
	char* tmp = vasprintf2(fmt, ap);
	write(child_stdin, tmp, strlen(tmp));
	free(tmp);
	va_end(ap);
}

VideoState::VideoState(State* state, const char* filename)
	: State(state)
	, _filename(strdup(filename)){

	if ( child_pid <= 0 ){
		Log::message(Log_Verbose, "no mplayer: process, cannot play video\n");
		return;
	}

	command("loadfile %s\n", filename);
}

VideoState::~VideoState(){
	free(_filename);
	_filename = NULL;
}

State* VideoState::action(bool &flip){
	if ( !child_pid <= 0 ){
		return new SwitchState(this);
	}

	command("get_property filename\n");

	while ( true ){
		const char* ans = read2(child_stdout);

		if ( !ans ){
			continue;
		}

		if ( strcmp(ans, "ANS_ERROR=PROPERTY_UNAVAILABLE\n") == 0){
			return new SwitchState(this);
		}

		return this;
	}
}

int VideoState::init(){
	/* Fork and get child std{in,out} */
	/* http://lists.mplayerhq.hu/pipermail/mplayer-dev-eng/2007-August/053602.html */

	int pipefds1[2];
	int pipefds2[2];

	pipe2(pipefds1, O_NONBLOCK);
	pipe2(pipefds2, O_NONBLOCK);

	child_pid = fork();

	close(pipefds1[!!child_pid]);
	close(pipefds2[!child_pid]);

	if (!child_pid) { /* in child process */
		dup2(pipefds1[1], STDOUT_FILENO);
		dup2(pipefds2[0], STDIN_FILENO);
		return execlp("mplayer", "slideshow-mplayer", "-slave", "-idle", "-quiet", "-msglevel", "all=-1:global=4", "-fs", NULL);
	} else { /* in parent process */
		child_stdin  = pipefds2[1];
		child_stdout = pipefds1[0];
		return 0;
	}
}

int VideoState::cleanup(){
	if ( child_pid > 0 ){
		command("quit\n");
	}
	return 0;
}

void VideoState::poll(){
	if ( child_pid < 0 ){
		return;
	}

	int sigstate = kill(child_pid, 0);
	if ( sigstate != 0 ){ /* child process terminated */
		Log::message(Log_Fatal, "mplayer process has gone away (for reasons unknown)");
		child_pid = -1;
	}

	int status;
	if ( waitpid(child_pid, &status, WNOHANG) != 0 ){
		if ( WIFEXITED(status) ){
			Log::message(Log_Fatal, "mplayer process has gone away (exited with code %d)\n", WEXITSTATUS(status));
		} else {
			Log::message(Log_Fatal, "mplayer process has gone away (terminated with signal %d)\n", WTERMSIG(status));
		}
		child_pid = -1;
	}

	read2(child_stdout);
}
