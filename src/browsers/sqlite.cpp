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

		virtual slide_context_t get_next_file();
		virtual void reload();

		virtual void dump_queue();

	private:
		void connect();
		void disconnect();

		void pop_intermediate(int id) const;

		bool _loop;
		int _old_id;

		sqlite3* _conn;
		sqlite3_stmt* _query;
		sqlite3_stmt* _query_looping;
		sqlite3_stmt* _query_pop_intermediate;
};

REGISTER_BROWSER_FACTORY(SQLiteBrowser, sqlite);
REGISTER_BROWSER_FACTORY(SQLiteBrowser, sqlite3);

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
	/* if we change queue we reset the position back to the start */
	if ( current_bin() != id ){
		_old_id = -1;
	}

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
			Log::message(Log::Info, "query_loop::sqlite3_step failed: SQLITE_MISUSE\n");
			break;
		default:
			Log::message(Log::Info, "query_loop::sqlite3_step failed: %d\n", ret);
			break;
	}

	sqlite3_reset(_query_looping);
}

slide_context_t SQLiteBrowser::get_next_file(){
	slide_context_t slide;
	slide.filename = NULL;
	slide.assembler = NULL;

	sqlite3_bind_int(_query, 1, current_bin());
	sqlite3_bind_int(_query, 2, _old_id);

	int ret = sqlite3_step(_query);
	if ( ret == SQLITE_DONE ){
		sqlite3_reset(_query);

		if ( !_loop ){
			Log::message(Log::Debug, "queue finished\n");
			return slide;
		}

		Log::message(Log::Debug, "queue wrapping\n");
		_old_id = -1;
		sqlite3_bind_int(_query, 1, current_bin());
		sqlite3_bind_int(_query, 2, _old_id);

		ret = sqlite3_step(_query);
		if ( ret == SQLITE_DONE ){
			sqlite3_reset(_query);
			return slide;
		}
	}

	int id = -1;
	int sort_order = -1;
	int queue_id = -1;

	switch ( ret ){
		case SQLITE_ROW:
			/* read columns */
			id              = sqlite3_column_int(_query, 0);
			slide.filename  = strdup((const char*)sqlite3_column_text(_query, 1));
			sort_order      = sqlite3_column_int(_query, 2);
			queue_id        = sqlite3_column_int(_query, 3);
			slide.assembler = strdup((const char*)sqlite3_column_text(_query, 4));

			Log::message(Log::Info, "slide: %s\n", slide.filename);
			Log::message(Log::Debug, "\tid: %d\n", id);
			Log::message(Log::Debug, "\tsort_order: %d\n", sort_order);
			Log::message(Log::Debug, "\tqueue_id: %d\n", queue_id);

			/* only update id if it comes from a regular queue, i.e., not from intermediate queue. */
			if ( queue_id > 0 ){
				_old_id = sort_order;
			} else {
				/* pop intermediate slides back to unsorted */
				Log::message(Log::Debug, "popping intermediate slide\n", slide.filename, id, queue_id);
				pop_intermediate(id);
			}

			break;
		case SQLITE_MISUSE:
			Log::message(Log::Info, "query_slide::sqlite3_step failed: SQLITE_MISUSE\n");
			Log::message(Log::Debug, "\tqueue_id: %d\n", current_bin());
			Log::message(Log::Debug, "\told_id: %d\n", _old_id);
			break;
		default:
			Log::message(Log::Info, "query_slide::sqlite3_step failed: %d\n", ret);
			break;
	}

	sqlite3_reset(_query);

	return slide;
}

void SQLiteBrowser::pop_intermediate(int id) const {
	sqlite3_bind_int(_query_pop_intermediate, 1, id);
	int ret = sqlite3_step(_query_pop_intermediate);
	sqlite3_reset(_query_pop_intermediate);

	switch ( ret ){
		case SQLITE_DONE:
		case SQLITE_ROW:
			return;
		case SQLITE_MISUSE:
			Log::message(Log::Info, "pop_intermediate::sqlite3_step failed: SQLITE_MISUSE\n");
			break;
		default:
			Log::message(Log::Info, "pop_intermediate::sqlite3_step failed: %d\n", ret);
			break;
	}
}

void SQLiteBrowser::connect(){
	int ret;
	if ( (ret = sqlite3_open(database(), &_conn)) != SQLITE_OK ){
		Log::message(Log::Fatal, "sqlite3_open failed with %d\n", ret);
	}

	const char* q =
		"	SELECT" /* select from intermediate queue */
		"		id, "
		"		path, "
		"		sortorder, "
		"		queue_id, "
		"		assembler "
		"	FROM"
		"		slide "
		"	WHERE"
		"		queue_id = -1 " /* -1 is intermediate queue */
		"UNION "
		"	SELECT" /* select next slide from regular queue */
		"		id, "
		"		path, "
		"		sortorder, "
		"		queue_id, "
		"		assembler "
		"	FROM"
		"		slide "
		"	WHERE"
		"		queue_id = ? AND "
		"		sortorder > ? "
		"ORDER BY"
		"	queue_id, "
		"	sortorder "
		"LIMIT 1";
	if ( (ret = sqlite3_prepare_v2(_conn, q, (int)(strlen(q)+1), &_query, NULL)) != SQLITE_OK ){
		Log::message(Log::Fatal, "query_slide::sqlite3_prepare_v2 failed with %d\n", ret);
	}

	q =
		"SELECT "
		"	loop "
		"FROM "
		"	queue "
		"WHERE "
		"	id = ? "
		"LIMIT 1";
	if ( (ret = sqlite3_prepare_v2(_conn, q, (int)(strlen(q)+1), &_query_looping, NULL)) != SQLITE_OK ){
		Log::message(Log::Fatal, "query_loop::sqlite3_prepare_v2 failed with %d\n", ret);
	}

	q =
		"UPDATE "
		"	slide "
		"SET "
		"	queue_id = 0 "
		"WHERE "
		"	id = ?";
	if ( (ret = sqlite3_prepare_v2(_conn, q, (int)(strlen(q)+1), &_query_pop_intermediate, NULL)) != SQLITE_OK ){
		Log::message(Log::Fatal, "pop_intermediate::sqlite3_prepare_v2 failed with %d\n", ret);
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
