#include "WebScrape.h"

size_t readCurlData(void *incoming, size_t size_of_chunk, size_t num_of_chunks, void *storage){
	//dereference storage back to struct
	struct MemoryStruct * mem = (struct MemoryStruct *)storage;
	//the total size of the incoming data is
	size_t tot_size = size_of_chunk * num_of_chunks;

	//realloc memory
	mem->memory = (char*)realloc(mem->memory, mem->size + tot_size + 1);
	if (mem->memory == NULL) {
		/* out of memory! */
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	//copy new data into storage
	memcpy(&(mem->memory[mem->size]), incoming, tot_size);
	mem->size += tot_size;
	//null term
	mem->memory[mem->size] = '\0';

	return tot_size;
}

//given the starting url, which contains mulitple images, select the first url, and go to the final url that has higest res image
//user written
char* findImageUrl(CURL *curl_handle, Storage * chunk, char* source_offste){
	//start of image url
	char* start_img_link = strstr(source_offste, "href=")+6;
	//end of image url
	char* end_img_link = strstr(source_offste, "title")-2;

	//store image url
	size_t url_length = end_img_link - start_img_link;
	char * image_url = (char*)malloc(url_length + 9 + 1);
	strncpy(image_url, start_img_link, url_length);
	strncpy(&image_url[url_length], "/pictures", 9);
	image_url[url_length + 9] = '\0';

	printf("test %s\n",image_url);

	curl_easy_setopt(curl_handle, CURLOPT_URL, image_url);
	curl_easy_perform(curl_handle);
}