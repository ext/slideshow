#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <json/json.h>

static const int DEFAULT_MASK = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
static const int USAGE_ERROR = 1;
static const int ACCESS_ERROR = 2;

typedef struct {
	const char* name;
	const char* fullpath;
	const char* data_path;
	const char* sample_path;
} slide_path_t;

typedef struct {
	int width;
	int height;
} resolution_t;

typedef struct {
	const char* type;
	const char** datafiles;
	slide_path_t path;
} slide_t;

static const char* program_name = NULL;

static void print_error(const char* fmt, ...){
	fprintf(stderr, "%s: ", program_name);
	va_list arg;
	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
}

static void print_usage_suggestion(){
	print_error("Try `%s --help' for more information.\n", program_name);
}

static void print_usage(){
	printf("Usage:\n");
	printf("%s create TARGET\n", program_name);
	printf("%s resample TARGET RESOLUTION [VIRTUAL_RESOLUTION]\n", program_name);
}

/**
 * Concaternates two strings. Similar to strcat but returns a *new* string instead.
 * Caller should free memory using free
 */
static const char* str_concat(const char* a, const char* b){
	size_t len = strlen(a) + strlen(b) + 1;
	char* buf = (char*)malloc(len);
	sprintf(buf, "%s%s", a, b);
	return buf;
}

static slide_path_t slide_filename_from_name(const char* name){
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

static int slide_create_directories(const slide_path_t* path_obj){
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

	if ( slide_create_directories(&path_obj) != 0 ){
		// error already shown.
		return ACCESS_ERROR;
	}

	return 0;
}

static resolution_t resolution_from_string(const char* str){
	int width;
	int height;
	resolution_t res = {0, 0};

	if ( sscanf(str, "%dx%d", &width, &height) != 2 ){
		return res;
	}

	res.width = width;
	res.height = height;

	return res;
}

int slide_resample(slide_t* target, resolution_t* resolution, resolution_t* virtual_resolution){
	/* ignore types for now, assume image */
	/* also, images only have exactly one datafile, assue that is true too */

	printf("datafile: %s\n", target->datafiles[0]);

	return 0;
}

static const char* json_object_get_string_copy(struct json_object* obj){
	const char* a = json_object_get_string(obj);
	char* b = (char*)malloc(strlen(a) + 1);
	strcpy(b, a);
	return b;
}

void slide_type_from_json(slide_t* slide, struct json_object* meta){
	struct json_object* type = json_object_object_get(meta, "type");
	slide->type = json_object_get_string_copy(type);
	json_object_put(type);
}
void slide_datafiles_from_json(slide_t* slide, struct json_object* meta){
	struct json_object* data = json_object_object_get(meta, "data");
	int nr_datafiles = json_object_array_length(data);
	slide->datafiles = (const char**)malloc(sizeof(char*) * nr_datafiles);
	for ( int i = 0; i < nr_datafiles; i++ ){
		 struct json_object* file = json_object_array_get_idx(data, i);
		 slide->datafiles[i] = json_object_get_string_copy(file);
		 json_object_put(file);
	}
	json_object_put(data);
}

slide_t* slide_from_name(const char* name){
	char* metafilename;
	asprintf(&metafilename, "%s/meta", name);

	FILE* meta_f = fopen(metafilename, "r");
	free(metafilename);

	if ( !meta_f ){
		print_error("Target is not a valid slide or you do not have permission to read it.\n");
		return NULL;
	}

	/* This will cause an error one day */
	static const int max_size = 1024;

	char meta_buf[max_size];
	int bytes = fread(meta_buf, 1, max_size, meta_f);

	struct json_object* meta = json_tokener_parse(meta_buf);
	if ( is_error(meta) ){
		print_error("Failed to parse meta.\n");
		return NULL;
	}
	assert( json_object_is_type(meta, json_type_object) );

	slide_t* slide = (slide_t*)malloc(sizeof(slide_t));

	slide_type_from_json(slide, meta);
	slide_datafiles_from_json(slide, meta);

	json_object_put(meta);

	return slide;
}

void slide_free(slide_t* slide){
	free(slide);
}

int main(int argc, const char* argv[]){
	program_name = argv[0];

	if ( argc < 2 ){
		print_usage();
		return 1;

	} else if ( strcmp("create", argv[1]) == 0 ){
		if ( argc < 3 ){
			print_error("create requires target.\n");
			print_usage_suggestion();
			return USAGE_ERROR;
		}

		const char* name = argv[2];

		return slide_create(name);

	} else if ( strcmp("resample", argv[1]) == 0 ){
		if ( argc < 4 ){
			print_error("resample requires target and resolution.\n");
			print_usage_suggestion();
			return USAGE_ERROR;
		}

		const char* filename = argv[2];
		slide_t* target = slide_from_name(filename);
		if ( !target ){
			print_error("failed to load `%s'.\n", filename);
			return ACCESS_ERROR;
		}

		resolution_t resolution = resolution_from_string(argv[3]);

		int rc = slide_resample(target, &resolution, NULL);
		slide_free(target);

		return rc;
	} else {
		print_error("Unknown action `%s'.\n", argv[1]);
		print_usage_suggestion();
		return USAGE_ERROR;
	}
}

