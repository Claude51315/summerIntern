#ifndef _EMOTIONROI 
#define _EMOTIONROI
#include <opencv2\opencv.hpp>
#include<vector>
#include <iostream>
#include<fstream>
#include<string>
#include<time.h>

#define CANVAS_WIDTH 1024
#define CANVAS_HEIGHT 768
#define N 5  // number of input image 

using namespace cv;
#define upAdjacent 100
#define downAdjacent 101
#define leftAdjacent 102
#define rightAdjacent 103

class emotionData {
	public:
	Mat originImage  , stimulusMap;
	vector<Mat> candidateROI;
	Rect currentRect;
	int layer;// not sure 
	int level;
	/*!
	0 : top left
	1 : top right
	2 : bottom left
	3 : bottom right
	*/
	Point seed , corners[4];
	double emotionROI_ratio(Mat stimuiusMap,int min_x, int min_y, int max_x, int max_y);	
	/*
		move the currentRect and update seed point , corners 
	*/
	void move(Mat &canvas , int deltaX , int deltaY);
	/*
		initialize the class members
		level 
		updateCandidateROI(); 
		updateCurrentRect(); // choose candidateROI[layer] 
		updateCorners();  // update the remaining three corners 
	*/
	void initialize();
	void updateCandidateROI();	
	/*
		need CurrentRect to be correct ; 
	*/
	void updateCorners();
	/*
		need seed point , i.e. corner[0]  and layer to be correct ; 
		border detection !
	*/
	void updateCurrentRect();
	Rect getAdjacentBlankArea(Mat& boolMap , int side);
	
};
/*!
	randomly assign a point to seed point ,i.e. top left corner to all input images.
*/
void randomSeedPoint(emotionData src[], int numOfSource , int canvasWidth, int canvasHeight);
double blankAreaRatio(Mat &src);
double resizeRatio_x (emotionData src[], int numOfSource, int canvasWidth);
double resizeRatio_y (emotionData src[], int numOfSource, int canvasHeight);
//bool isOverlapped();
//int overlappedArea();
//void swapEmotionROI(emotionROI a , emotionROI b);
//double varianceOfEmotionROIRatio(emotionROI *p , int n );

void draw(emotionData *src , Mat &canvas);
void updateBoolMap(emotionData *src , Mat& outputBoolMap);
bool rectIsValid(Rect tmpRect);


bool readImage( std::fstream& emotionFiles, emotionData& output  , int number ); 
class emotionROI  //  for reading images
{
public:
	Rect emotionRect;
	bool emotionCategory[7];
	void print();
};
#endif
