#ifndef _EMOTIONROI 
#define _EMOTIONROI
#include <opencv2\opencv.hpp>
#include<vector>
#include <iostream>
using namespace cv;
	class emotionROI {
	Mat maxEmotionROI , currentROI;
	vector<Mat> candidateROI;
	Rect currentRect;
	int layer;// not sure 
	Point center;
	double emotionROIRatio(){
		return area(currentROI) / area(maxEmotionROI);
	}	
	int area(Mat a){
		return a.cols * a.rows;
	}	
	void move(int deltaX , int deltaY);
	void expand(int );
};


double blankAreaRatio(){
	// return area(all the emotionROIs) / area(the whole image);
}
bool isOverlapped();
int overlappedArea();
void swapEmotionROI(emotionROI a , emotionROI b);
double varianceOfEmotionROIRatio(emotionROI *p , int n );


#endif
