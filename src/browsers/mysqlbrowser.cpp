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

#include "Browser.h"
#include "Log.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <stdarg.h>
#include <mysql/mysql.h>

class MySQLBrowser: public Browser {
	public:
		MySQLBrowser(const browser_context_t& context);
		virtual ~MySQLBrowser();

		virtual const char* get_next_file();
		virtual void reload();

		virtual void dump_queue();

	private:
		void connect();
		void disconnect();
		MYSQL_RES* query(const char* str, ...);

		void clear_fields();
		void allocate_fields(unsigned int n);
		void set_field(unsigned int n, const char* str);
		const char* get_field(unsigned int n);

		MYSQL* _conn;

		char** _fields;
		my_ulonglong _nr_of_fields;
		unsigned int _current_field;
};

class MySQLBrowser_factory {
	public:
		MySQLBrowser_factory(){
			Browser::register_factory(&MySQLBrowser_factory::factory, "mysql");
		}

		static Browser* factory(const browser_context_t& context){
			return new MySQLBrowser(context);
		}
};

MySQLBrowser_factory MySQLBrowser_factory_inst;

MySQLBrowser::MySQLBrowser(const browser_context_t& context):
	Browser(context),
	_conn(NULL),
	_fields(NULL),
	_nr_of_fields(0),
	_current_field(0){

	connect();
}

MySQLBrowser::~MySQLBrowser(){
	disconnect();

	clear_fields();
}

const char* MySQLBrowser::get_next_file(){
	return get_field(_current_field++);;
}

void MySQLBrowser::connect(){
	_conn = mysql_init(NULL);

	if (!mysql_real_connect(_conn, hostname(), username(), password(), database(), 0, NULL, 0)) {
		Log::message(Log::Fatal, "MySQLBrowser: Could not connect to database: %s\n", mysql_error(_conn));
		exit(2);
	}
}

void MySQLBrowser::disconnect(){
	mysql_close(_conn);
}

MYSQL_RES* MySQLBrowser::query(const char* str, ...){
	va_list arg;
	va_start(arg, str);

	char* query_str;
	vasprintf(&query_str, str, arg);

	if ( mysql_query(_conn, query_str) != 0 ) {
		Log::message(Log::Warning, "MySQLBrowser: Could not execute query '%s': %s\n", query_str, mysql_error(_conn));
	}

	free(query_str);

	va_end(arg);

	return mysql_store_result(_conn);
}

void MySQLBrowser::reload(){
	clear_fields();

	MYSQL_RES *res = query("SELECT fullpath FROM files WHERE bin_id = %d AND active = true ORDER BY sortorder, id", current_bin());
	MYSQL_ROW row;

	_nr_of_fields = mysql_num_rows(res);

	allocate_fields( _nr_of_fields + 1 ); // The last field is set to NULL

	unsigned int i;
	for ( i = 0; i < _nr_of_fields; i++ ){
		row = mysql_fetch_row(res);
		assert(row);

		set_field(i, row[0]);
	}
	set_field(i, NULL);

	mysql_free_result(res);
}

void MySQLBrowser::clear_fields(){
	if ( !_fields ){
		return;
	}

	unsigned int index = 0;

	while ( _fields[index] ){
		free(_fields[index]);
		_fields[index] = NULL;

		index++;
	}

	free(_fields);
	_fields = NULL;
}

void MySQLBrowser::allocate_fields(unsigned int n){
	_fields = (char**)malloc( sizeof(char*) * n );

	for ( unsigned int i = 0; i < 0; i++ ){
		_fields[i] = NULL;
	}
}

void MySQLBrowser::set_field(unsigned int n, const char* str){
	if ( !str ){
		_fields[n] = NULL;
		return;
	}

	_fields[n] = (char*)malloc( strlen(str)+1);
	strcpy( _fields[n], str );
}

const char* MySQLBrowser::get_field(unsigned int n){
	if ( _nr_of_fields == 0 ){
		return NULL;
	}

	return _fields[n % _nr_of_fields];
}

void MySQLBrowser::dump_queue(){
	Log::message(Log::Debug, "MySQLBrowser: Dumping queue\n");

	unsigned int index = 0;

	while ( _fields[index] ){
		Log::message(Log::Debug, "  %s\n", _fields[index++]);
	}
}
