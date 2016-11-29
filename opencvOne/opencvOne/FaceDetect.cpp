#include "stdafx.h"
#include "opencv2/objdetect.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

/** Function Headers */
void detectAndDisplay(Mat frame);

/** Global variables */
String face_cascade_name = "C:\\Users\\usr\\Documents\\Visual Studio 2013\\Projects\\opencvOne\\Release\\haarcascade_frontalface_alt_tree.xml";
String eyes_cascade_name = "C:\\Users\\usr\\Documents\\Visual Studio 2013\\Projects\\opencvOne\\Release\\haarcascade_eye_tree_eyeglasses.xml";
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
String window_name = "Capture - Face detection";

/** @function main */
int main(void)
{
	VideoCapture capture(0);
	Mat frame;

	//-- 1. Load the cascades
	if (!face_cascade.load(face_cascade_name)){ printf("--(!)Error loading face cascade\n"); return -1; };
	if (!eyes_cascade.load(eyes_cascade_name)){ printf("--(!)Error loading eyes cascade\n"); return -1; };

	//-- 2. Read the video stream
	if (!capture.isOpened()) { printf("--(!)Error opening video capture\n"); return -1; }

	while (true)
	{
		bool bSuccess = capture.read(frame);
		if (!bSuccess) //if not success, break loop
		{
			cout << "Cannot read a frame from video file" << endl;
			break;
		}

		//-- 3. Apply the classifier to the frame
		detectAndDisplay(frame);

		int c = waitKey(10);
		if ((char)c == 27) { break; } // escape
	}
	return 0;
}

/** @function detectAndDisplay */
void detectAndDisplay(Mat frame)
{
	std::vector<Rect> faces;
	Mat frame_gray;

	//Conversion of frame to grayscale
	cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
	//Contrast enhance(Spread out intensity distribution)
	equalizeHist(frame_gray, frame_gray);

	//-- Detect faces
	face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
	cout << faces.size();

	for (size_t i = 0; i < faces.size(); i++)
	{		
		Point pt1(faces[i].x, faces[i].y);
		Point pt2(faces[i].x + faces[i].width, faces[i].y + faces[i].height);
		rectangle(frame, pt1, pt2, Scalar(0, 255, 0), 1, 8, 0);
		Mat faceROI = frame_gray(faces[i]);
	}
	//-- Show what you got
	imshow(window_name, frame);
}