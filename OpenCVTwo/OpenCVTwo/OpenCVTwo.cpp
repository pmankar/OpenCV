// OpenCVTwo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

// function headers
void displayFrames(Mat frame);
void resetValues();

// global variables
double ROITopMargin = 0.57; // percentage
double ROIBottomMargin = 0.20; // percentage
double ROILeftMargin = 0.22;
double ROIRightMargin = 0.22;
int lineThreashold = 30;
int framesToScan = 100;
int minFramesRequired = 10; // less than defaultFramesToScan
int scanThreshold = 4;
bool reqMargin = true;
int leftMin;
int rightMax;
int LTR = 0;
int RTL = 0;
int xSel = 0;
int nol = 0;
string direction = " ";
int avgX = 0;
bool scaleInc = false;

int main()
{
	//string fileLoc = "G:\\TUD\\Sem V\\KOM\\Prog\\lane detection\\first project pc\\opencv-lane-vehicle-track-master\\opencv-lane-vehicle-track-master\\bin\\road.avi";
	string fileLoc = "C:\\Users\\usr\\Documents\\Visual Studio 2013\\Projects\\OpenCVTwo\\Release\\LaneChange.mp4";
	//fileLoc = "G:\\TUD\\Sem V\\KOM\\Prog\\Android\\scenario videos\\VID_20160310_191811.3gp";
	fileLoc = "G:\\TUD\\Sem V\\KOM\\Prog\\Android\\scenario videos\\VID_20160311_083930.3gp";
	VideoCapture capture(fileLoc);
	Mat frame;
	if (!capture.isOpened()) { printf("--(!)Error opening video file\n"); return -1; }
	bool pause = false;
	while (true)
	{
		if (!pause)
		{
			bool bSuccess = capture.read(frame);
			if (!bSuccess) //if not success, break loop
			{
				cout << "Cannot read a frame from video file" << endl;
				break;
			}
			displayFrames(frame);
		}

	#pragma region KeyMaps
		int c = waitKey(10);

		if ((char)c == 27) { break; } // escape
		if ((char)c == 'p'){ pause = !pause; } //pause
		if ((char)c == 'a'){ capture.set(CV_CAP_PROP_POS_FRAMES, capture.get(CV_CAP_PROP_POS_FRAMES) - 100); } // seek backwards
		if ((char)c == 'd'){ capture.set(CV_CAP_PROP_POS_FRAMES, capture.get(CV_CAP_PROP_POS_FRAMES) + 100); } // seek forwards

		// move ROI
		if ((int)c == 2424832) { ROILeftMargin -= 0.001; ROIRightMargin += 0.001; } // left
		if ((int)c == 2555904) { ROILeftMargin += 0.001; ROIRightMargin -= 0.001; } // right
		if ((int)c == 2490368) { ROITopMargin -= 0.001; ROIBottomMargin += 0.001; } // up
		if ((int)c == 2621440) { ROITopMargin += 0.001; ROIBottomMargin -= 0.001; } // down

		// scale ROI
		// use q to toggle increase / decrease mode
		if ((char)c == 'q') { scaleInc = !scaleInc; } // 
		if ((char)c == 'j') { ROILeftMargin += 0.001 * (scaleInc ? 1: -1); } // left
		if ((char)c == 'l') { ROIRightMargin += 0.001 * (scaleInc ? 1 : -1); } // right
		if ((char)c == 'i') { ROITopMargin += 0.001 * (scaleInc ? 1 : -1); } // up
		if ((char)c == 'k') { ROIBottomMargin += 0.001 * (scaleInc ? 1 : -1); } // down
	#pragma endregion
	}
	return 0;
}

