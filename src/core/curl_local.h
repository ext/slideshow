#ifndef CURL_LOCAL_H
#define CURL_LOCAL_H

#include <stdlib.h>
#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif

struct MemoryStruct {
	char *memory;
	size_t size;
};

size_t curl_local_resize(void *ptr, size_t size, size_t nmemb, void *data);

#ifdef __cplusplus
}
#endif

#endif /* CURL_LOCAL_H */
