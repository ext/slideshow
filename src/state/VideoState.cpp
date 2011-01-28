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
#include <unistd.h>
#include <fcntl.h>
#include <portable/asprintf.h>

int VideoState::child_pid = -1;
int VideoState::child_stdin = -1;
int VideoState::child_stdout = -1;

static const char* read2(int fd){
	static char buf[1024];
	while ( read(fd, buf, 1024) > 0 ){
		if ( strncmp(buf, "ANS_", 4) == 0 ){
			return buf;
		}

		Log::message(Log_Verbose, "mplayer: %s", buf);
	}

	switch ( errno ){
		case EWOULDBLOCK:
			break;
		default:
			Log::message(Log_Warning, "mplayer: read failed with code %d\n", errno);
	}

	return NULL;
}

VideoState::VideoState(State* state, const char* filename)
	: State(state)
	, _filename(strdup(filename)){

	char* tmp = asprintf2("load %s\n", _filename);
	write(child_stdin, tmp, strlen(tmp));
	free(tmp);
}

VideoState::~VideoState(){
	free(_filename);
	_filename = NULL;
}

State* VideoState::action(bool &flip){
	write(child_stdin, "get_property filename\n", 22);

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
	write(child_stdin, "quit\n", 5);
	return 0;
}

void VideoState::poll(){
	read2(child_stdout);
}