void displayFrames(Mat frame)
{
	//imshow("original frame", frame);
	Mat blrFrame;
	GaussianBlur(frame, blrFrame, cvSize(9, 9), 5, 5);
	Mat frame_gray;
	leftMin = (((1 - (ROILeftMargin + ROIRightMargin)) * frame.cols) * 0.25 ) + ROILeftMargin * frame.cols;
	rightMax = (((1 - (ROILeftMargin + ROIRightMargin)) * frame.cols) * 0.75 ) + ROILeftMargin * frame.cols;
	Rect ROI = Rect(ROILeftMargin * frame.cols, ROITopMargin * frame.rows, (1 - (ROILeftMargin + ROIRightMargin)) * frame.cols, (1 - (ROITopMargin + ROIBottomMargin)) * frame.rows);
	//Conversion of frame to grayscale
	cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

	// Apply Canny
	Mat contours, cdst;
	Canny(frame, contours, 80, 80, 3);
	cvtColor(contours, cdst, COLOR_GRAY2BGR);

	// Hough Transform
	Mat dst;
	Mat newFrame(dst.rows, dst.cols, CV_8UC3);

	dst = contours(ROI);
	imshow("roidst", dst);
	nol = 0;
	xSel = 0;

#if 1
	vector<Vec2f> lines;
	// detect lines
	HoughLines(dst, lines, 1, CV_PI / 180, lineThreashold, 0, 0);

#pragma region drawLines
	// draw lines
	
	for (size_t i = 0; i < lines.size(); i++)
	{
		//cout << i << endl;
		float rho = lines[i][0], theta = lines[i][1];
		Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		double k = 1000;
		pt1.x = cvRound(x0 + k * (-b));
		pt1.y = cvRound(y0 + k * (a));
		pt2.x = cvRound(x0 - k * (-b));
		pt2.y = cvRound(y0 - k * (a));
		double dx = pt2.x - pt1.x;
		double dy = pt2.y - pt1.y;
		double angle = atan2(dy, dx) * 180.0 / CV_PI;
		if (angle < 0) angle = angle * (-1);
		double angleToDetect = 90;
		double angleDeviation = 35;

		pt1.y = pt1.y + (ROITopMargin * frame.rows);
		pt2.y = pt2.y + (ROITopMargin * frame.rows);
		pt1.x = pt1.x + (ROILeftMargin * frame.cols);
		pt2.x = pt2.x + (ROILeftMargin * frame.cols);
		clipLine(ROI, pt1, pt2);

		if ((angle < angleToDetect + angleDeviation) && (angle > angleToDetect - angleDeviation))
		{
			line(frame, pt1, pt2, Scalar(0, 0, 255), 4, CV_AA);
			nol++;
			// find the x coordinate of lowest point
			if (pt1.y > pt2.y) xSel += pt1.x;
			if (pt1.y < pt2.y) xSel += pt2.x;
		}
	}
#pragma endregion
#else
	vector<Vec4i> lines;
	HoughLinesP(dst, lines, 1, CV_PI / 180, 40, 0, 10);
	for (size_t i = 0; i < lines.size(); i++)
	{
		Vec4i l = lines[i];
		line(frame, Point(l[0] + (ROILeftMargin * frame.cols), l[1] + (ROITopMargin * frame.rows)), Point(l[2] + (ROILeftMargin * frame.cols), l[3] + (ROITopMargin * frame.rows)), Scalar(0, 0, 255), 3, LINE_AA);
	}
#endif

	framesToScan--;
	if (framesToScan <= 0 || framesToScan >=150)
	{
		framesToScan = 0;
		resetValues();
	}

	if (nol > 0)
	{
		if (avgX < (xSel / nol)) RTL++;
		if (avgX > (xSel / nol)) LTR++;
		framesToScan += scanThreshold;
		avgX = xSel / nol;
		if ((framesToScan > minFramesRequired) && (reqMargin))
		{
			if ((avgX < leftMin) || (avgX > rightMax))
				direction = (LTR > RTL) ? "===>" : "<===";
		}
		else
		if (framesToScan > minFramesRequired)
			direction = (LTR > RTL) ? "===>" : "<===";
	}
	putText(frame, format("%s \t %i \t %i", &direction, framesToScan, avgX), cvPoint(frame.cols / 2, frame.rows  * 0.95), FONT_HERSHEY_SIMPLEX, 0.5, cvScalar(0, 0, 255), 1, CV_AA);
	// circle to spot deviation range
	if (reqMargin)
	{
		circle(frame, cvPoint(leftMin, (1 - ROIBottomMargin) * frame.rows), 2, cvScalar(0, 0, 255));
		circle(frame, cvPoint(rightMax, (1 - ROIBottomMargin) * frame.rows), 2, cvScalar(0, 0, 255));
	}
	// rectangle to show ROI
	rectangle(frame, cvPoint(ROILeftMargin * frame.cols, ROITopMargin * frame.rows), cvPoint((1-ROIRightMargin) * frame.cols, (1-ROIBottomMargin) * frame.rows), CvScalar(0, 0, 255));
	imshow("detected lines", frame);
}

void resetValues()
{
	LTR = 0;
	RTL = 0;
	direction = " ";
	avgX = 0;
}