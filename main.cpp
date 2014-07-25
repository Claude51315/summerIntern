#include "emotionROI.h"
#define CANVAS_WIDTH 1024
#define CANVAS_HEIGHT 768
#define N 5  // number of input image 
void main()
{
	Mat canvas(CANVAS_HEIGHT   , CANVAS_WIDTH , CV_8UC3 , Scalar(0,0,0));
	emotionData src[N];
	std::fstream  imageData;
	imageData.open("emotionROI_value.txt", std::fstream::in);
	if(!imageData.is_open())
	{
		std::cerr<<"cannot open file."<<std::endl;
	}
	for(size_t i = 0 ; i< N ;++i)
	{
		readImage( imageData , src[i],i);
		src[i].updateCandidateROI();
		//std::cout<<src[i].candidateROI.size()<<std::endl;
	}
	imageData.close();
	randomLeftUpPoint(src, N, CANVAS_WIDTH, CANVAS_HEIGHT);
	Mat srcPointMap(CANVAS_HEIGHT,CANVAS_WIDTH,CV_8UC3,Scalar(0,0,0));
	for(size_t i = 0 ; i< N ;++i)
			std::cout << "[" << src[i].leftUpPoint.x << ", "<< src[i].leftUpPoint.y << "]" << std::endl;
	// randomly spread seeds
	// initialize currentRect 
	system("pause");
	
}