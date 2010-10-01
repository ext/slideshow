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

		virtual void change_bin(unsigned int id);

		virtual char* get_next_file();
		virtual void reload();

		virtual void dump_queue();

	private:
		void connect();
		void disconnect();

		bool _loop;
		int _old_id;

		sqlite3* _conn;
		sqlite3_stmt* _query;
		sqlite3_stmt* _query_looping;
};

REGISTER_BROWSER_FACTORY(SQLiteBrowser, sqlite);

SQLiteBrowser::SQLiteBrowser(const browser_context_t& context)
	: Browser(context)
	, _loop(true)
	, _old_id(-1)
	, _conn(NULL)
	, _query(NULL) {

	connect();
}

SQLiteBrowser::~SQLiteBrowser(){
	disconnect();
}

void SQLiteBrowser::change_bin(unsigned int id){
	Browser::change_bin(id);

	/* query whenever to loop this queue */
	sqlite3_bind_int(_query_looping, 1, current_bin());
	int ret = sqlite3_step(_query_looping);

	_loop = true;

	switch ( ret ){
		case SQLITE_DONE:
			break;
		case SQLITE_ROW:
			_loop = sqlite3_column_int(_query_looping, 0) == 1;
			break;
		case SQLITE_MISUSE:
			Log::message(Log::Info, "sqlite3_step failed: SQLITE_MISUSE\n");
			break;
		default:
			Log::message(Log::Info, "sqlite3_step failed: %d\n", ret);
			break;
	}

	sqlite3_reset(_query_looping);
}

char* SQLiteBrowser::get_next_file(){
	sqlite3_bind_int(_query, 1, current_bin());
	sqlite3_bind_int(_query, 2, _old_id);

	int ret = sqlite3_step(_query);
	if ( ret == SQLITE_DONE ){
		sqlite3_reset(_query);

		if ( !_loop ){
			Log::message(Log::Debug, "queue finished\n");
			return NULL;
		}

		Log::message(Log::Debug, "queue wrapping\n");
		sqlite3_bind_int(_query, 1, current_bin());
		sqlite3_bind_int(_query, 2, -1);

		ret = sqlite3_step(_query);
		if ( ret == SQLITE_DONE ){
			sqlite3_reset(_query);
			return NULL;
		}
	}

	char* path = NULL;

	switch ( ret ){
		case SQLITE_ROW:
			 path = strdup((const char*)sqlite3_column_text(_query, 0));
			_old_id = sqlite3_column_int(_query, 1);

			Log::message(Log::Info, "slide: %s\n", (char*)path);
			break;
		case SQLITE_MISUSE:
			Log::message(Log::Info, "sqlite3_step failed: SQLITE_MISUSE\n");
			break;
		default:
			Log::message(Log::Info, "sqlite3_step failed: %d\n", ret);
			break;
	}

	sqlite3_reset(_query);

	return path;
}

void SQLiteBrowser::connect(){
	int ret;
	if ( (ret = sqlite3_open(database(), &_conn)) != SQLITE_OK ){
		printf("sqlite3_open failed with %d\n", ret);
	}

	const char* q =
		" SELECT"
		"	path, "
		"	sortorder "
		"FROM"
		"	slide "
		"WHERE"
		"	queue_id = ? AND "
		"	sortorder > ? "
		"ORDER BY"
		"	sortorder "
		"LIMIT 1";
	if ( (ret = sqlite3_prepare_v2(_conn, q, (int)(strlen(q)+1), &_query, NULL)) != SQLITE_OK ){
		printf("sqlite3_prepare_v2 failed with %d\n", ret);
	}

	q =
		" SELECT "
		"	loop "
		"FROM "
		"	queue "
		"WHERE "
		"	id = ? "
		"LIMIT 1";
	if ( (ret = sqlite3_prepare_v2(_conn, q, (int)(strlen(q)+1), &_query_looping, NULL)) != SQLITE_OK ){
		printf("sqlite3_prepare_v2 failed with %d\n", ret);
	}
}

void SQLiteBrowser::disconnect(){
	sqlite3_finalize(_query_looping);
	sqlite3_finalize(_query);
	sqlite3_close(_conn);
}

void SQLiteBrowser::reload(){

}

void SQLiteBrowser::dump_queue(){

}
