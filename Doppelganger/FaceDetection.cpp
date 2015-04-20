#include "FaceDetection.h"

void setupFaceDetector(void){
	Vector<Mat> inputFaces;
	//load faces from files
	for (int i = 1; i <= 5; i++){
		char buf[10];
		sprintf(buf, "face%d.jpg", i); // puts string into buffer
		printf("%s\n", buf);
		inputFaces.push_back(imread(buf, CV_LOAD_IMAGE_GRAYSCALE));
	}
	//start face detector
	if (!face_cascade.load(face_cascade_name)){
		printf("--(!)Error loading\n");
		assert(false);
	};

	//store perfect cropped faces for face recognizer training
	vector<Mat> images;
	vector<int> labels;

	//get the faces from the input image
	for (size_t i = 0; i < inputFaces.size(); i++){
		vector<Rect> faces;
		face_cascade.detectMultiScale(inputFaces[i], faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));

		//if face found (if there is more than 1 face in an image, we have a problem)
		if(faces.size()){
			//copy face roi to new mat
			Mat croppedImage;
			inputFaces[i](faces[0]).copyTo(croppedImage);

			//resize image
			resize(croppedImage, croppedImage, img_size);

			//store the face
			images.push_back(croppedImage);
			labels.push_back(1);
		}
	}

	face_compare = createEigenFaceRecognizer();
	face_compare->train(images, labels);
}

void test(void){
	Mat face = imread("testface.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	resize(face, face, img_size);
	int predictedLabel = -1;
	double confidence = 0.0;
	face_compare->predict(face, predictedLabel, confidence);

	printf("Result:%d %f\n", predictedLabel, confidence);
}