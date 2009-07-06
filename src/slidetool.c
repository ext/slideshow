#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static const int DEFAULT_MASK = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

typedef struct {
	const char* name;
	const char* fullpath;
	const char* data_path;
	const char* sample_path;
} slide_path_t;

static const char* program_name = NULL;

void print_error(const char* fmt, ...){
	fprintf(stderr, "%s: ", program_name);
	va_list arg;
	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
}

void print_usage_suggestion(){
	print_error("Try `%s --help' for more information.\n", program_name);
}

void print_usage(){
	printf("Usage:\n");
	printf("%s create TARGET\n", program_name);
}

/**
 * Concaternates two strings. Similar to strcat but returns a *new* string instead.
 * Caller should free memory using free
 */
const char* str_concat(const char* a, const char* b){
	size_t len = strlen(a) + strlen(b) + 1;
	char* buf = (char*)malloc(len);
	sprintf(buf, "%s%s", a, b);
	return buf;
}

slide_path_t slide_filename_from_name(const char* name){
	const char* extension = ".slide";
	const char* paths[] = {
		"/data",    // slide_path_t.data_path
		"/samples", // slide_path_t.sample_path
		NULL        // sentinel
	};

	slide_path_t path_obj;
	path_obj.name = name;
	path_obj.fullpath    = str_concat(name, extension);
	path_obj.data_path   = str_concat(path_obj.fullpath, paths[0]);
	path_obj.sample_path = str_concat(path_obj.fullpath, paths[1]);

	return path_obj;
}

int slide_create_directories(const slide_path_t* path_obj){
	const char* path[] = {
		path_obj->fullpath,
		path_obj->data_path,
		path_obj->sample_path
	};

	for ( size_t i = 0; i < (sizeof(path)/sizeof(char*)); i++ ){
		if ( mkdir(path[i], DEFAULT_MASK) != 0 ){
			print_error("mkdir failed to create %s.\n", path[i]);
			perror(program_name);
			return -1;
		}
	}

	return 0;
}

int slide_create(const char* name){
	slide_path_t path_obj = slide_filename_from_name(name);

	printf("creating slide\n");
	printf("mkdir %s\n", path_obj.fullpath);

	if ( slide_create_directories(&path_obj) != 0 ){
		// error already shown.
		return 2;
	}

	return 0;
}

int main(int argc, const char* argv[]){
	program_name = argv[0];

	if ( argc < 2 ){
		print_usage();
		return 1;
	}

	else if ( strcmp("create", argv[1]) == 0 ){
		if ( argc < 3 ){
			print_error("create requires target.\n");
			print_usage_suggestion();
			return 1;
		}

		const char* name = argv[2];

		return slide_create(name);

	} else {
		print_error("Unknown action `%s'.\n", argv[1]);
		print_usage_suggestion();
		return 1;
	}
}

