#include "emotionROI.h"

double globalblockedRatio(emotionData src[]);
void main()
{
	time_t t = clock();
	Mat canvas(CANVAS_HEIGHT   , CANVAS_WIDTH , CV_8UC3 , Scalar(0,0,0)),
		boolMap(CANVAS_HEIGHT   , CANVAS_WIDTH , CV_8U , Scalar(0)) , 
		block(CANVAS_HEIGHT   , CANVAS_WIDTH , CV_8U , Scalar(255)) ;;
	
	emotionData src[N]; 
	std::fstream  imageData;
	imageData.open("emotionROI_value.txt", std::fstream::in);
	if(!imageData.is_open())
	{
		std::cerr<<"cannot open file."<<std::endl;
		 exit(1);
	}
	
	randomSeedPoint(src, N, CANVAS_WIDTH, CANVAS_HEIGHT); // randomly choose seed point 
	for(size_t i = 0 ; i< N ;++i)
	{
		readImage( imageData , src[i],i); // origin image , stimulus map , layer
		src[i].initialize();
	}
	imageData.close();
	updateBoolMap(src , boolMap);
	draw(src,canvas);
	for(int i = 0 ; i< N ; ++i)
		src[i].updateVisbleAreaRatio(canvas);
	for(int i = 0 ; i< N ;i++)
	{
		std::cout << src[i].visbleAreaRatio <<std::endl;
		std::cout << src[i].level <<std::endl;
		std::cout<<"-----"<<std::endl;
	}
	std::cout << "before : blank area ratio "<< globalBlankAreaRatio(boolMap)<< std::endl;
	std::cout << "before : blocked area ratio "<< totalBlockedAreaRatio(src , canvas)<< std::endl;
	
	std::cout << "==========================="<< std::endl;
	imwrite("origin.jpg" , canvas);
	int order[N];
	//VideoWriter out("out.avi",-1,5,Size(1024,768));
	//out << canvas ; 
	int n = 0 ,qq = 0; 
	bool vFlag = false ; 
	
	//::cout  <<"WWW : "<< src[0].blockedEmotionROIRatio(canvas) <<std::endl;
	while(    n++  <  1  )//  && globalBlankAreaRatio(boolMap) > 0.25
	{
		qq = 0 ;
		vFlag = false; 
		while(vFlag == false && qq < 25 )
		{
			vFlag = true ;
			localMove(src , canvas ,boolMap);
			for(int i = 0 ; i< N ;i++)
				if(src[i].visbleAreaRatio  < 0.8 || src[i].LevelZeroROIBlockedRatio(canvas) <0.9)
				{
					globalRandomMove(src[i],canvas);
					//out << canvas ;
					vFlag = false; 
				}
			draw(src , canvas);
			updateBoolMap(src , boolMap);
			qq++;
		}
		for(int i = 0 ; i< N ;i++)
			src[i].expand();
		draw(src , canvas);
		for(int i = 0 ; i< N ;i++)
			src[i].updateVisbleAreaRatio(canvas);
		updateBoolMap(src , boolMap);
	}
	for(int i = 0 ; i< N ;i++)
	{
		std::cout << src[i].visbleAreaRatio <<std::endl;
		std::cout << src[i].level <<std::endl;
		std::cout<<"-----"<<std::endl;
	}
	std::cout << "after : blank area ratio "<< globalBlankAreaRatio(boolMap)<< std::endl;
	std::cout << "after : blocked area ratio "<< totalBlockedAreaRatio(src , canvas)<< std::endl;
	
	imwrite("result.jpg" , canvas);
	t = clock() - t ; 
	std::cout<< t <<std::endl;
	system("pause");
	
}
double globalblockedRatio(emotionData src[])
{
	double ans = 0 , allArea = 0 , blockedArea = 0;
	for(int i = 0 ; i < N ;i++)
	{
		allArea += src[i].currentRect.area();
		for(int j = i+1; j< N ;j++)
			blockedArea += (src[i].currentRect & src[j].currentRect).area();
	}
	ans = blockedArea / allArea ;
	return ans;
}