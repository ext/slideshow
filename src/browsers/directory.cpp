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

#include "Browser.h"
#include "Exceptions.h"
#include "Log.h"
#include <vector>
#include <algorithm>

#include <portable/asprintf.h>
#include <portable/scandir.h>

#ifdef WIN32
#	include "win32.h"
#endif

class DirectoryBrowser: public Browser {
	public:
		DirectoryBrowser(const browser_context_t& context);
		virtual ~DirectoryBrowser();

		virtual const char* get_next_file();
		virtual void reload();

		virtual void dump_queue();

	private:
		typedef std::vector<char*> vector;
		typedef vector::iterator iterator;

		void clear();

		char* _path;
		vector _files;
		iterator _cur;
};

REGISTER_BROWSER_FACTORY(DirectoryBrowser, directory);

DirectoryBrowser::DirectoryBrowser(const browser_context_t& context)
	: Browser(context) {

	_path = strdup(context.name);
	reload();
}

DirectoryBrowser::~DirectoryBrowser(){
	clear();
	free(_path);
}

const char* DirectoryBrowser::get_next_file(){
	char* tmp = (*_cur);
	++_cur;

	if ( _cur == _files.end() ){
		_cur = _files.begin();
	}

	return tmp;
}

void DirectoryBrowser::clear(){
	for ( iterator it = _files.begin(); it != _files.end(); ++it ){
		free(*it);
	}
	_files.clear();
}

static int filter(const struct dirent* d){
	if ( 
		strcmp(d->d_name, ".") == 0 ||
		strcmp(d->d_name, "..") == 0 ||
		strcmp(d->d_name, "Thumbs.db") == 0 ){

		return 0;
	}
	return 1;
}

#ifdef WIN32
#	define SEPARATOR "\\"
#else
#	define SEPARATOR "/"
#endif

void DirectoryBrowser::reload(){
	clear();

	struct dirent **namelist;
	int n = scandir(_path, &namelist, filter, NULL);

	while ( n-- > 0 ){ /* note that n is decremented here, but it also makes is
					    * suitable as index into namelist */

		_files.push_back(asprintf2("%s" SEPARATOR "%s", _path, namelist[n]->d_name));
		free(namelist[n]);
	}

	std::reverse(_files.begin(), _files.end());

	_cur = _files.begin();

	free(namelist);
}

void DirectoryBrowser::dump_queue(){
	Log::message(Log::Debug, "DirectoryBrowser: Dumping queue\n");
}
