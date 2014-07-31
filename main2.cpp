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
	
	/*for(int i = 0 ; i< N ;i++)
	{
		std::cout << src[i].visbleAreaRatio <<std::endl;
		std::cout << src[i].level <<std::endl;
		std::cout<<"-----"<<std::endl;
	}*/
	//std::cout << "before : blank area ratio "<< globalBlankAreaRatio(boolMap)<< std::endl;
	//std::cout << "before : blocked area ratio "<< totalBlockedAreaRatio(src , canvas)<< std::endl;
	std::cout << "==========================="<< std::endl;
	imwrite("origin.jpg" , canvas);
	int order[N];
	
	VideoWriter out("out.avi",-1,5,Size(1024,768));
	out << canvas ; 

	for(int i = 0 ; i<N ; i++)
	{
		std::cout << src[i].LevelZeroROIBlockedRatio(canvas) <<std::endl;
	}
	std::cout << "==========================="<< std::endl;
	
	//imshow("origin" , canvas);
	int n = 0 ,qq = 0; 
	bool vFlag = false ; 
	int qqq = 0 ; 
	/*for(int i = 0 ; i< N ; i++)
	{
		localMove(src , canvas ,boolMap ,i);
		out << canvas ;
		imshow("tmp" , canvas);
			waitKey(1);
	}*/
	double mininumBlankAreaRatio = 1 - maxinumVisibleAreaRatio(src , canvas , boolMap);
	mininumBlankAreaRatio = (mininumBlankAreaRatio < 0.2 ) ? 0.2 : mininumBlankAreaRatio ; 
	std::cout<<"mininumBlankAreaRatio = "<< mininumBlankAreaRatio <<std::endl;
	std::cout<<"globalBlankAreaRatio(boolMap) = "<< globalBlankAreaRatio(boolMap) <<std::endl;
	srand(time(NULL));
	int qqqq = 0;
	imshow("tmp" , canvas);
	waitKey(1);
	for(int i = 0 ; i< N ; i++)
	if(src[i].LevelZeroROIBlockedRatio(canvas) > 0.15 ) //src[i].visbleAreaRatio < 0.7   && 
	{
		globalRandomMove(src , i , canvas , boolMap);
		imshow("tmp" , canvas);
		waitKey(1);
	}
	while( mininumBlankAreaRatio >0 && globalBlankAreaRatio(boolMap) > 0.1 )
	{
		std::cout<<"wwwww1"<<std::endl;
		vFlag = false;
		while(  vFlag ==false  )
		{
			qqq++ ;
			vFlag = true;
			for(int i = 0 ; i< N ; i++)
				if(src[i].LevelZeroROIBlockedRatio(canvas) > 0.1 ) //src[i].visbleAreaRatio < 0.7   && 
				{
					std::cout<<"wwwww2"<<std::endl;
					globalRandomMove(src , i , canvas , boolMap);
					imshow("tmp" , canvas);
					waitKey(1);
					std::cout<<"wwwww2.555"<<std::endl;
					out << canvas ;

					localMove(src , canvas ,boolMap ,i);
					imshow("tmp" , canvas);
					waitKey(1);
					std::cout<<"wwwww2.7777"<<std::endl;
					out << canvas ; 
					vFlag =false;
					std::cout<<"wwwww3"<<std::endl;
				}
		std::cout<<"wwwww4"<<std::endl;
		/*for(int i = 0 ; i< N ; i++)
				if(src[i].localBlockedAreaRatio(canvas , src[i].level)>0.15)
				{
					std::cout<<"wwwww5"<<std::endl;
					localMove(src , canvas ,boolMap ,i);
					imshow("tmp" , canvas);
					waitKey(1);
					out << canvas ;
					std::cout<<"wwwww6"<<std::endl;
				}*/
			if(vFlag)
				break;
			std::cout<<"wwwww7"<<std::endl;
		for(int i = 0 ; i< N ; i++)
				//if(src[i].LevelZeroROIBlockedRatio(canvas) < 0.1 || src[i].localBlockedAreaRatio(canvas , src[i].level)>0.1)
		{
			localMove(src , canvas ,boolMap ,i);
			imshow("tmp" , canvas);
			waitKey(1);
			out << canvas ;
		}
			std::cout<<"wwwww8"<<std::endl;
			draw(src , canvas);
			for(int i = 0 ; i< N ;i++)
				src[i].updateVisbleAreaRatio(canvas);
			updateBoolMap(src , boolMap);
			std::cout<<"wwwww9"<<std::endl;
			std::cout <<"qqq = " <<qqq<<std::endl;
			std::cout<<"mininumBlankAreaRatio = "<< mininumBlankAreaRatio <<std::endl;
			std::cout<<"globalBlankAreaRatio(boolMap) = "<< globalBlankAreaRatio(boolMap) <<std::endl;
			for(int i = 0 ; i<N ; i++)
			{
				std::cout << src[i].LevelZeroROIBlockedRatio(canvas) <<std::endl;
			}
		}
		std::cout<<"qqqqqqqqqqq1"<<std::endl;
		if(mininumBlankAreaRatio > 0)
		for(int i = 0 ; i< N ;i++)
		{
			src[i].expand();
			
		}
		std::cout<<"qqqqqqqqqqq2"<<std::endl;
		draw(src , canvas);
		imshow("tmp" , canvas);
		waitKey(1);
		std::cout<<"qqqqqqqqqqq3"<<std::endl;
		for(int i = 0 ; i< N ;i++)
			src[i].updateVisbleAreaRatio(canvas);
		std::cout<<"qqqqqqqqqqq4"<<std::endl;
		updateBoolMap(src , boolMap);
		std::cout<<"qqqqqqqqqqq5"<<std::endl;
		mininumBlankAreaRatio = 1 - maxinumVisibleAreaRatio(src , canvas , boolMap);
		mininumBlankAreaRatio = (mininumBlankAreaRatio < 0 ) ? 0 : mininumBlankAreaRatio ; 
		std::cout<<"qqqqqqqqqqq6"<<std::endl;
		
	}
	std::cout <<"-------------" <<std::endl;
	for(int i = 0 ; i<N ; i++)
	{
		std::cout << src[i].LevelZeroROIBlockedRatio(canvas) <<std::endl;
	}
	std::cout << "after : blank area ratio "<< globalBlankAreaRatio(boolMap)<< std::endl;
	/*for(int i = 0 ; i< N ;i++)
	{
		std::cout << src[i].visbleAreaRatio <<std::endl;
		std::cout << src[i].level <<std::endl;
		std::cout<<"-----"<<std::endl;
	}
	std::cout << "after : blank area ratio "<< globalBlankAreaRatio(boolMap)<< std::endl;
	std::cout << "after : blocked area ratio "<< totalBlockedAreaRatio(src , canvas)<< std::endl;
	*/
	out << canvas ; 
	out << canvas ; 
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