#include "emotionROI.h"
#define CANVAS_WIDTH 1024
#define CANVAS_HEIGHT 768
#define N 5  // number of input image 
void main()
{
	Mat canvas(CANVAS_WIDTH  , CANVAS_HEIGHT , CV_8UC3 , Scalar(0,0,0));
	emotionData src[N];
	std::fstream  imageData;
	imageData.open("emotionROI_value.txt", std::fstream::in);
	if(!imageData.is_open())
		std::cerr<<"cannot open file."<<std::endl;
	for(size_t i = 0 ; i< N ;++i)
	{
		readImage( imageData , src[i]);
		src[i].updateCandidateROI();
		std::cout<<src[i].candidateROI.size()<<std::endl;
		for(int j = 0 ;j<src[i].candidateROI.size();j++)
		{
			imshow("qq",src[i].candidateROI[j]);
			waitKey(0);
		}

		destroyWindow("qq");
	}

	imageData.close();
}