#include "flatfile.h"
#include <cassert>
#include <sys/file.h>
s
FlatFileBrowser::FlatFileBrowser(const char* filename):
	_filename(filename)
	_records(NULL),
	_index(0),
	_record_cnt(0){
	
	read_file();
}

FlatFileBrowser::~FlatFileBrowser(){
	clear_records();
}

void FlatFileBrowser::read_file(){
	clear_records();
	
	FILE* f = fopen(_filename, "r");

	if ( !f ){
		throw std::runtime_error("Could not open file");
	}
	
	while ( flock(f, LOCK_EX) != -1 ){
		if ( errno != EWOULDBLOCK ){
			printf("An unknown error occured when locking the file.\n")
		}
	}
	
	
	
}

void FlatFileBrowser::clear_records(){
	for ( unsigned int n = 0; n < _record_cnt; n++ ){
		free( _records[n] );
	}
	free(_records);
	
	_records = NULL;
	_record_cnt = 0;
}

void FlatFileBrowser::get_record(unsigned int n){
	assert( _records );
	assert( n < _record_cnt );
	
	return _records[n];
}
