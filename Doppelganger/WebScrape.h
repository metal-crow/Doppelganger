#ifndef WEBSCR_H
#define WEBSCR_H

#include <curl/curl.h> 
#pragma warning(disable: 4996)

extern const char * starturl;

typedef struct MemoryStruct {
	char *memory;
	size_t size;
} Storage;

size_t readCurlData(void *incoming, size_t size_of_chunk, size_t num_of_chunks, void *storage);

char* findImageUrl(CURL *curl_handle, Storage * chunk, char* source_offste);

#endif