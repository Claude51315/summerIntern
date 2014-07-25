#ifndef _EMOTIONROI 
#define _EMOTIONROI
#include <opencv2\opencv.hpp>
#include<vector>
#include <iostream>
#include<fstream>
#include<string>
#include<time.h>
using namespace cv;
	class emotionData {
	public:
	Mat originImage , currentROI , stimulusMap;
	vector<Mat> candidateROI;

	Rect currentRect;
	int layer;// not sure 
	int number , level;
	Point leftUpPoint;
	/*
		for
	*/
	double emotionROI_ratio(Mat stimuiusMap,int min_x, int min_y, int max_x, int max_y);	
	void move(int deltaX , int deltaY);
	/*
		initialize the remaining class member 
	*/
	void initialize();
	bool expand();
	void updateCandidateROI();	
};

void randomLeftUpPoint(emotionData src[], int numOfSource , int canvasWidth, int canvasHeight);
double blankAreaRatio(Mat src);
double resizeRatio_x (emotionData src[], int numOfSource, int canvasWidth);
double resizeRatio_y (emotionData src[], int numOfSource, int canvasHeight);
//bool isOverlapped();
//int overlappedArea();
//void swapEmotionROI(emotionROI a , emotionROI b);
//double varianceOfEmotionROIRatio(emotionROI *p , int n );

/*!
initialize 
load in origin image and stimulus map only !
number means n-th-input
set level to be zero 
*/
bool readImage( std::fstream& emotionFiles, emotionData& output  , int number ); 
class emotionROI  //  for reading images
{
public:
	Rect emotionRect;
	bool emotionCategory[7];
	void print();
};
#endif
