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

	int n =0  , randomImageNumber=0 , randomSide=0 , 
		dx=0,dy=0; 
	double currentScore , nextScore;
	emotionData *p;
	Rect tmpCurrentRect;
	draw(src , canvas);
	imwrite("before.jpg" , canvas);
	updateBoolMap(src ,boolMap);
	std::cout<< blankAreaRatio(boolMap)  <<std::endl;
	

	time_t ddd= clock();
	double qq = 0;
	for(int i = 0 ;i< N ;i++)
		qq += (src[i].candidateROI[src[i].layer].cols * src[i].candidateROI[src[i].layer].rows);
	qq = qq / (CANVAS_HEIGHT * CANVAS_WIDTH);
	std::cout<< qq  <<std::endl;
	
	while( n <20000)
	{
		randomImageNumber = rand()% N ; 
		randomSide = rand()%4 + 100 ;
		p = &src[randomImageNumber];
		Rect tmpRect = p->getAdjacentBlankArea(boolMap , randomSide);
		currentScore  = blankAreaRatio(boolMap) ;
		if(abs(currentScore -qq) < 0.08)
			break;
		if(rectIsValid(tmpRect))
		{
			
			dx = tmpRect.x - p->seed.x;
			dy = tmpRect.y - p->seed.y;
			p->move(canvas , dx ,dy );
			updateBoolMap(src , boolMap);
			nextScore = blankAreaRatio(boolMap);
			
			if(nextScore >= currentScore)  // not good 
			{
				p->move(canvas , -dx ,-dy );
				updateBoolMap(src , boolMap);
				//std::cout<< "qq" <<std::endl;
			}
			else // good 
			{
				
				
			}
		}
		n++;
	}
	ddd =  clock() -ddd;
	std::cout<<ddd<<std::endl;
	std::cout<<n<<std::endl;
	draw(src , canvas);
	imwrite("after.jpg" , canvas);
	std::cout<< blankAreaRatio(boolMap)  <<std::endl;

	system("pause");
}
// 782649