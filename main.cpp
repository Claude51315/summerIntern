#include "emotionROI.h"

double globalblockedRatio(emotionData src[]);
void main()
{
	time_t t = clock();
	Mat canvas(CANVAS_HEIGHT   , CANVAS_WIDTH , CV_8UC3 , Scalar(0,0,0)),
		boolMap(CANVAS_HEIGHT   , CANVAS_WIDTH , CV_8U , Scalar(0)) , 
		block(CANVAS_HEIGHT   , CANVAS_WIDTH , CV_8U , Scalar(255)) ;
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
		src[i].initialize(i);
	}
	imageData.close();
	updateBoolMap(src , boolMap);
	draw(src,canvas);
	for(int i = 0 ; i< N ; ++i)
		src[i].updateVisbleAreaRatio(canvas);
	std::cout << "==========================="<< std::endl;
	imwrite("origin.jpg" , canvas);
		for(int i = 0 ; i<N ; i++)
	{
		std::cout << src[i].LevelZeroROIBlockedRatio(canvas) <<std::endl;
	}
	int n = 0 ,qq = 0; 
	bool vFlag = false ; 
	int qqq = 0 ; 
	srand(time(NULL));
	imshow("tmp" , canvas);
	waitKey(1);	
	for(int i = 0 ; i< N ; i++)
	{
		globalRandomMove(src , i , canvas , boolMap);
	}
	for(int i = 0 ;i < N; i++)
	{
		src[i].expand(canvas , 0.7);
		draw(src , canvas);
		for(int i = 0 ; i< N ;i++)
			src[i].updateVisbleAreaRatio(canvas);
		updateBoolMap(src , boolMap);
	}
	double mininumBlankAreaRatio = 1 - maxinumVisibleAreaRatio(src , canvas , boolMap);
	mininumBlankAreaRatio = (mininumBlankAreaRatio < 0.0 ) ? 0.0 : mininumBlankAreaRatio ; 
	//std::cout<<"mininumBlankAreaRatio = "<< mininumBlankAreaRatio <<std::endl;
	//std::cout<<"globalBlankAreaRatio(boolMap) = "<< globalBlankAreaRatio(boolMap) <<std::endl;
	double thresholdMin =0.2 ,thresholdG = 0.3; 
	if(N >=5 && N < 10)
	{
		thresholdG = 0.11;
		 thresholdMin =0;
	}
	else if(N >=10)
	{
		thresholdG = 0.02;
		thresholdMin =0;
	}
	int stop = 0 ;
	while( ( mininumBlankAreaRatio > thresholdMin || globalBlankAreaRatio(boolMap) > thresholdG )    )
	{
		std::cout <<"stop = "<<stop++ << std::endl;
		if(stop >=5)
			break;
		qqq = 0 ;
		vFlag = false;
		while(  vFlag ==false && qqq < 2 )
		{
			qqq++ ;
			vFlag = true;
			for(int i = 0 ; i< N ; i++)
			{
				if(src[i].LevelZeroROIBlockedRatio(canvas) > 0.2) //   && 
				{
					if(globalRandomMove(src , i , canvas , boolMap))
						localMove(src , canvas ,boolMap ,i , 0);
					
					for(int j = 0 ; j< N; j++)
							if(j!=i)
							{
									Rect R = overlappedArea(src[i].levelZeroRectinCanvas, src[j].levelZeroRectinCanvas);
									if( (R.width*R.height)/(src[i].levelZeroRectinCanvas.height*src[i].levelZeroRectinCanvas.width) > 0.4
									   || (R.width*R.height)/(src[j].levelZeroRectinCanvas.height*src[j].levelZeroRectinCanvas.width) > 0.4)
									{
										swapEmotionROI(src[i], src[j]);
										localMove(src , canvas ,boolMap ,i , 2);
										std::cout << "swap" << std::endl;
										system("pause");
										break;
									}
								
							}
					vFlag =false;
				}
				else
				{
					localMove(src , canvas ,boolMap ,i , 1);
				}
			}
			
			if(vFlag == true)
				break;
			draw(src , canvas);
			for(int i = 0 ; i< N ;i++)
				src[i].updateVisbleAreaRatio(canvas);
			updateBoolMap(src , boolMap);
			std::cout <<"qqq = " <<qqq<<std::endl;
			std::cout<<"mininumBlankAreaRatio = "<< mininumBlankAreaRatio <<std::endl;
			std::cout<<"globalBlankAreaRatio(boolMap) = "<< globalBlankAreaRatio(boolMap) <<std::endl;
			
			for(int i = 0 ; i<N ; i++)
				if(src[i].LevelZeroROIBlockedRatio(canvas) > 0.1) 
					vFlag =false;
			if(vFlag == true)
				break;
		}
		if(mininumBlankAreaRatio > thresholdMin )
		for(int i = 0 ;i < N; i++)
		{
			if(src[i].level < (60-2*N) /4 )
			src[i].expand(canvas , 0.7);
			else if(i == 0)
				src[i].expand(canvas , 0.7);
			draw(src , canvas);
			for(int i = 0 ; i< N ;i++)
				src[i].updateVisbleAreaRatio(canvas);
			updateBoolMap(src , boolMap);
			mininumBlankAreaRatio = 1 - maxinumVisibleAreaRatio(src , canvas , boolMap);
			mininumBlankAreaRatio = (mininumBlankAreaRatio < thresholdMin ) ? thresholdMin : mininumBlankAreaRatio ; 
			std::cout<<"mininumBlankAreaRatio = "<< mininumBlankAreaRatio <<std::endl;
			std::cout<<"globalBlankAreaRatio(boolMap) = "<< globalBlankAreaRatio(boolMap) <<std::endl;
			if(mininumBlankAreaRatio <= thresholdMin )
				break;
		}
	}	

	/*for(int i = 0 ; i< N ; i++)
		if(src[i].LevelZeroROIBlockedRatio(canvas) > 0.1) //   && 
			for(int j = i+1 ; j< N; j++)
				if(j!=i)
				{
					if(switchable(src , canvas ,boolMap , i ,j ))
					{
						system("pause");
						break;
					}
				}*/

	/*for(int i = 0 ; i< N ; i++)
	{
		localMove(src , canvas ,boolMap ,i);
		imshow("tmp" , canvas);
		waitKey(1);
		out << canvas ;
	}*/
	draw(src , canvas);
	for(int i = 0 ; i< N ;i++)
		src[i].updateVisbleAreaRatio(canvas);
	updateBoolMap(src , boolMap);
	mininumBlankAreaRatio = 1 - maxinumVisibleAreaRatio(src , canvas , boolMap);
	mininumBlankAreaRatio = (mininumBlankAreaRatio < 0 ) ? 0 : mininumBlankAreaRatio ; 
		
	
	std::cout <<"-------------" <<std::endl;
	for(int i = 0 ; i<N ; i++)
	{
		std::cout << src[i].LevelZeroROIBlockedRatio(canvas) <<std::endl;
	}
	std::cout << "after : blank area ratio "<< globalBlankAreaRatio(boolMap)<< std::endl;
	
	
	for(int i = 0 ; i< N ; i++)
		if(src[i].LevelZeroROIBlockedRatio(canvas) > 0.1) //   && 
			for(int j = i+1 ; j< N; j++)
				{
					if(switchable(src , canvas ,boolMap , i ,j ))
					{
						std::cout << i <<std::endl;
						std::cout << j <<std::endl;
						break;
					}
				}
	//finalextension(src , canvas);
	imshow("result" , canvas);
	imshow("result_boolean" , boolMap);
	imwrite("result.jpg" , canvas);
	t = clock() - t ; 
	std::cout<< t <<std::endl;
	waitKey(0);
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