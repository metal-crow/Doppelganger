#include <curl/curl.h> 
#include <string.h> 
#pragma warning(disable: 4996)
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/contrib/contrib.hpp"
using namespace cv;

const char* face_cascade_name = "E:/Code Workspace/.external libraries/opencv 2.4.9/sources/data/haarcascades/haarcascade_frontalface_alt.xml";
CascadeClassifier face_cascade;
Ptr<FaceRecognizer> face_compare;
Size img_size(200,200);

#include "FaceDetection.h"

typedef struct MemoryStruct {
	char *memory;
	size_t size;
} Storage;

static size_t readCurlData(void *incoming, size_t size_of_chunk, size_t num_of_chunks, void *storage){
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

//open the image url, and read to a mat
void openCurlImage(const char * url){
	CURL *curl_handle;
	Storage image_store;
	image_store.memory = NULL;
	image_store.size = 0;

	curl_handle = curl_easy_init();
	if (curl_handle) {
		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
		curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, readCurlData);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&image_store);
		curl_easy_perform(curl_handle);

		// decode memory buffer using OpenCV
		Mat imgTmp = imdecode(Mat(1, image_store.size, CV_8UC1, image_store.memory), CV_LOAD_IMAGE_UNCHANGED);
		//find face in the image
		vector<Rect> faces;
		cvtColor(imgTmp, imgTmp, CV_BGR2GRAY);
		equalizeHist(imgTmp, imgTmp);
		face_cascade.detectMultiScale(imgTmp, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));
		//RESIZE TO FIT EigenFaceRecognizer trained image sizes!
		for (size_t i = 0; i < faces.size(); i++){
			Mat face;
			imgTmp(faces[i]).copyTo(face);
			resize(face, face, img_size);
			imwrite("web_face.jpg", face);
			int predictedLabel = -1;
			double confidence = 0.0;
			face_compare->predict(face, predictedLabel, confidence);

			printf("Result:%d %f\n", predictedLabel, confidence);
		}
	}

	curl_easy_cleanup(curl_handle);
	free(image_store.memory);
}

///Read the html form the page, and get the image urls
void openCurl(const char * url, Storage * chunk){
	CURL *curl_handle;
	CURLcode res;

	curl_handle = curl_easy_init();
	if (curl_handle) {
		//url
		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
		//follow redirect
		curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
		//write data with this function
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, readCurlData);
		//use this for storage in the write data function
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)chunk);

		//get url
		res = curl_easy_perform(curl_handle);

		char* source_offste = chunk->memory;
		//marker for image
		while ((source_offste = strstr(source_offste, "gridviewimage")) != NULL){
			//start of image url
			char* start_img_link = strstr(source_offste, "img src=");
			//end of image url
			char* end_img_link = strstr(source_offste, ".jpg");

			//store image url
			size_t url_length = (end_img_link - &start_img_link[9])+4;
			char * image_url = (char*)malloc(url_length+1);
			strncpy(image_url, &start_img_link[9], url_length);
			image_url[url_length] = '\0';

			printf("%s\n", image_url);

			//read image
			openCurlImage(image_url);

			free(image_url);

			//increment match pointer by 1 to go to next match
			source_offste = &source_offste[1];
		}

		//cleanup
		curl_easy_cleanup(curl_handle);
	}
}

const char * url = "http://www.listal.com/actors/_/all";

int main(void){
	Storage * chunk = (Storage *)malloc(sizeof(Storage));
	chunk->memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
	chunk->size = 0;    /* no data at this point */

	setupFaceDetector();

	Mat face = imread("testface.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	resize(face, face, img_size);
	int predictedLabel = -1;
	double confidence = 0.0;
	face_compare->predict(face, predictedLabel, confidence);

	printf("Result:%d %f\n", predictedLabel, confidence);

	//openCurl(url, chunk);
	
	free(chunk->memory);
	return 0;
}