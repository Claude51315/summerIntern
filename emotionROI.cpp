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
	angle = 0;
	rotate(angle);
	updateCurrentRect(); // choose candidateROI[layer] 
	updateCorners();  // update the remaining three corners 

} // revise
void emotionData::updateVisbleAreaRatio(Mat& canvas)
{

	double k = currentRectAndCandidateROIAreaRatio();
	double t = localBlockedAreaRatio(canvas,level);
	visbleAreaRatio = k * (1-t);
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
	int range_x =this->candidateROI[this->level].cols, 
		range_y =this->candidateROI[this->level].rows, 
		xmin = 0, ymin = 0 ;
	
	corners[0].x = seed.x ; 
	corners[0].y = seed.y;
	if(corners[0].x < 0)
	{
		range_x = candidateROI[level].cols + corners[0].x;
		corners[0].x = 0;
	}
	if(corners[0].y < 0)
	{
		range_y = candidateROI[level].rows + corners[0].y;
		corners[0].y = 0;
	}
	if(corners[0].x+candidateROI[level].cols >= CANVAS_WIDTH + DIFF_X -1 )
		range_x = CANVAS_WIDTH + DIFF_X -1-corners[0].x;
	if(corners[0].y + candidateROI[level].rows >= CANVAS_HEIGHT + DIFF_Y  -1)
			range_y = CANVAS_HEIGHT + DIFF_Y -1-corners[0].y;
	
	currentRect = Rect(corners[0].x ,corners[0].y , range_x , range_y);
	
}	// revise
void emotionData::expand(Mat canvas, double thresholdOfNearBlankArea)
{
	if(level != candidateROI.size()-1)
	{

		canvas(currentRect).setTo(0);
		int center_x , center_y , newcorner_x , newcorner_y , range_x , range_y;
		center_x = corners[0].x + currentRect.width/2 ;
		center_y = corners[0].y + currentRect.height/2 ;
		newcorner_x = ((center_x - candidateROI[level+1].cols/2) > 0 ) ? (center_x - candidateROI[level+1].cols/2) :0 ;
		newcorner_y = ((center_y - candidateROI[level+1].rows/2) > 0 ) ? (center_y - candidateROI[level+1].rows/2) :0 ;
		range_x =  ((newcorner_x + candidateROI[level+1].cols) < canvas.cols ) ? candidateROI[level+1].cols :(canvas.cols-1-newcorner_x);
		range_y =  ((newcorner_y + candidateROI[level+1].rows) < canvas.rows ) ? candidateROI[level+1].rows :(canvas.rows-1-newcorner_y);
		rectangle( canvas,
           Point( newcorner_x, newcorner_y),
           Point( newcorner_x+range_x, newcorner_y+range_y),
           Scalar( 255, 255, 255 ),3);
		
		std::cout <<globalBlankAreaRatio( canvas(Rect(newcorner_x,newcorner_y,range_x,range_y))) << std::endl;
		if(globalBlankAreaRatio( canvas(Rect(newcorner_x,newcorner_y,range_x,range_y))) > thresholdOfNearBlankArea)
		{
			level ++;
			seed.x = seed.x + newcorner_x - corners[0].x ;
			seed.y = seed.y + newcorner_y - corners[0].y ;
			updateCurrentRect();
			updateCorners();
		//	std::cout<<"expand" << std::endl;
		}
	

	}
	else
		std::cout<<"largest level" << std::endl;
}
double emotionData::currentRectAndCandidateROIAreaRatio() // k
{
	return (double)(currentRect.width*currentRect.height)/(double)(candidateROIRect[level].width*candidateROIRect[level].height) ;
}
Rect emotionData::getAdjacentBlankArea(Mat& boolMap , int side)
{
	Mat tmp;
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
bool emotionData::move(Mat &canvas, int deltaX , int deltaY)
{
	if( !( (deltaY + this->seed.y >= CANVAS_HEIGHT + DIFF_Y-1) || 
			(deltaX + this->seed.x >= CANVAS_WIDTH + DIFF_X-1)|| 
			(deltaY + this->seed.y + candidateROI[this->level].rows <= DIFF_Y) ||
			(deltaX + this->seed.x + candidateROI[this->level].cols <= DIFF_X)
		)
	)
	{
		this->seed.x = this->seed.x + deltaX ; 
		this->seed.y = this->seed.y + deltaY ;
		updateCurrentRect();
		updateCorners();
		return 1;
	}
	updateCurrentRect();
	updateCorners();
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
double emotionData::LevelZeroROIBlockedRatio(Mat& canvas)
{
	//std::cout <<"here in "<<std::endl;
	double blockedArea , output = 1;
	Mat tmpLevelzero = candidateROI[0];
	int tmpWidth = tmpLevelzero.cols , tmpHeight = tmpLevelzero.rows;
	Mat tmpOnCanvas(tmpHeight , tmpWidth , CV_8UC3 , Scalar(0,0,0));
	Rect showOnCanvas;
	int tmpx = candidateROIRect[0].x  - candidateROIRect[level].x , 
		tmpy = candidateROIRect[0].y  - candidateROIRect[level].y ;
	int lx = seed.x + tmpx ,ly = seed.y + tmpy;
	int XonCandidateROI =0  ,YonCandidateROI =0 , widthOnCandidateROI = tmpWidth  , heightOnCandidateROI = tmpHeight;
	if(lx < DIFF_X)
		XonCandidateROI = DIFF_X  -lx ; 
	if(ly < DIFF_Y)
		YonCandidateROI = DIFF_Y -ly ; 
	if(lx +  tmpWidth > CANVAS_WIDTH + DIFF_X)
		widthOnCandidateROI = CANVAS_WIDTH + DIFF_X- lx -1 ;
	if(ly + tmpHeight > CANVAS_HEIGHT + DIFF_Y)
		heightOnCandidateROI = CANVAS_HEIGHT + DIFF_Y- ly -1  ;
	lx = (lx < 0) ? 0 : lx;
	ly = (ly < 0) ? 0 : ly;
	this->levelZeroRectinCanvas = Rect(lx,ly,widthOnCandidateROI,heightOnCandidateROI);
	
	if(widthOnCandidateROI <= 0 || heightOnCandidateROI <= 0 || lx > CANVAS_WIDTH +DIFF_X   || ly > CANVAS_HEIGHT  + DIFF_Y)
	{
		
		return 1 ; 
	}
	showOnCanvas =Rect(lx , ly , widthOnCandidateROI , heightOnCandidateROI);
	
	if(0 <= XonCandidateROI && 0 <= widthOnCandidateROI &&XonCandidateROI + widthOnCandidateROI <=tmpWidth && 0 <= heightOnCandidateROI && 0 <= heightOnCandidateROI  && YonCandidateROI + heightOnCandidateROI <= tmpHeight)
		canvas(showOnCanvas).copyTo(tmpOnCanvas(Rect( XonCandidateROI,YonCandidateROI,widthOnCandidateROI,heightOnCandidateROI)));
	else
	{
		
		return 1;
	}
	
	int nr= tmpOnCanvas.rows; // number of rows  
	double blockArea = 0 ;
		  for (int j=0; j<nr; j++) {  
			  uchar* data_original= tmpLevelzero.ptr<uchar>(j);  
			  uchar* data_onCanvas= tmpOnCanvas.ptr<uchar>(j);  
			  for (int i=0; i<tmpOnCanvas.cols * tmpOnCanvas.channels(); i=i+tmpOnCanvas.channels()) {  
				if(data_original[i] != data_onCanvas[i])
					blockArea++;
				} // end of row                   
		  }
	output = blockArea / (tmpWidth * tmpHeight);
	return output;
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

}
// rotate
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
	    p->bigRotationROI(p->boundingRect).copyTo(canvas(p->boundingRect + Point(  p->seed.x-p->boundingRect.x + p->candidateROIRect[p->level].width/2 - p->bigRotationROI.cols/2 ,
								 p->seed.y-p->boundingRect.y + p->candidateROIRect[p->level].height/2 - p->bigRotationROI.rows/2 )) , p->mask(p->boundingRect));
		
		
		
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
	Mat tmpBoolMap(fake_height   , fake_width , CV_8U , Scalar(0)) , 
	block(fake_height   , fake_width , CV_8U , Scalar(255)) ;
	for(int i = 0 ; i< N ;++i)
	{
		if(src[i].currentRect.width !=0 && src[i].currentRect.height!=0)
		{
			block(Rect(src[i].currentRect)).copyTo(tmpBoolMap(src[i].currentRect));
		}
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
	int distance_x =  src[n].candidateROI[src[n].level].cols-1 ; 
	int distance_y = src[n].candidateROI[src[n].level].rows-1 ;
	int minScore_x=0, minScore_y=0;
	double Score=0.4f;
	bool goodFlag = 0;
	for(int i = DIFF_X ; i < CANVAS_WIDTH + DIFF_X; i=i+ distance_x/2)
	{
		for(int j = DIFF_Y; j < CANVAS_HEIGHT + DIFF_Y; j = j+distance_y/2)
			{	

				
				rectangle(temp,Rect(i,j,distance_x,distance_y),Scalar(0,0,255));
				
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
	
	
	if(src[n].move(canvas,dx ,dy))
		goodFlag =  1;
	else
		goodFlag = 0 ;
	updateBoolMap(src , boolMap);
	draw(src , canvas);
	for(int i = 0 ; i<N;i++)
		src[i].updateVisbleAreaRatio(canvas);
	return (goodFlag) ? 1 : 0 ;

}  // revised
//qqq
double localMove(emotionData *src , Mat& canvas , Mat& boolMap , int number ,int mode)
{

	Rect canvasRect(DIFF_X , DIFF_Y , CANVAS_WIDTH , CANVAS_HEIGHT);
	draw(src , canvas);
	updateBoolMap(src ,boolMap);
	for(int i = 0 ; i< N ; ++i)
		src[i].updateVisbleAreaRatio(canvas);
	emotionData *p;
	int n  = 1  , side = 0 ,dx=0 ,dy=0 ; 

	double adjacentAreaBlankScore;
	double curGlobalBlankAreaRatio = 0 , nextGlobalBlankAreaRatio = 0;
	double curLocalVisbleAreaRatio = 0 , nextLocalVisbleAreaRatio = 0;
	double curLevelZeroBlockedROIRatio = 0 , nextLevelZeroBlockedROIRatio = 0;
	double curOtherLevelZeroBlockedROIRatioSum = 0 , nextOtherLevelZeroBlockedROIRatioSum = 0;
	//otherLevelZeroBlockedAreaSum(src ,canvas, number)
	bool goodFlag = 0 ,endFlag=0 , moveFlag = 0;
	int badCount = 0 ;
	while(n++ <10)
	{
		moveFlag = 0 ;
		for(int j = 0 ; j< 4 ;j++)
		{
				if(moveFlag)
					break;
				p = &src[number];
				side =100+ rand()%4; 
				Rect tmpRect = p->getAdjacentBlankArea(boolMap,side);
				curGlobalBlankAreaRatio = globalBlankAreaRatio(boolMap(canvasRect));
				curLocalVisbleAreaRatio = p->visbleAreaRatio;
				curLevelZeroBlockedROIRatio = p->LevelZeroROIBlockedRatio(canvas);
				curOtherLevelZeroBlockedROIRatioSum = otherLevelZeroBlockedAreaSum(src ,canvas, number);
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
					Rect beforeMove = p->currentRect;	
					bool movable = 0;
					movable = p->move(canvas , dx ,dy );
					if(movable)
					{
						updateBoolMap(src , boolMap);
						draw(src , canvas);
						for(int i = 0 ; i<N;i++)
							src[i].updateVisbleAreaRatio(canvas);
						nextGlobalBlankAreaRatio = globalBlankAreaRatio(boolMap(canvasRect));
						nextLocalVisbleAreaRatio = p->visbleAreaRatio;
						nextLevelZeroBlockedROIRatio = p->LevelZeroROIBlockedRatio(canvas);
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
							updateBoolMap(src , boolMap);
							draw(src , canvas);
							for(int i = 0 ; i<N;i++)
								src[i].updateVisbleAreaRatio(canvas);
						}
						else if( mode == 1
							 && nextGlobalBlankAreaRatio <= curGlobalBlankAreaRatio 
							// && nextOtherLevelZeroBlockedROIRatioSum <= curOtherLevelZeroBlockedROIRatioSum
						  ) // good
						{
							//badCount = 0;
							//std::cout<<"good"<<std::endl;
							moveFlag = 1;
							updateBoolMap(src , boolMap);
							draw(src , canvas);
							for(int i = 0 ; i<N;i++)
								src[i].updateVisbleAreaRatio(canvas);
						}
						else if( mode == 2
							 && nextLevelZeroBlockedROIRatio <= curLevelZeroBlockedROIRatio
							// && nextOtherLevelZeroBlockedROIRatioSum <= curOtherLevelZeroBlockedROIRatioSum
						  ) // good
						{
							//badCount = 0;
							//std::cout<<"good"<<std::endl;
							moveFlag = 1;
							updateBoolMap(src , boolMap);
							draw(src , canvas);
							for(int i = 0 ; i<N;i++)
								src[i].updateVisbleAreaRatio(canvas);
						}
						else
						{
						
							Rect afterMove = p->currentRect;	
							//std::cout<<"bad"<<std::endl;
							p->move(canvas , -dx ,-dy );
							updateBoolMap(src , boolMap);
							draw(src , canvas);
							for(int i = 0 ; i<N;i++)
								src[i].updateVisbleAreaRatio(canvas);
						
						}
					}
				}
			}
	}
	updateBoolMap(src , boolMap);
	draw(src , canvas);
	for(int i = 0 ; i<N;i++)
		src[i].updateVisbleAreaRatio(canvas);
	
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
	

	for(int i = 0; i <N ; i++) // i for pictures
	{
		/* j
		currentROIExtensionLeft_x : 0 , currentROIExtensionUp_y : 1 , 
		currentROIExtensionRight_x : 2 , currentROIExtensionDown_y : 3 ;
		*/
		int currentROIExtension[4];
		for(int j =0 ; j < 4 ; j++)
		{
				 if(j == 0)
				 {		
						currentROIExtension[j] = min( src[order[i]].currentRect.x,  src[order[i]].ShowRectInOriginalImage.x);
						src[order[i]].finalRectInCanvas.x = src[order[i]].finalRectInCanvas.x-currentROIExtension[j];
						src[order[i]].finalRectInCanvas.width = src[order[i]].finalRectInCanvas.width +currentROIExtension[j];
						src[order[i]].finalRectInSrc.x = src[order[i]].finalRectInSrc.x-currentROIExtension[j];
						src[order[i]].finalRectInSrc.width = src[order[i]].finalRectInSrc.width +currentROIExtension[j];
						
						if(src[order[i]].finalRectInCanvas.x < 0  || src[order[i]].finalRectInSrc.x <0)
							std::cout << "x dead" << std::endl;
				
				 }
				 else if(j == 1)
				 {
						currentROIExtension[j] = min( src[order[i]].currentRect.y,  src[order[i]].ShowRectInOriginalImage.y);
						src[order[i]].finalRectInCanvas.y = src[order[i]].finalRectInCanvas.y-currentROIExtension[j];
						src[order[i]].finalRectInCanvas.height = src[order[i]].finalRectInCanvas.height +currentROIExtension[j];
						src[order[i]].finalRectInSrc.y = src[order[i]].finalRectInSrc.y-currentROIExtension[j];
						src[order[i]].finalRectInSrc.height = src[order[i]].finalRectInSrc.height +currentROIExtension[j];

						if(src[order[i]].finalRectInCanvas.y < 0  || src[order[i]].finalRectInSrc.x <0)
							std::cout << "y dead" << std::endl;
				 }
				 else if(j == 2)
				 {
						currentROIExtension[j] = min( CANVAS_WIDTH -1 -src[order[i]].finalRectInCanvas.x,
						src[order[i]].originImage.cols-1 -src[order[i]].finalRectInSrc.x );
						src[order[i]].finalRectInCanvas.width =  currentROIExtension[j];
						src[order[i]].finalRectInSrc.width =  currentROIExtension[j];

						
		
						if(src[order[i]].finalRectInCanvas.x +src[order[i]].finalRectInCanvas.width  > CANVAS_WIDTH -1   || src[order[i]].finalRectInSrc.x + src[order[i]].finalRectInSrc.width > src[order[i]].originImage.cols-1)
							std::cout << "width dead" << std::endl;
				 }
				 else if(j == 3)
				 {
						currentROIExtension[j] = min( CANVAS_HEIGHT -1 -src[order[i]].finalRectInCanvas.y,
						src[order[i]].originImage.rows-1 -src[order[i]].finalRectInSrc.y );
						src[order[i]].finalRectInCanvas.height = currentROIExtension[j];
						src[order[i]].finalRectInSrc.height = currentROIExtension[j];
						
						if(src[order[i]].currentRect.y +src[order[i]].currentRect.height  > CANVAS_HEIGHT -1   || src[order[i]].ShowRectInOriginalImage.y + src[order[i]].ShowRectInOriginalImage.height> src[order[i]].originImage.rows-1)
							std::cout << "height dead" << std::endl;
				
				 }
		}


	}

	canvas.setTo(0);


	for(int i = 0 ; i< N ;++i)
	{
		Mat copyCanvas1 = canvas.clone();

		if(src[order[i]].finalRectInSrc.width !=0 && src[order[i]].finalRectInSrc.height!=0)
			src[order[i]].originImage(src[order[i]].finalRectInSrc).copyTo(canvas(Rect(src[order[i]].finalRectInCanvas)));
		Mat copyCanvas2 = canvas.clone();
		Rect *R = new Rect[N];
		for(int k = i-1 ; k>=0 ; k --) // k for the the picture under current picture
		{	
			if(rectIsValid(src[order[k]].currentRect) && isOverlapped(src[order[i]].finalRectInCanvas,src[order[k]].currentRect) ) 
				{
					R[k] = overlappedArea(src[order[i]].finalRectInCanvas,src[order[k]].currentRect);	
					copyCanvas1(R[k]).copyTo(canvas(R[k]));	
					copyCanvas2(src[order[i]].currentRect).copyTo(canvas(src[order[i]].currentRect));	
				}	
		}
			
	}

}
bool switchable(emotionData *src , Mat& canvas , Mat&boolMap,int p ,int q)
{
	emotionData *pp = &src[p] , *qq =&src[q];
	double cur_p_visibleArea = pp->visbleAreaRatio , cur_p_levelZeroBlockedArea = pp->LevelZeroROIBlockedRatio(canvas) ;
	double cur_q_visibleArea = qq->visbleAreaRatio , cur_q_levelZeroBlockedArea = qq->LevelZeroROIBlockedRatio(canvas) ;
	if(isOverlapped(src[p].currentRect , src[q].currentRect))
	{
		if(cur_p_levelZeroBlockedArea > 0.75 )
		{
			switchLayer(*pp ,*qq);
			draw(src , canvas);
			for(int i = 0 ;i< N;i++)
				src[i].updateVisbleAreaRatio(canvas);
			double next_p_visibleArea = pp->visbleAreaRatio , next_p_levelZeroBlockedArea = pp->LevelZeroROIBlockedRatio(canvas) ;
			double next_q_visibleArea = qq->visbleAreaRatio , next_q_levelZeroBlockedArea = qq->LevelZeroROIBlockedRatio(canvas) ;
			if(next_p_levelZeroBlockedArea <  cur_p_visibleArea )
				return 1;
			else
			{
				switchLayer(*pp ,*qq);
				draw(src , canvas);
				for(int i = 0 ;i< N;i++)
					src[i].updateVisbleAreaRatio(canvas);
				return 0 ;
			}
	
		}
	}
	else
		return 0 ;
}