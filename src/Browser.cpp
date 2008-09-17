#include "Browser.h"
#include <map>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>

typedef std::map<const char*, Browser::factory_callback> factory_map;
typedef std::pair<const char*, Browser::factory_callback> pair;
typedef factory_map::iterator iterator;

factory_map* factories = NULL;

Browser::Browser(const browser_context_t& context):
	_bin(0),
	_username(NULL),
	_password(NULL),
	_database(NULL),
	_hostname(NULL){

	set_username(context.user);
	set_password(context.pass);
	set_hostname(context.host);
	set_database(context.name);
}

Browser::~Browser(){
	free(_username);
	free(_password);
	free(_hostname);
	free(_database);
}

void Browser::set_username(const char* username){
	free(_username);
	_username = NULL;

	if ( !username ){
		return;
	}

	_username = (char*)malloc( strlen(username) + 1 );
	strcpy( _username, username);
}

void Browser::set_password(const char* password){
	free(_password);
	_password = NULL;

	if ( !password ){
		return;
	}

	_password = (char*)malloc( strlen(password) + 1 );
	strcpy( _password, password);
}

void Browser::set_database(const char* database){
	free(_database);
	_database = NULL;

	if ( !database ){
		return;
	}

	_database = (char*)malloc( strlen(database) + 1 );
	strcpy( _database, database);
}

void Browser::set_hostname(const char* hostname){
	free(_hostname);
	_hostname = NULL;

	if ( !hostname ){
		return;
	}

	_hostname = (char*)malloc( strlen(hostname) + 1 );
	strcpy( _hostname, hostname);
}

void Browser::register_factory(factory_callback callback, const char* name){
	if ( !factories ){
		factories = new factory_map;
	}

	factories->insert(pair(name, callback));
}

Browser* Browser::factory(const char* string, const char* password){
	if ( !(factories && string) ){
		return NULL;
	}

	browser_context_t context = get_context(string);

	// If the contex doesn't contain a password and a password was passed from stdin (arg password)
	// we set that as the password in the context.
	if ( !context.pass && password ){
		context.pass = (char*)malloc( strlen(password) + 1 );
		strcpy(context.pass, password);
	}

	for ( iterator it = factories->begin(); it != factories->end(); ++it ){
		if ( strcmp(context.provider, it->first) == 0){
			Browser* browser = it->second(context);
			free_context(context);
			return browser;
		}
	}

	free_context(context);

	return NULL;
}
