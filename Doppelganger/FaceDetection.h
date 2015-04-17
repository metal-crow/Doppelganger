#ifndef Face_Detection_H
#define Face_Detection_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/contrib/contrib.hpp"
using namespace cv;

extern const char* face_cascade_name;
extern CascadeClassifier face_cascade;
extern Ptr<FaceRecognizer> face_compare;
extern Size img_size;

void setupFaceDetector(void);

#endif