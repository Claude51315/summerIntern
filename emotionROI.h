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
	double emotionROIRatio(){
		return area(currentROI) / area(maxEmotionROI);
	}	
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
=======
};


double blankAreaRatio(){
	// return area(all the emotionROIs) / area(the whole image);
}
bool isOverlapped();
int overlappedArea();
void swapEmotionROI(emotionROI a , emotionROI b);
double varianceOfEmotionROIRatio(emotionROI *p , int n );


>>>>>>> origin/master
#endif
