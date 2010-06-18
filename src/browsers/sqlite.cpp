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

#include "Browser.h"
#include "exception.h"
#include "Log.h"
#include <sqlite3.h>
#include <cstring>
#include <vector>

class SQLiteBrowser: public Browser {
	public:
		SQLiteBrowser(const browser_context_t& context);
		virtual ~SQLiteBrowser();

		virtual const char* get_next_file();
		virtual void reload();

		virtual void dump_queue();

	private:
		void clear();
		void connect();
		void disconnect();

		typedef std::vector<char*> vector;

		sqlite3* _conn;
		sqlite3_stmt* _query;

		vector _slides;
		vector::iterator _current;
};

REGISTER_BROWSER_FACTORY(SQLiteBrowser, sqlite);

SQLiteBrowser::SQLiteBrowser(const browser_context_t& context)
	: Browser(context)
	, _conn(NULL)
	, _query(NULL) {

	connect();
	reload();
}

SQLiteBrowser::~SQLiteBrowser(){
	clear();
	disconnect();
}

void SQLiteBrowser::clear(){
	for ( vector::iterator it = _slides.begin(); it != _slides.end(); ++it ){
		free(*it);
	}
	_slides.clear();
	_current = _slides.end();
}

const char* SQLiteBrowser::get_next_file(){
	if ( _slides.size() == 0 ){
		return NULL;
	}

	if ( _current == _slides.end() ){
		_current = _slides.begin();
	}

	return *_current++;
}

void SQLiteBrowser::connect(){
	int ret;
	if ( (ret = sqlite3_open(database(), &_conn)) != SQLITE_OK ){
		printf("sqlite3_open failed with %d\n", ret);
	}

	const char* q =
		" SELECT"
		"	path "
		"FROM"
		"	slide "
		"WHERE"
		"	queue_id = ? "
		"ORDER BY"
		"	sortorder ";
	if ( (ret = sqlite3_prepare_v2(_conn, q, (int)(strlen(q)+1), &_query, NULL)) != SQLITE_OK ){
		printf("sqlite3_prepare_v2 failed with %d\n", ret);
	}
}

void SQLiteBrowser::disconnect(){
	sqlite3_finalize(_query);
	sqlite3_close(_conn);
}

void SQLiteBrowser::reload(){
	clear();

	sqlite3_bind_int(_query, 1, current_bin());

	int ret;
	while ( ( ret = sqlite3_step(_query) ) == SQLITE_ROW ){
		const unsigned char* path = sqlite3_column_text(_query, 0);
		Log::message(Log::Info, "slide: %s\n", (char*)path);
		_slides.push_back(strdup((const char*)path));
	}

	switch ( ret ){
		case SQLITE_DONE: break;
		case SQLITE_MISUSE: Log::message(Log::Info, "sqlite3_step failed: SQLITE_MISUSE\n"); break;
		default: Log::message(Log::Info, "sqlite3_step failed: %d\n", ret);
	}

	sqlite3_reset(_query);

	_current = _slides.begin();
}

void SQLiteBrowser::dump_queue(){

}
