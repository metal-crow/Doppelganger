#include <string.h> 
#include "FaceDetection.h"
#include "WebScrape.h"

const char* face_cascade_name = "E:/Code Workspace/.external libraries/opencv 2.4.9/sources/data/haarcascades/haarcascade_frontalface_alt.xml";
CascadeClassifier face_cascade;
Ptr<FaceRecognizer> face_compare;
Size img_size(200, 200);

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
		//for faces in image
		for (size_t i = 0; i < faces.size(); i++){
			Mat face;
			imgTmp(faces[i]).copyTo(face);
			//RESIZE TO FIT EigenFaceRecognizer trained image sizes!
			resize(face, face, img_size);
			int predictedLabel = -1;
			double confidence = 0.0;
			face_compare->predict(face, predictedLabel, confidence);

			printf("Result:%d %f\n", predictedLabel, confidence);

			char fn[100];
			sprintf(fn, "found/web_face%f.jpg", confidence);
			imwrite(fn, face);
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

			//note that i am passing this pointer, and changing what the pointer copy points to, but not this pointer, so this is pass by value
			char* image_url = findImageUrl(curl_handle, chunk,source_offste);

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

const char * starturl = "http://www.listal.com/actors/_/all";

int main(void){
	Storage * chunk = (Storage *)malloc(sizeof(Storage));
	chunk->memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
	chunk->size = 0;    /* no data at this point */

	setupFaceDetector();

	openCurl(starturl, chunk);
	
	free(chunk->memory);
	return 0;
}