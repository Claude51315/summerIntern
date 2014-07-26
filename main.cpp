#include "emotionROI.h"
void main()
{
	Mat canvas(CANVAS_HEIGHT   , CANVAS_WIDTH , CV_8UC3 , Scalar(0,0,0)),
		boolMap(CANVAS_HEIGHT   , CANVAS_WIDTH , CV_32S , Scalar(0)) , 
		block(CANVAS_HEIGHT   , CANVAS_WIDTH , CV_32S , Scalar(2000)) ;;
	emotionData src[N];
	std::fstream  imageData;
	imageData.open("emotionROI_value.txt", std::fstream::in);
	if(!imageData.is_open())
	{
		std::cerr<<"cannot open file."<<std::endl; exit(1);
	}
	randomSeedPoint(src, N, CANVAS_WIDTH, CANVAS_HEIGHT); // randomly choose seed point 
	for(size_t i = 0 ; i< N ;++i)
	{
		readImage( imageData , src[i],i); // origin image , stimulus map , layer
		src[i].initialize();
	}	
	imageData.close();
	draw(src , canvas);
	int nn = 0 ;
	srand(time(NULL));
	while(1)
	{
		for(int i = 0 ;i< N ;i++)
			src[i].move(canvas,rand()%40-20,rand()%40-20);
		
		nn +=10;
		draw(src , canvas);
		imshow("qq", canvas);
		waitKey(0);
	}
	
}
