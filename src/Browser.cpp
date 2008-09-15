#include "Browser.h"
#include <map>
#include <cstdio>
#include <cstring>

typedef std::map<const char*, Browser::factory_callback> factory_map;
typedef std::pair<const char*, Browser::factory_callback> pair;
typedef factory_map::iterator iterator;

factory_map* factories = NULL;

void Browser::register_factory(factory_callback callback, const char* name){
	if ( !factories ){
		factories = new factory_map;
	}

	factories->insert(pair(name, callback));
}

Browser* Browser::factory(const char* string){
	if ( !factories ){
		return NULL;
	}

	printf("%s\n", string);
	browser_context_t context = get_context(string);
	for ( iterator it = factories->begin(); it != factories->end(); ++it ){
		if ( strcmp(context.provider, it->first) == 0){
			printf("fount a matching provider: %p\n", it->second);
		}
	}

	return NULL;
}
