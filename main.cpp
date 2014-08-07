#include "emotionROI.h"

double globalblockedRatio(emotionData src[]);
void main()
{

	

	time_t t = clock();
	Mat canvas(fake_height   , fake_width , CV_8UC3 , Scalar(0,0,0)),
		boolMap(fake_height   , fake_width , CV_8UC3 , Scalar(0)) , 
		block(fake_height   , fake_width , CV_8U , Scalar(255)) ;
	emotionData src[N]; 
	Rect canvasRect(DIFF_X , DIFF_Y , CANVAS_WIDTH , CANVAS_HEIGHT);

	
	std::fstream  imageData;
	imageData.open("emotionROI_value.txt", std::fstream::in);
	if(!imageData.is_open())
	{
		std::cerr<<"cannot open file."<<std::endl;
		exit(1);
	}	
	randomSeedPoint(src, N, fake_width, fake_height); // randomly choose seed point 
	for(size_t i = 0 ; i< N ;++i)
	{
		readImage( imageData , src[i],i); // origin image , stimulus map , layer
		src[i].initialize(i);	
	}
	imageData.close();
	updateBoolMap(src , boolMap);
	draw(src,canvas);
	for(int i = 0 ; i< N ; ++i)
		src[i].updateVisbleAreaRatio(i , boolMap);
	srand(time(NULL));

	int randomAngle;
	for(int i = 0 ; i< N ; ++i)
	{
		randomAngle = rand()%30 - 15;
		src[i].rotateWithUpdate(randomAngle);
	}
	draw(src,canvas);
	updateBoolMap(src , boolMap);
	/*
	imshow("sss" , canvas);
	imshow("sss2" , boolMap);
	waitKey();
	for(int i = 0 ; i< N ; ++i)
	{
		src[i].expand();
		src[i].expand();
		src[i].expand();
	}
	draw(src,canvas);
	updateBoolMap(src , boolMap);
	
	imshow("sss" , canvas);
	imshow("sss2" , boolMap);
	waitKey();
	for(int i = 0 ; i<N ; i++)
	{
		std::cout << src[i].LevelZeroROIBlockedRatio(i , boolMap) <<std::endl;
	}
	for(int i = 0 ; i<N ; i++)
	{
		globalRandomMove(src , i , canvas , boolMap);
	}
	imshow("sss" , canvas);
	imshow("sss2" , boolMap);
	waitKey();
	waitKey();
	*/
	int n = 0 ,qq = 0; 
	bool vFlag = false ; 
	int qqq = 0 ; 
	
	imshow("result_boolean" , boolMap(canvasRect));
	imshow("result" , canvas(canvasRect));
	waitKey(1);
	for(int i = 0 ; i< N ; i++)
		globalRandomMove(src , i , canvas , boolMap);
	imshow("result_boolean" , boolMap(canvasRect));
	imshow("result" , canvas(canvasRect));
	waitKey(1);
	for(int i = 0 ;i < N; i++)
	{
		src[i].expand();
		draw(src , canvas);
		for(int i = 0 ; i< N ;i++)
			src[i].updateVisbleAreaRatio(i, boolMap);
		updateBoolMap(src , boolMap);
		imshow("result_boolean" , boolMap(canvasRect));
		imshow("result" , canvas(canvasRect));
		waitKey(1);
	}
	
	double mininumBlankAreaRatio = 1 - maxinumVisibleAreaRatio(src , canvas , boolMap);
	mininumBlankAreaRatio = (mininumBlankAreaRatio < 0.0 ) ? 0.0 : mininumBlankAreaRatio ; 
	std::cout<<"mininumBlankAreaRatio = "<< mininumBlankAreaRatio <<std::endl;
	std::cout<<"globalBlankAreaRatio(boolMap) = "<< globalBlankAreaRatio(boolMap(canvasRect)) <<std::endl;
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
	while( ( mininumBlankAreaRatio > thresholdMin || globalBlankAreaRatio(boolMap(canvasRect)) > thresholdG )    )
	{
		std::cout <<"stop = "<<stop++ << std::endl;
		if(stop >=5)
			break;
		qqq = 0 ;
		vFlag = false;
		while(  vFlag ==false && qqq < 4 )
		{
			qqq++ ;
			vFlag = true;
			for(int i = 0 ; i< N ; i++)
			{
				if(src[i].LevelZeroROIBlockedRatio(i,boolMap) > 0.2) //   && 
				{
					if(globalRandomMove(src , i , canvas , boolMap))
						localMove(src , canvas ,boolMap ,i , 0);
					else
						localMove(src , canvas ,boolMap ,i , 2);
					imshow("result_boolean" , boolMap(canvasRect));
					imshow("result" , canvas(canvasRect));
					waitKey(1);
					/*for(int j = 0 ; j< N; j++)
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
								
							}*/
						
					vFlag =false;
				}
				else
				{
					localMove(src , canvas ,boolMap ,i , 1);
					imshow("result_boolean" , boolMap(canvasRect));
					imshow("result" , canvas(canvasRect));
					waitKey(1);
				}
			}

	
			if(vFlag == true)
				break;
			draw(src , canvas);
			for(int i = 0 ; i< N ;i++)
				src[i].updateVisbleAreaRatio(i,boolMap);
			updateBoolMap(src , boolMap);
			std::cout <<"qqq = " <<qqq<<std::endl;
			std::cout<<"mininumBlankAreaRatio = "<< mininumBlankAreaRatio <<std::endl;
			std::cout<<"globalBlankAreaRatio(boolMap) = "<< globalBlankAreaRatio(boolMap(canvasRect)) <<std::endl;
			
			for(int i = 0 ; i<N ; i++)
				if(src[i].LevelZeroROIBlockedRatio(i,boolMap) > 0.1) 
					vFlag =false;
			if(vFlag == true)
			{
				std::cout <<"claude51315"<<std::endl;
				break;
			}
		}
		if(mininumBlankAreaRatio > thresholdMin )
		for(int i = 0 ;i < N; i++)
		{
			if(src[i].level < (60-2*N) /4 )
			src[i].expand();
			else if(i == 0)
				src[i].expand();
			draw(src , canvas);
			for(int i = 0 ; i< N ;i++)
				src[i].updateVisbleAreaRatio(i,boolMap);
			updateBoolMap(src , boolMap);
			imshow("result_boolean" , boolMap(canvasRect));
			imshow("result" , canvas(canvasRect));
			waitKey(1);
			if(mininumBlankAreaRatio <= thresholdMin )
				break;
		}
		mininumBlankAreaRatio = 1 - maxinumVisibleAreaRatio(src , canvas , boolMap);
		mininumBlankAreaRatio = (mininumBlankAreaRatio < thresholdMin ) ? thresholdMin : mininumBlankAreaRatio ; 
		std::cout<<"mininumBlankAreaRatio = "<< mininumBlankAreaRatio <<std::endl;
		std::cout<<"globalBlankAreaRatio(boolMap) = "<< globalBlankAreaRatio(boolMap(canvasRect)) <<std::endl;
	}	
	
	/*for(int i = 0 ; i< N ; i++)
		if(src[i].LevelZeroROIBlockedRatio(i,canvas) > 0.1) //   && 
			for(int j = i+1 ; j< N; j++)
				if(j!=i)
				{
					if(switchable(src , canvas ,boolMap , i ,j ))
					{
						
						break;
					}
				}
				
	
	draw(src , canvas);
	for(int i = 0 ; i< N ;i++)
		src[i].updateVisbleAreaRatio(i,canvas);
	updateBoolMap(src , boolMap);
	mininumBlankAreaRatio = 1 - maxinumVisibleAreaRatio(src , canvas , boolMap);
	mininumBlankAreaRatio = (mininumBlankAreaRatio < 0 ) ? 0 : mininumBlankAreaRatio ; 
		
	
	std::cout <<"-------------" <<std::endl;
	for(int i = 0 ; i<N ; i++)
	{
		std::cout << src[i].LevelZeroROIBlockedRatio(i,canvas) <<std::endl;
	}
	std::cout << "after : blank area ratio "<< globalBlankAreaRatio(boolMap(canvasRect))<< std::endl;
	
	*/
	/*for(int i = 0 ; i< N ; i++)
		if(src[i].LevelZeroROIBlockedRatio(i,canvas) > 0.1) //   && 
			for(int j = i+1 ; j< N; j++)
				{
					if(switchable(src , canvas ,boolMap , i ,j ))
					{
						std::cout << i <<std::endl;
						std::cout << j <<std::endl;
						break;
					}
				}*/
	finalextension(src , canvas);
	
	imshow("result_boolean" , boolMap(canvasRect));
	imshow("result" , canvas(canvasRect));
	imwrite("result.jpg" ,  canvas(canvasRect));
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

