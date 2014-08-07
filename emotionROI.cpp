#include "emotionROI.h"

double emotionData::emotionROI_ratio(Mat stimuiusMap,int min_x, int min_y, int max_x, int max_y)
{
	double output=0, stimuiusMap_score=0, cropROI_score=0;
    int nr= stimuiusMap.rows; // number of rows  
    for (int j=0; j<nr; j++) {  
          uchar* data= stimuiusMap.ptr<uchar>(j);  
          for (int i=0; i<stimuiusMap.cols * stimuiusMap.channels(); i++) {  
				stimuiusMap_score+= data[i];
					if(i>=min_x && i<=max_x && j<=max_y && j>=min_y)
							cropROI_score += data[i];
            } // end of row                   
      }  
		output = (double)cropROI_score/stimuiusMap_score;
		return output;
}
void emotionData::updateCandidateROI()
{
	Mat a;
	for(int i =255; i>0;i = i-15)
	{
		Mat c(originImage.rows , originImage.cols , CV_8U ,Scalar(0));
		threshold(stimulusMap,a,i,255,0);
	
		Canny(a , a ,10,20);
		vector<Vec4i> lines;
		vector<int> px ,py;
		HoughLinesP(a, lines, 1, CV_PI/180, 50, 50, 10 );
		if(lines.size()>0)
		{
			for(int j = 0 ; j<lines.size();++j)
			{
				Vec4i l = lines[j];
				px.push_back(l[0]);
				px.push_back(l[2]);
				py.push_back(l[1]);
				py.push_back(l[3]);
			}
			std::sort(px.begin(),px.end());
			std::sort(py.begin(),py.end());
			int xmin = px.front() , ymin = py.front() , xmax = px.back() , ymax = py.back();
			Rect crop(xmin , ymin , xmax- xmin , ymax-ymin);
			if(emotionROI_ratio(stimulusMap ,xmin , ymin , xmax ,ymax) > 0.5)
			{
				candidateROI.push_back(	originImage(crop).clone());
				candidateROIRect.push_back(Rect(crop));
			}
			
		}
	}
}
void emotionData::initialize(int number)
{
	this->layer = number;
	this->level = 0; // set level = 0 
	updateCandidateROI(); 	
	angle =0;
	center.x = seed.x + candidateROI[level].cols / 2 ;
	center.y = seed.y + candidateROI[level].rows / 2 ;
	rotate(angle);
	seed.x = center.x - bigRotationROI.cols /2 ; 
	seed.y = center.y - bigRotationROI.rows /2; 
	updateCurrentRect(); // choose candidateROI[layer] 
	updateCorners();  // update the remaining three corners 

} // revise
void emotionData::updateVisbleAreaRatio(int index , Mat& boolMap)
{
	
	Rect ShowOnSmallCanvas = boundingRect + Point(seed.x , seed.y); 
	if(ShowOnSmallCanvas.x< DIFF_X)
	{
		ShowOnSmallCanvas.width = ShowOnSmallCanvas.x  + ShowOnSmallCanvas.width - DIFF_X ; 
		ShowOnSmallCanvas.x  = DIFF_X;
	}
	if(ShowOnSmallCanvas.y< DIFF_Y)
	{
		ShowOnSmallCanvas.height = ShowOnSmallCanvas.y  + ShowOnSmallCanvas.height - DIFF_Y ; 
		ShowOnSmallCanvas.x  = DIFF_Y;
	}
	
	if(ShowOnSmallCanvas.x + ShowOnSmallCanvas .width > CANVAS_WIDTH + DIFF_X)
		ShowOnSmallCanvas.width = CANVAS_WIDTH + DIFF_X - ShowOnSmallCanvas.x;
	if(ShowOnSmallCanvas.y + ShowOnSmallCanvas .height > CANVAS_HEIGHT + DIFF_Y)
		ShowOnSmallCanvas.height = CANVAS_HEIGHT + DIFF_Y - ShowOnSmallCanvas.y;
	
	
	if(ShowOnSmallCanvas.height == 0 || ShowOnSmallCanvas.width ==0)
		visbleAreaRatio =0;
	else
	{
		
		int score = 0;
		Mat tmp = boolMap(ShowOnSmallCanvas); // Scalar((order[i]+1)*10,(order[i]+1)*10 ,(order[i]+1)*10)
		int nr = tmp.rows;
		uchar* data;
		int xx = tmp.cols * tmp.channels();
		for(int i = 0 ; i< nr; i++)
		{
			data = tmp.ptr<uchar>(i);
			for(int j = 0 ; j<xx ; j =j+ tmp.channels() )
				if(data[j] == (index+1)*10)
				{
					score++;
				}
		}
		visbleAreaRatio = (double)score / (candidateROI[level].cols * candidateROI[level].rows);
		visbleAreaRatio = (visbleAreaRatio> 1) ? 1 : visbleAreaRatio ; 
	}
}
void emotionData::updateCorners()
{
	corners[1] = Point(corners[0].x + currentRect.width ,corners[0].y); 
	corners[2] = Point(corners[0].x  ,corners[0].y + currentRect.height); 
	corners[3] = Point(corners[0].x + currentRect.width ,corners[0].y+ currentRect.height ); 
	ShowRectInCandidateROI = Rect(corners[0].x - seed.x,corners[0].y - seed.y,currentRect.width ,currentRect.height);
	ShowRectInOriginalImage = Rect(corners[0].x - seed.x + candidateROIRect[level].x ,corners[0].y - seed.y + candidateROIRect[level].y ,currentRect.width ,currentRect.height);
	finalRectInSrc = Rect(ShowRectInOriginalImage);
	finalRectInCanvas = Rect(currentRect);
} // revise
void emotionData::updateCurrentRect()
{
	int range_x =this->bigRotationROI.cols, 
		range_y =this->bigRotationROI.rows, 
		xmin = 0, ymin = 0 ;
	if(boundingRect.x   < 0  || boundingRect.y < 0)
		std::cout <<"claude51315 QQ" <<std::endl;
	/*
		std::cout << Point(  p->seed.x-p->boundingRect.x + p->candidateROIRect[p->level].width/2 - p->bigRotationROI.cols/2 ,
								 p->seed.y-p->boundingRect.y + p->candidateROIRect[p->level].height/2 - p->bigRotationROI.rows/2 ) <<std::endl;
		std::cout << p->seed <<std::endl;
	*/
	

	corners[0].x = seed.x;
	corners[0].y = seed.y;
	
	if(corners[0].x < DIFF_X)
	{
		range_x =   corners[0].x + bigRotationROI.cols - DIFF_X;
		corners[0].x = DIFF_X;
	}
	if(corners[0].y < DIFF_Y)
	{
		
		range_y =   corners[0].y + bigRotationROI.rows - DIFF_Y;
		corners[0].y = DIFF_Y;
	}
	if(corners[0].x+bigRotationROI.cols >= CANVAS_WIDTH + DIFF_X -1 )
	{
		
		range_x = CANVAS_WIDTH + DIFF_X -1-corners[0].x;
	}
	if(corners[0].y + bigRotationROI.rows >= CANVAS_HEIGHT + DIFF_Y  -1)
	{
		
		range_y = CANVAS_HEIGHT + DIFF_Y -1-corners[0].y;
	}
	
	currentRect = Rect(corners[0].x ,corners[0].y , range_x , range_y);
	
}	// revise
void emotionData::expand()
{
	if(level != candidateROI.size()-1)
	{
		level ++;
		rotateWithUpdate(angle);
		//	std::cout<<"expand" << std::endl;
	}
	else
		std::cout<<"largest level" << std::endl;
}
double emotionData::currentRectAndCandidateROIAreaRatio() // k
{
	return (double)(currentRect.width*currentRect.height)/(double)(candidateROIRect[level].width*candidateROIRect[level].height) ;
}
Rect emotionData::getAdjacentBlankArea( int side)
{
	
	switch(side)
	{
	case upAdjacent:
		{
			Rect tmpRect  = currentRect + Point(0 , - candidateROI[level].rows);
			if(tmpRect.y < 0 + DIFF_Y)
			{
				tmpRect.height = currentRect.y - DIFF_Y;
				tmpRect.y = DIFF_Y;
			}
			return tmpRect;
		}
		break;
		case downAdjacent:
		{
		
			Rect tmpRect  = currentRect + Point(0 , candidateROI[level].rows);
			if(tmpRect.y  + tmpRect.height >= CANVAS_HEIGHT + DIFF_Y -1)
			{
				tmpRect.height =    CANVAS_HEIGHT + DIFF_Y -1 - tmpRect.y ;
			}
			return tmpRect;
			break;
		}
		case rightAdjacent:
		{
			Rect tmpRect  = currentRect + Point(  candidateROI[level].cols , 0);
			if(tmpRect.x  + tmpRect.width >= CANVAS_WIDTH + DIFF_X-1)
			{
				tmpRect.width =    CANVAS_WIDTH + DIFF_X-1 - tmpRect.x;
				
			}
			return tmpRect;
			break;
		}
		case leftAdjacent:
		{
			Rect tmpRect  = currentRect + Point(- candidateROI[level].cols , 0);
			if(tmpRect.x <=  0 + DIFF_X)
			{
				tmpRect.width = currentRect.x - DIFF_X;
				tmpRect.x = DIFF_X;
			}
			return tmpRect;
			break;
		}
	}

}
bool emotionData::move(int deltaX , int deltaY)
{
	if( !( (deltaY + this->seed.y >= CANVAS_HEIGHT + DIFF_Y + DIFF_Y / 2  -1) || 
			(deltaX + this->seed.x >= CANVAS_WIDTH + DIFF_X + DIFF_X/2 -1)|| 
			(deltaY + this->seed.y + bigRotationROI.rows <= DIFF_Y/2) ||
			(deltaX + this->seed.x + bigRotationROI.cols <= DIFF_X/2)
		)
	)
	{
		this->seed.x = this->seed.x + deltaX ; 
		this->seed.y = this->seed.y + deltaY ;
		this->center.x = this->center.x + deltaX ; 
		this->center.y = this->center.y + deltaY ;
		updateCurrentRect();
		updateCorners();
		return 1;
	}
	return 0;
}
double emotionData::localBlockedAreaRatio(Mat& canvas , int mlevel) //t    qqqqq
{
	/*counting ratio with respect to area*/
		
		double output =0 , blocked_area = 0;
		
		Mat covered_current = canvas(currentRect).clone();
		Rect showRect(corners[0].x - seed.x,
						  corners[0].y - seed.y,
						  currentRect.width ,
						  currentRect.height);	
		
		Mat current = candidateROI[mlevel](showRect).clone();
		
		int nr= current.rows; // number of rows  
		  for (int j=0; j<nr; j++) {  
			  uchar* data_covered= covered_current.ptr<uchar>(j);  
			  uchar* data= current.ptr<uchar>(j);  
			  for (int i=0; i<current.cols * current.channels(); i=i+current.channels()) {  
				if(data[i] != data_covered[i])
					blocked_area++;
				} // end of row                   
		  }  

		if((double)(this->currentRect.height*this->currentRect.width) == 0)
			output =0;
		else
			output = blocked_area/(double)(this->currentRect.height*this->currentRect.width);
		
		return output;

}  
//qqq
double emotionData::blockedEmotionROIRatio(Mat canvas)
{
	double output =0 , blockedEmotionROI = 0 , totalEmotionROI=0 ;

	Mat temp =  stimulusMap(candidateROIRect[level]).clone();
	
	Rect showRect(corners[0].x - seed.x,
						  corners[0].y - seed.y,
						  currentRect.width ,
						  currentRect.height);	
	
	Mat currentstimulusMap = temp( showRect).clone();
	Mat covered_current = canvas( Rect(currentRect)).clone();
	Mat current = candidateROI[level](showRect).clone();
	
	
	int nr= currentstimulusMap.rows; // number of rows  
	
		  for (int j=0; j<nr; j++) {  
			  uchar* data_covered= covered_current.ptr<uchar>(j);  
			  uchar* data = current.ptr<uchar>(j); 
			  uchar* dataStimulus = currentstimulusMap.ptr<uchar>(j); 
			  for (int i=0; i<currentstimulusMap.cols * currentstimulusMap.channels(); i++)
				{  
				 totalEmotionROI += (dataStimulus[i] * dataStimulus[i] );
					if(data[i*3] != data_covered[i*3])
							blockedEmotionROI += ((int)dataStimulus[i]) * ((int)dataStimulus[i]);
				} // end of row                   
		  }  
	
		  	output = (double)blockedEmotionROI /totalEmotionROI;
			return output;



}  //qqq
//qqq
double emotionData::LevelZeroROIBlockedRatio(int index , Mat& boolMap)
{
	/*
		Mat block(fake_height   , fake_width , CV_8UC3, Scalar((order[i]+1)*10,(order[i]+1)*10 ,(order[i]+1)*10)) ;
		block.setTo(Scalar((order[i]+1)*10,(order[i]+1)*10 + 20 ,(order[i]+1)*10));   // level zero 
	*/
	Mat tmp = boolMap(currentRect);
	int nr = tmp.rows;
	double score;
	int value = (index+1) * 10 +20 ; 
	int cols_channels = tmp.cols * tmp.channels();
	for(int i = 0 ; i< nr ;i++)
	{
		uchar* data = tmp.ptr<uchar>(i);
		for(int j= 1 ; j<cols_channels ; j +=tmp.channels() )
			if(data[j] == value)
				score ++;
	}
	double ans = score / (candidateROI[0].cols *candidateROI[0].rows) ;
	ans = (ans > 1 )? 1:ans;
	return 1 - ans ; 
} //qq
//qqq
void emotionData::rotate(double newAngle)
{
	
	angle = newAngle;
	
	Mat roi = candidateROI[level].clone();
//	std::cout << "roi .width " << roi.cols <<std::endl;
//	std::cout << "roi .height " << roi.rows <<std::endl;
	int h1 = roi.rows;
	int w1 = roi.cols;
	int h2 =  ceil( ceil( h1*cos(abs(angle)*PI/180))+ ceil (w1*sin(abs(angle)*PI/180))) + 2 ;
	int w2 =  ceil( ceil( h1*sin(abs(angle)*PI/180))+ ceil (w1*cos(abs(angle)*PI/180))) + 2 ;
	int range_x = std::max(w1,w2);
	int range_y = std::max(h1,h2);
	Mat tmp(Size(  range_x , range_y ) ,CV_8UC3 , Scalar(0,0,0));
	//std::cout << "tmp .width " << tmp.cols <<std::endl;
	//std::cout << "tmp .height " << tmp.rows <<std::endl;
	roi.copyTo(tmp(Rect((tmp.cols-roi.cols)/2,(tmp.rows-roi.rows)/2,roi.cols,roi.rows)));

	Point2f  pt(tmp.cols/2, tmp.rows/2);
	Mat r = cv::getRotationMatrix2D(pt , angle , 1);
	warpAffine(tmp, tmp, r, tmp.size());

	//std::cout << "tmp .width " << tmp.cols <<std::endl;
	//std::cout << "tmp .height " << tmp.rows <<std::endl;
	
	Rect www  = candidateROIRect[level];
	//std::cout << "www = "<<www <<std::endl;
	RotatedRect rRect(Point2f(  tmp.cols /2   , tmp.rows /2  ) , Size(www.width , www.height) ,-angle);
	Rect brect = rRect.boundingRect();
	Mat tmpmask = Mat::zeros(tmp.rows , tmp.cols, CV_8UC1);
	Point2f verticesqq[4]; // convert point to point2f
	rRect.points(verticesqq);
	Point vertices[4];
	vector<Point> contour;
	for( int j = 0; j < 4; j++ )
	{
		vertices[j] = verticesqq[j];
	}
	
	fillConvexPoly(tmpmask , vertices , 4 ,Scalar(255));
	mask = tmpmask.clone();
	boundingRect = brect;
	bigRotationROI = tmp.clone();
	if(level == 0)
	{
		levelZeroMask = tmpmask.clone();
		levelZeroBoundingRect = brect;
		levelZeroBigRotationROI = tmp.clone();
	}
	
}
// rotate
void emotionData::rotateWithUpdate(double newAngle)
{
	rotate(newAngle);  // upDate bigRotationROI , Mask , boundingRect
	seed.x = center.x - bigRotationROI.cols /2 ; 
	seed.y = center.y - bigRotationROI.rows /2; 
	updateCurrentRect(); 
	updateCorners();  
}
void switchLayer(emotionData& src1, emotionData& src2)
{
	std::swap(src1.layer,src2.layer);
}
bool readImage( std::fstream& emotionFiles, emotionData& output , int number) 
{
	string text , emotionString , fileString = "Emotion6/" , imageString;
	// fileString =  "Emotion6/" + emotionString + imageString
	emotionROI data[15];
	std::size_t x;
	int ROI_xmin,ROI_xmax, ROI_ymin, ROI_ymax ;
	bool tmp2 , init = false ;
	
	int n = 0;
	Mat src;
	while(n <= 15)
	{
		emotionFiles >> text;
		
		//std::cout<<text<<std::endl;

		x = text.find_first_of('/');
		emotionString = text.substr(0,x);
		imageString = text.substr(x,text.size());
		fileString = "Emotion6/";
		fileString +=emotionString;
		fileString += imageString;
		if(n==0)
		{

			src = imread(fileString,1);
			output.originImage  = src.clone();
			output.layer = number;
		}	
		emotionFiles >> ROI_xmin >> ROI_xmax >> ROI_ymin >> ROI_ymax;
		// foo
		if(ROI_xmin == ROI_xmax)
			ROI_xmax = ROI_xmax +1;
		if(ROI_ymin == ROI_ymax)
			ROI_ymax = ROI_ymax +1;
		if(ROI_xmin < 0 )
			ROI_xmin = 0;
		if(ROI_xmax > src.cols )
			ROI_xmax = src.cols-1;
		if(ROI_ymin < -1)
			ROI_ymin = 0;
		if(ROI_ymax > src.rows )
			ROI_ymax = src.rows-1;
		
		data[n].emotionRect = Rect(ROI_xmin , ROI_ymin ,ROI_xmax - ROI_xmin , ROI_ymax-ROI_ymin ) ;
		for(int j = 0;j<7;++j)
		{
			emotionFiles >> tmp2;
			data[n].emotionCategory[j] = tmp2;
		}
		n++;
		text.clear();

		if(n == 15)  // making stimulus map 
		{
			
			//std::cout<<" read data : "<<t<<std::endl;

			//int emotionkeywd = findKeyword(emotionString);
			Mat  tmpROI,binary (src.rows , src.cols , CV_8U ,Scalar(0)),a;
			for(int i = 0 ;i<15;i++)
			{
				tmpROI.create(data[i].emotionRect.height , data[i].emotionRect.width , CV_8U);
				tmpROI.setTo(15);
				addWeighted(binary(data[i].emotionRect) , 1 , tmpROI , 1,0,binary(data[i].emotionRect));
				//if(data[i].emotionCategory[emotionkeywrd] ==1)
				//rectangle(src , data[i].emotionRect,Scalar(0,255,0) ,1,CV_AA);
			}
			//std::cout<<" addweighted "<<t<<std::endl;
			output.stimulusMap =  binary.clone();
			return true;
		}
	}
}
void randomSeedPoint(emotionData src[], int numOfSource , int canvasWidth , int canvasHeight)
{
	srand(time(NULL));
	Mat check_src(canvasHeight,canvasWidth,CV_8U,Scalar(0));
	//check_src.setTo(0);	
	for(int i = 0; i < numOfSource ; ++i)
	{
		bool not_done = true;
		while(not_done)
		{
			
			int temp_seed_x = 0,temp_seed_y = 0;
			temp_seed_x = (int)(rand()% CANVAS_WIDTH + DIFF_X);
			temp_seed_y = (int)(rand()% CANVAS_HEIGHT + DIFF_Y); 
			if(check_src.at<uchar>(temp_seed_y,temp_seed_x)==0)
			{
				not_done =false;
				check_src.at<uchar>(temp_seed_y,temp_seed_x)=1; 
				src[i].seed.x =temp_seed_x; 
				src[i].seed.y =temp_seed_y;
			}
		}
	}
}
// qqq
double resizeRatio_x (emotionData src[], int numOfSource , int canvasWidth)
{
	double resizeRatio_x = 0 ;
	for(int i = 0; i< numOfSource; i++)
	{
		resizeRatio_x += (double)src[i].candidateROI[0].cols;
	}
	if((double)(canvasWidth/resizeRatio_x) <1)
		return (double)(canvasWidth/resizeRatio_x);
	else
		return 1;
}
double resizeRatio_y (emotionData src[], int numOfSource , int canvasHeight)
{
	double resizeRatio_y = 0;
	for(int i = 0; i< numOfSource; i++)
	{
		resizeRatio_y += (double)src[i].candidateROI[0].cols;
	}
	if((double)(canvasHeight/resizeRatio_y) <1)
		return (double)(canvasHeight/resizeRatio_y);
	else
		return 1;
}
double globalBlankAreaRatio(Mat &src)
{
	
	double output=0, src_score=0;
      int nr= src.rows; // number of rows  
	  
      for (int j=0; j<nr; j++) {  
          uchar* data= src.ptr<uchar>(j);  
          for (int i=0 ; i<(src.cols ) * src.channels(); i=i+ src.channels()) {  
			  
			  if(data[i]==0)
			  {
				src_score++;
				
			  }
			 } // end of row                   
      }  
	  
	  output = (double)src_score/(src.rows*src.cols);
	  return output;
}
void draw(emotionData *src , Mat &canvas)
{
	canvas.setTo(0);
	int *order = new int[N] ;
	for(int i = 0; i <N ; i++)
		order[i] = i;
	for(int i = 0; i <N ; i++)
	{
		for(int j = i+1; j < N ; j++)
		{
			if(src[i].layer > src[j].layer)
				std::swap(order[i],order[j]);
		}
	}

	for(int i = 0 ; i< N ;++i)
	{
		emotionData *p ;
		p = &src[order[i]];
		int dx =  p->center.x - p->bigRotationROI.cols / 2 ;
	    int dy =  p->center.y - p->bigRotationROI.rows / 2 ;
	    
		/*p->bigRotationROI(p->boundingRect).copyTo(canvas(p->boundingRect + Point(  p->seed.x-p->boundingRect.x + p->candidateROIRect[p->level].width/2 - p->bigRotationROI.cols/2 ,
								 p->seed.y-p->boundingRect.y + p->candidateROIRect[p->level].height/2 - p->bigRotationROI.rows/2 )) , p->mask(p->boundingRect));*/
		p->bigRotationROI(p->boundingRect).copyTo(canvas(p->boundingRect + Point( dx ,dy )) , p->mask(p->boundingRect));
		
		
		
	}


	/*canvas =  Mat(fake_height   , fake_width , CV_8UC3 , Scalar(0,0,0)).clone();
	int *order = new int[N] ;
	for(int i = 0; i <N ; i++)
		order[i] = i;
	for(int i = 0; i <N ; i++)
	{
		for(int j = i+1; j < N ; j++)
		{
			if(src[i].layer > src[j].layer)
				std::swap(order[i],order[j]);
		}
	}

	for(size_t i = 0 ; i< N ;++i)
	{
		int range_x =src[order[i]].currentRect.width, range_y =src[order[i]].currentRect.height;


		if(src[order[i]].ShowRectInCandidateROI.width !=0 && src[order[i]].ShowRectInCandidateROI.height!=0)
			src[order[i]].candidateROI[src[order[i]].level](src[order[i]].ShowRectInCandidateROI).copyTo(canvas(Rect(src[order[i]].currentRect)));
	}*/
}
// qqq
bool isOverlapped(Rect R1, Rect R2)
{
	Rect check = overlappedArea( R1,  R2);
	if(check.x == 0 && check.y == 0 && check.height == 0 && check.width == 0)
		return false;
	else
		return true;
} 
Rect overlappedArea(Rect R1, Rect R2)
{
		Rect intersect;
		intersect.x = (R1.x < R2.x) ? R2.x : R1.x; 
		intersect.y = (R1.y < R2.y) ? R2.y : R1.y; 
		intersect.width = (R1.x + R1.width < R2.x + R2.width) ? 
			R1.x + R1.width : R2.x + R2.width; 
		intersect.width -= intersect.x; 
		intersect.height = (R1.y + R2.height < R2.y + R2.height) ? 
			R1.y + R1.height : R2.y + R2.height; 
		intersect.height -= intersect.y;     
    
		// check for non-overlapping regions 
		if ((intersect.width <= 0) || (intersect.height <= 0)) { 
			intersect = Rect(0, 0, 0, 0); 
		} 
    
		return intersect;

}
void swapEmotionROI(emotionData &src1, emotionData &src2)
{
	int xMoveDistance = src1.corners[0].x -src2.corners[0].x ,
		yMoveDistance = src1.corners[0].y -src2.corners[0].y;
	switchLayer(src1,src2);
	src1.seed = Point(src1.seed.x - xMoveDistance, src1.seed.y - yMoveDistance );	
	src2.seed = Point(src2.seed.x + xMoveDistance, src2.seed.y + yMoveDistance );
	src1.updateCurrentRect();	
	src1.updateCorners();
	src2.updateCurrentRect();
	src2.updateCorners();
}
double totalBlockedAreaRatio(emotionData *src , Mat canvas)
{
	double  totalBlockedArea = 0; 
	for(int i =0 ; i < N; i++)
	{
		totalBlockedArea += src[i].localBlockedAreaRatio(canvas,src[i].level);
	}
	totalBlockedArea = totalBlockedArea/N ;
	return totalBlockedArea;
}
//qqq
void updateBoolMap(emotionData *src , Mat& outputBoolMap)
{
	outputBoolMap.release();
	Mat tmpBoolMap(fake_height   , fake_width , CV_8UC3 , Scalar(0)); 
	
	int *order = new int[N] ;
	for(int i = 0; i <N ; i++)
		order[i] = i;
	for(int i = 0; i <N ; i++)
	{
		for(int j = i+1; j < N ; j++)
		{
			if(src[i].layer > src[j].layer)
				std::swap(order[i],order[j]);
		}
	}
	Mat block(fake_height   , fake_width , CV_8UC3, Scalar(0,0,0)) ;
	emotionData *p ;
	int dx ,dy;
	Rect pb ;
	double value;
	
	for(int i = 0 ; i< N ;++i)
	{
		
		p = &src[order[i]];
		pb= p->boundingRect;
		dx =  p->center.x - p->bigRotationROI.cols / 2 ;
	    dy =  p->center.y - p->bigRotationROI.rows / 2 ;
		value = (order[i]+1)*10;
		block.setTo(Scalar(value,value ,value));
		//Mat block(fake_height   , fake_width , CV_8UC3, Scalar((order[i]+2)*20,0,0) );
		block(pb).copyTo(tmpBoolMap(pb + Point( dx ,dy )) , p->mask(pb));
		// deal with level zero 
		dx =  p->center.x - p->levelZeroBigRotationROI.cols / 2 ;
	    dy =  p->center.y - p->levelZeroBigRotationROI.rows / 2 ;
		block.setTo(Scalar(value,value+20 ,value));
		//block.setTo(Scalar(255,255,255));
		block(p->levelZeroBoundingRect).copyTo(tmpBoolMap(p->levelZeroBoundingRect + Point( dx ,dy )) , p->levelZeroMask(p->levelZeroBoundingRect));
	}
	
	
	outputBoolMap = tmpBoolMap;
}
bool rectIsValid(Rect tmpRect)
{
	if(tmpRect.height >0 && tmpRect.width > 0 && tmpRect.x >=DIFF_X && tmpRect.x <= CANVAS_WIDTH  + DIFF_X  && tmpRect.y >=DIFF_Y && tmpRect.y<=CANVAS_HEIGHT + DIFF_Y) 
		return true;
	else
		return false;
}
double varianceVisbleAreaRatio(emotionData *src, Mat canvas)
{
    int i, n;
    double mean= 0.0, dev= 0.0, sum = 0.0,
        sdev = 0.0;


    for (i = 1; i < N; ++i)
		sum = sum + src[i].visbleAreaRatio;
    mean = sum / N;
 
    for(i = 1; i < N; ++i){
        dev = (src[i].visbleAreaRatio - mean)*(src[i].visbleAreaRatio - mean);
        sdev = sdev + dev;
    }
 
    return sdev; 
  
}
bool globalRandomMove(emotionData *src ,int n, Mat& canvas , Mat& boolMap)
{
	Point output;
	srand(time(NULL));
	Mat temp = canvas.clone();
	temp(src[n].currentRect).setTo(0);
	int distance_x =  src[n].bigRotationROI.cols-1 ; 
	int distance_y = src[n].bigRotationROI.rows-1 ;
	int minScore_x=0, minScore_y=0;
	double Score=0.4f;
	bool goodFlag = 0;
	for(int i = DIFF_X ; i < CANVAS_WIDTH + DIFF_X; i=i+ distance_x/2)
	{
		for(int j = DIFF_Y; j < CANVAS_HEIGHT + DIFF_Y; j = j+distance_y/2)
			{	
				if(i+distance_x <temp.cols - DIFF_X && j+distance_y <temp.rows- DIFF_Y)
				if( globalBlankAreaRatio(temp(Rect(i,j,distance_x,distance_y))) >= Score)
				{
					minScore_x = i;
					minScore_y = j; 
					Score = globalBlankAreaRatio(temp(Rect(i,j,distance_x,distance_y)));
					if(Score > 0.5)
					{	
					
						break;
						
					}
				}
			}
		if(Score > 0.5)
		{	
			
			break;
			
		}
	}	
		
		for(int i = CANVAS_WIDTH+ DIFF_X -1; i >=  DIFF_X  ; i=i- distance_x/2)
		for(int j = CANVAS_HEIGHT + DIFF_Y -1 ; j >= DIFF_Y ; j = j- distance_y/2)
			{	
				if(i+distance_x <temp.cols - DIFF_X && j+distance_y <temp.rows- DIFF_Y)
				if( globalBlankAreaRatio(temp(Rect(i,j,distance_x,distance_y))) >= Score)
				{
					minScore_x = i;
					minScore_y = j; 
					Score = globalBlankAreaRatio(temp(Rect(i,j,distance_x,distance_y)));
					if(Score==1)
					{
					
						break;
					}
				}
			}
			
	output.x =  minScore_x + rand()%(distance_x/16 ); 
	output.y =  minScore_y +rand()%(distance_y/16);
	int dx = output.x-src[n].corners[0].x, dy = output.y - src[n].corners[0].y;
	
	
	if(src[n].move(dx ,dy))
		goodFlag =  1;
	else
		goodFlag = 0 ;
	updateBoolMap(src , boolMap);
	draw(src , canvas);
	for(int i = 0 ; i<N;i++)
		src[i].updateVisbleAreaRatio(i,boolMap);
	return (goodFlag) ? 1 : 0 ;

}  // revised
//qqq
double localMove(emotionData *src , Mat& canvas , Mat& boolMap , int number ,int mode)
{

	Rect canvasRect(DIFF_X , DIFF_Y , CANVAS_WIDTH , CANVAS_HEIGHT);
	emotionData *p;
	int n  = 1  , side = 0 ,dx=0 ,dy=0 ; 
	p = &src[number];
	double adjacentAreaBlankScore;
	double curGlobalBlankAreaRatio = 0 , nextGlobalBlankAreaRatio = 0;
	double curLocalVisbleAreaRatio = 0 , nextLocalVisbleAreaRatio = 0;
	double curLevelZeroBlockedROIRatio = 0 , nextLevelZeroBlockedROIRatio = 0;
	double curOtherLevelZeroBlockedROIRatioSum = 0 , nextOtherLevelZeroBlockedROIRatioSum = 0;
	//otherLevelZeroBlockedAreaSum(src ,canvas, number)
	bool goodFlag = 0 ,endFlag=0 , moveFlag = 0;
	int badCount = 0 ;
	std::cout << "number = "<<number <<std::endl;
	curGlobalBlankAreaRatio = globalBlankAreaRatio(boolMap(canvasRect));
	curLocalVisbleAreaRatio = p->visbleAreaRatio;
	curLevelZeroBlockedROIRatio = p->LevelZeroROIBlockedRatio(number ,boolMap);
	curOtherLevelZeroBlockedROIRatioSum = otherLevelZeroBlockedAreaSum(src ,canvas, number);
	
	updateBoolMap(src , boolMap);
	draw(src , canvas);
	for(int i = 0 ; i<N;i++)
		src[i].updateVisbleAreaRatio(i,canvas);
	Mat tmpCanvas =canvas.clone();
	Mat tmpBoolMap = boolMap.clone();
	while(n++ <10)
	{
		
		std::cout << n << std::endl;
		moveFlag = 0 ;
		for(int j = 0 ; j< 4 ;j++)
		{
				
				if(moveFlag)
					break;
				side =100+ rand()%4; 
						
				
				Rect tmpRect = p->getAdjacentBlankArea(side);
				
				Rect UptmpRect = p->getAdjacentBlankArea(upAdjacent);
				Rect DowntmpRect = p->getAdjacentBlankArea(downAdjacent);
				Rect RighttmpRect = p->getAdjacentBlankArea(rightAdjacent);
				Rect LefttmpRect = p->getAdjacentBlankArea(leftAdjacent);
				
				if(rectIsValid(tmpRect))
				{
					
					adjacentAreaBlankScore = adjacentAreaBlankRatio(boolMap(tmpRect));
					if(adjacentAreaBlankScore > 0.8)
					{
						dx = (tmpRect.x - p->currentRect.x) /2;
						dy = (tmpRect.y - p->currentRect.y) /2 ;
					}
					else
					{
						dx = (tmpRect.x - p->currentRect.x) / ((p->level+1));
						dy = (tmpRect.y - p->currentRect.y) / ((p->level+1)) ;
					}
					if(mode == 1 ||  mode == 2)
					{
						dx = mindx(side);
						dy = mindy(side);
					}
					if( p->move(dx ,dy ))
					{
						//std::cout << "haha" <<std::endl;
						updateBoolMap(src , boolMap);
						draw(src , canvas);
						for(int i = 0 ; i<N;i++)
							src[i].updateVisbleAreaRatio(i,canvas);
						nextGlobalBlankAreaRatio = globalBlankAreaRatio(boolMap(canvasRect));
						nextLocalVisbleAreaRatio = p->visbleAreaRatio;
						nextLevelZeroBlockedROIRatio = p->LevelZeroROIBlockedRatio(number , boolMap);
						nextOtherLevelZeroBlockedROIRatioSum = otherLevelZeroBlockedAreaSum(src ,canvas, number);
						
						/*
							std::cout<<"curGlobalBlankAreaRatio = "<<curGlobalBlankAreaRatio<<std::endl;
							std::cout<<"nextGlobalBlankAreaRatio = "<<nextGlobalBlankAreaRatio<<std::endl;
							std::cout<<"curLocalVisbleAreaRatio = "<<curLocalVisbleAreaRatio<<std::endl;
							std::cout<<"nextLocalVisbleAreaRatio = "<<nextLocalVisbleAreaRatio<<std::endl;
						*/
						if( mode == 0 && 
							nextGlobalBlankAreaRatio <= curGlobalBlankAreaRatio&& 
							nextLocalVisbleAreaRatio >= curLocalVisbleAreaRatio  &&
							nextLevelZeroBlockedROIRatio <= curLevelZeroBlockedROIRatio// &&
						//nextOtherLevelZeroBlockedROIRatioSum <= curOtherLevelZeroBlockedROIRatioSum
						  ) // good
						{
							moveFlag = 1;
							//std::cout<<"good"<<std::endl;
							
							curGlobalBlankAreaRatio = nextGlobalBlankAreaRatio;
							curLocalVisbleAreaRatio = nextLocalVisbleAreaRatio;
							curLevelZeroBlockedROIRatio =nextLevelZeroBlockedROIRatio;
							curOtherLevelZeroBlockedROIRatioSum = nextOtherLevelZeroBlockedROIRatioSum;

						}
						else if( mode == 1
							 && nextGlobalBlankAreaRatio <= curGlobalBlankAreaRatio 
							// && nextOtherLevelZeroBlockedROIRatioSum <= curOtherLevelZeroBlockedROIRatioSum
						  ) // good
						{
							//badCount = 0;
							//std::cout<<"good"<<std::endl;
							moveFlag = 1;
							
							curGlobalBlankAreaRatio = nextGlobalBlankAreaRatio;
							curLocalVisbleAreaRatio = nextLocalVisbleAreaRatio;
							curLevelZeroBlockedROIRatio =nextLevelZeroBlockedROIRatio;
							curOtherLevelZeroBlockedROIRatioSum = nextOtherLevelZeroBlockedROIRatioSum;
						}
						else if( mode == 2
							 && nextLevelZeroBlockedROIRatio <= curLevelZeroBlockedROIRatio
							// && nextOtherLevelZeroBlockedROIRatioSum <= curOtherLevelZeroBlockedROIRatioSum
						  ) // good
						{
							//badCount = 0;
							std::cout<<"good"<<std::endl;
							moveFlag = 1;
							curGlobalBlankAreaRatio = nextGlobalBlankAreaRatio;
							curLocalVisbleAreaRatio = nextLocalVisbleAreaRatio;
							curLevelZeroBlockedROIRatio =nextLevelZeroBlockedROIRatio;
							curOtherLevelZeroBlockedROIRatioSum = nextOtherLevelZeroBlockedROIRatioSum;
						}
						else
						{
							//std::cout<<"bad"<<std::endl;
							
							p->move( -dx ,-dy );
							updateBoolMap(src , boolMap);
							draw(src , canvas);
							for(int i = 0 ; i<N;i++)
								src[i].updateVisbleAreaRatio(i,canvas);
						
						}
					}
				}
			}
			
	}
	updateBoolMap(src , boolMap);
	draw(src , canvas);
	for(int i = 0 ; i<N;i++)
		src[i].updateVisbleAreaRatio(i,canvas);
	
	return globalBlankAreaRatio(boolMap(canvasRect));
}
//qqq
double adjacentAreaBlankRatio(Mat& boolMap)
{
	return globalBlankAreaRatio(boolMap);
}
int mindx(int side)
{
	switch(side)
	{
		case upAdjacent:
			{
			return 0 ; 
			break;
		}
		case downAdjacent:
		{
			return 0 ; 
			break;
		}
		case leftAdjacent:
		{
			return -15 ;
			break;
		}
		case rightAdjacent:
		{
			return 15 ;
			break;
		}
	}

}
int mindy(int side)
{
	switch(side)
	{
		case rightAdjacent:
			{
			return 0 ; 
			break;
		}
		case leftAdjacent:
		{
			return 0 ; 
			break;
		}
		case upAdjacent:
		{
			return -15 ;
			break;
		}
		case downAdjacent:
		{
			return 15 ;
			break;
		}	
	}
}
double maxinumVisibleAreaRatio(emotionData *src , Mat& canvas , Mat& boolMap)
{
	double canvasArea = CANVAS_WIDTH * CANVAS_HEIGHT; 
	double tmp = 0 ;
	emotionData *p;
	for(int i = 0 ;i<N ;  ++i)
	{
		p = &src[i];	
		tmp += ( p->candidateROI[p->level].cols * p->candidateROI[p->level].rows) ; 
	}
	return tmp / canvasArea ; 
}
double otherLevelZeroBlockedAreaSum(emotionData *src ,  Mat&canvas , int select)
{
	double ans = 0 ;
	for(int i = 0 ;i< N ;i++)
		if(i!=select)
		{
			ans += src[i].visbleAreaRatio;
		}
	return ans ; 
}
void finalextension(emotionData *src , Mat &canvas)
{
	int *order = new int[N] ;
	for(int i = 0; i <N ; i++)
		order[i] = i;
	for(int i = 0; i <N ; i++)
	{
		for(int j = i+1; j < N ; j++)
		{
			if(src[i].layer > src[j].layer)
				std::swap(order[i],order[j]);
		}
	}
	canvas.setTo(0);

	for(int i = 0; i <N ; i++) // i for pictures
	{
		emotionData *p;
		p = &src[order[i]];
		bool flag = true;
		while(flag && p->level < p->candidateROI.size()-1)
		{
						for(int k = i-1 ; k>=0 ; k --)
							if(isOverlapped(Rect(p->seed.x,p->seed.y,p->bigRotationROI.cols,p->bigRotationROI.rows),
								src[k].levelZeroBoundingRect + Point(src[k].center.x - src[k].levelZeroBigRotationROI.cols / 2,src[k].center.y - src[k].levelZeroBigRotationROI.rows / 2)))
								flag = false;
						if(flag == true)
							 p->expand();

						
		}
	}

	draw(src,canvas);

}
bool switchable(emotionData *src , Mat& canvas , Mat&boolMap,int p ,int q)
{
	emotionData *pp = &src[p] , *qq =&src[q];
	double cur_p_visibleArea = pp->visbleAreaRatio , cur_p_levelZeroBlockedArea = pp->LevelZeroROIBlockedRatio(p,canvas) ;
	double cur_q_visibleArea = qq->visbleAreaRatio , cur_q_levelZeroBlockedArea = qq->LevelZeroROIBlockedRatio(q,canvas) ;
	if(isOverlapped(src[p].currentRect , src[q].currentRect))
	{
		if(cur_p_levelZeroBlockedArea > 0.75 )
		{
			switchLayer(*pp ,*qq);
			draw(src , canvas);
			for(int i = 0 ;i< N;i++)
				src[i].updateVisbleAreaRatio(i,canvas);
			double next_p_visibleArea = pp->visbleAreaRatio , next_p_levelZeroBlockedArea = pp->LevelZeroROIBlockedRatio(p,canvas) ;
			double next_q_visibleArea = qq->visbleAreaRatio , next_q_levelZeroBlockedArea = qq->LevelZeroROIBlockedRatio(q,canvas) ;
			if(next_p_levelZeroBlockedArea <  cur_p_visibleArea )
				return 1;
			else
			{
				switchLayer(*pp ,*qq);
				draw(src , canvas);
				for(int i = 0 ;i< N;i++)
					src[i].updateVisbleAreaRatio(i,canvas);
				return 0 ;
			}
	
		}
	}
	else
		return 0 ;
}