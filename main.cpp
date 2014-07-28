#include "emotionROI.h"

void main()
{
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

	srand(time(NULL));

	int  randomImageNumber=0 , randomSide=0 , 
		dx=0,dy=0; 
	double canvasCurrentBlankAreaScore , canvasNextBlankAreaScore; // global 

	
	
	updateBoolMap(src ,boolMap);
	draw(src , canvas);
	for(int i = 0 ; i< N ; ++i)
	{
		src[i].updateVisbleAreaRatio(canvas);
		
	}
	int order[N];
	for(int i = 0 ; i< N ;i++)
		order[i] = i; 
	for(int i = 0 ; i< N ;i++)
		for(int j = i+ 1 ; j < N ; j++)
		{
			if(src[i].visbleAreaRatio > src[j].visbleAreaRatio)
				std::swap(order[i] , order[j]);
		}
	
	imshow("before" , canvas);
	waitKey(0);
	double qq = 0;
	for(int i = 0 ;i< N ;i++)
		qq += (src[i].candidateROI[src[i].layer].cols * src[i].candidateROI[src[i].layer].rows);
	qq = qq / (CANVAS_HEIGHT * CANVAS_WIDTH);
	
	
	emotionData *p;
	Rect tmpCurrentRect;
	int n = 0 , pick = 0; 
	bool goodFlag = 0  ,endFlag = 0 ;;
	while( n <1000)
	{

		for(int i= 0 ;i< N ; i++)
		{
			for(int j = 0 ; j < 4 ; j++)
			{
				randomImageNumber = order[i];
				randomSide = j + 100 ;
				p = &src[randomImageNumber];
				Rect tmpRect = p->getAdjacentBlankArea(boolMap , randomSide);
				canvasCurrentBlankAreaScore  = globalBlankAreaRatio(boolMap) ;
		
				if(abs(canvasCurrentBlankAreaScore -qq) < 0.08  )
				{
					endFlag = 1;
					break;
				}
				if(rectIsValid(tmpRect))
				{
			
					dx = tmpRect.x - p->seed.x;
					dy = tmpRect.y - p->seed.y;
					p->move(canvas , dx ,dy );
					updateBoolMap(src , boolMap);
					canvasNextBlankAreaScore = globalBlankAreaRatio(boolMap);
					if(canvasNextBlankAreaScore >= canvasCurrentBlankAreaScore  )  // not good 
					{
						p->move(canvas , -dx ,-dy );
						updateBoolMap(src , boolMap);
						//std::cout<< "qq" <<std::endl;
					}
					else  // good 
					{
						draw(src , canvas);
						for(int i = 0 ; i< N ; ++i)
							src[i].updateVisbleAreaRatio(canvas);
						for(int i = 0 ; i< N ;i++)
							for(int j = i+ 1 ; j < N ; j++)
							{
								if(src[i].visbleAreaRatio > src[j].visbleAreaRatio)
								std::swap(order[i] , order[j]);
							}
						goodFlag = 1 ; 
					}
					if(goodFlag)
						break;
					if(endFlag)
						break;
				}
				if(goodFlag)
						break;
				if(endFlag)
						break;
			}
			if(goodFlag)
			{
				std::cout<<"good"<<std::endl;
				break;
			}
			if(endFlag)
						break;
		}
		goodFlag = 0 ;
		n++;
		std::cout<<n<<std::endl;
		if(endFlag)
			break;
	}
	std::cout<<"done"<<std::endl;
	draw(src , canvas);
	imshow("after" , canvas);
	waitKey(0);
}