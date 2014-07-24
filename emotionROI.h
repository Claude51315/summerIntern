#ifndef _EMOTIONROI 
#define _EMOTIONROI
#include <opencv2\opencv.hpp>
#include<vector>
#include <iostream>
#include<fstream>
#include<string>
using namespace cv;
	class emotionData {
	public:
	Mat originImage , currentROI , stimulusMap;
	vector<Mat> candidateROI;
	
	Rect currentRect;
	int layer;// not sure 
	Point leftUpPoint;
	double emotionROI_ratio(Mat stimuiusMap,int min_x, int min_y, int max_x, int max_y);	
	int area(Mat a){
		return a.cols * a.rows;
	}	
	void move(int deltaX , int deltaY);
	void expand(int );
	void updateCandidateROI();	
};


/*double blankAreaRatio(){
	// return area(all the emotionROIs) / area(the whole image);
}
//bool isOverlapped();
//int overlappedArea();
//void swapEmotionROI(emotionROI a , emotionROI b);
//double varianceOfEmotionROIRatio(emotionROI *p , int n );
*/

/*!
load in origin image and stimulus map only ! s
*/
bool readImage( std::fstream& emotionFiles, emotionData& output ); 
class emotionROI  //  for reading images
{
public:
	Rect emotionRect;
	bool emotionCategory[7];
	void print();
};
#endif
