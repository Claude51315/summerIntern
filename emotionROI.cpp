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
void emotionData::initialize()
{
	this->level = 0; // set level = 0 
	updateCandidateROI(); 
	updateCurrentRect(); // choose candidateROI[layer] 
	updateCorners();  // update the remaining three corners 
	
}
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
}
void emotionData::updateCurrentRect()
{
	int range_x =this->candidateROI[this->level].cols, 
		range_y =this->candidateROI[this->level].rows, 
		xmin = 0, ymin = 0 ;
	
	corners[0].x = seed.x ; 
	corners[0].y = seed.y;
	if(corners[0].x+candidateROI[level].cols >= CANVAS_WIDTH -1 )
			range_x = CANVAS_WIDTH-1-corners[0].x;
	if(corners[0].y + candidateROI[level].rows >= CANVAS_HEIGHT -1)
			range_y = CANVAS_HEIGHT-1-corners[0].y;
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
	currentRect = Rect(corners[0].x ,corners[0].y , range_x , range_y);
}	
void emotionData::expand()
{
	if(level != candidateROI.size()-1)
	{
		level++;
		updateCurrentRect();
		updateCorners();
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
			Rect tmpRect  = currentRect + Point(0 , - candidateROI[layer].rows);
			if(tmpRect.y < 0)
			{
				tmpRect.height += tmpRect.y;
				tmpRect.y = 0;
			}
			return tmpRect;
		}
		break;
		case downAdjacent:
		{
		
			Rect tmpRect  = currentRect + Point(0 , candidateROI[layer].rows);
			if(tmpRect.y  + tmpRect.height >= CANVAS_HEIGHT-1)
			{
				tmpRect.height =    CANVAS_HEIGHT-1 - tmpRect.y ;
			}
			return tmpRect;
			break;
		}
		case leftAdjacent:
		{
			Rect tmpRect  = currentRect + Point(candidateROI[layer].cols , 0);
			if(tmpRect.x  + tmpRect.width >= CANVAS_WIDTH-1)
			{
				tmpRect.width =    CANVAS_WIDTH-1 - tmpRect.x;
				
			}
			return tmpRect;
			break;
		}
		case rightAdjacent:
		{
			Rect tmpRect  = currentRect + Point(-candidateROI[layer].cols , 0);
			if(tmpRect.x <=  0)
			{
				tmpRect.width += tmpRect.x;
				tmpRect.x = 0;
			}
			return tmpRect;
			break;
		}
	}

}
bool emotionData::move(Mat &canvas, int deltaX , int deltaY)
{
	if( !(
			((deltaY + this->seed.y) <=1)||
			((deltaX + this->seed.x) <=1)||
			(deltaX < 0 && this->seed.x <=1)|| 
			(deltaY < 0 && this->seed.y <=1)
			|| (deltaX > 0 && this->seed.x > canvas.cols-1) || 
			(deltaY > 0 && this->seed.y > canvas.rows-1) ||
			(deltaY + this->seed.y >= canvas.rows-1) || 
			(deltaX + this->seed.x >= canvas.cols-1)|| 
			(deltaY + this->seed.y + candidateROI[this->level].rows <= 1) ||
			(deltaX + this->seed.x + candidateROI[this->level].cols <= 1)
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
double emotionData::localBlockedAreaRatio(Mat& canvas , int mlevel) //t 
{
	/*counting ratio with respect to area*/
		
		double output =0 , blocked_area = 0;
		
		Mat covered_current = canvas( currentRect).clone();
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



}
double emotionData::LevelZeroROIBlockedRatio(Mat& canvas)
{

	double blockedArea , output = 1;
	Mat tmpLevelzero = candidateROI[0];
	int tmpWidth = tmpLevelzero.cols , tmpHeight = tmpLevelzero.rows;
	Mat tmpOnCanvas(tmpHeight , tmpWidth , CV_8UC3 , Scalar(0,0,0));
	Rect showOnCanvas;
	int tmpx = candidateROIRect[0].x  - candidateROIRect[level].x , 
		tmpy = candidateROIRect[0].y  - candidateROIRect[level].y ;
	int lx = seed.x + tmpx ,ly = seed.y + tmpy;
	int lx_0_dis = 0 - lx , ly_0_dis = 0 - ly ; 
	int XonCandidateROI =0  ,YonCandidateROI =0 , widthOnCandidateROI = tmpWidth  , heightOnCandidateROI = tmpHeight;
	if(lx < 0)
		XonCandidateROI = -lx ; 
	if(ly < 0)
		YonCandidateROI = -ly ; 
	if(lx +  tmpWidth > CANVAS_WIDTH)
		widthOnCandidateROI = CANVAS_WIDTH - lx -1 ;
	if(ly + tmpHeight > CANVAS_HEIGHT)
		heightOnCandidateROI = CANVAS_HEIGHT - ly -1  ;
	lx = (lx < 0) ? 0 : lx;
	ly = (ly < 0) ? 0 : ly;
	std::cout<<" lx = " << lx <<std::endl;
	std::cout<<" ly = " << ly <<std::endl;
	std::cout<<" widthOnCandidateROI = " << widthOnCandidateROI <<std::endl;
	std::cout<<" heightOnCandidateROI = " << heightOnCandidateROI <<std::endl;
	
	showOnCanvas =Rect(lx , ly , widthOnCandidateROI , heightOnCandidateROI);
	
	canvas(showOnCanvas).copyTo(tmpOnCanvas(Rect( XonCandidateROI,YonCandidateROI,widthOnCandidateROI,heightOnCandidateROI)));
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
			temp_seed_x = (int)(rand()% canvasWidth);
			temp_seed_y = (int)(rand()% canvasHeight); 
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
          for (int i=0; i<src.cols * src.channels(); i=i+ src.channels()) {  
            if(data[i]== 0)
				src_score++;
            } // end of row                   
      }  
	  output = (double)src_score/(src.cols*src.rows);
	  return output;
}
void draw(emotionData *src , Mat &canvas)
{
	canvas =  Mat(CANVAS_HEIGHT   , CANVAS_WIDTH , CV_8UC3 , Scalar(0,0,0)).clone();
	for(size_t i = 0 ; i< N ;++i)
	{
		int range_x =src[i].currentRect.width, range_y =src[i].currentRect.height;
		
		Rect showRect(src[i].corners[0].x - src[i].seed.x,
			          src[i].corners[0].y - src[i].seed.y,
					  src[i].currentRect.width ,
					  src[i].currentRect.height);	
		if(showRect.width !=0 && showRect.height!=0)
			src[i].candidateROI[src[i].level](showRect).copyTo(canvas(Rect(src[i].currentRect)));
	}
}
bool isOverlapped(emotionData src1, emotionData src2)
{
	Rect check = overlappedArea( src1,  src2);
	if(check.x == 0 && check.y == 0 && check.height == 0 && check.width == 0)
		return false;
	else
		return true;
} 
Rect overlappedArea(emotionData src1, emotionData src2)
{
		Rect intersect,
			 R1 = src1.currentRect,
			 R2 = src2.currentRect; 

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

void updateBoolMap(emotionData *src , Mat& outputBoolMap)
{
	outputBoolMap.release();
	Mat tmpBoolMap(CANVAS_HEIGHT   , CANVAS_WIDTH , CV_8U , Scalar(0)) , 
	block(CANVAS_HEIGHT   , CANVAS_WIDTH , CV_8U , Scalar(255)) ;
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
	if(tmpRect.height >0 && tmpRect.width > 0 && tmpRect.x >= 0 && tmpRect.x < CANVAS_WIDTH  && tmpRect.y >=0 && tmpRect.y<CANVAS_HEIGHT) 
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
void globalRandomMove(emotionData *src ,int n, Mat& canvas , Mat& boolMap)
{
	Point output;
	srand(time(NULL));
	Mat temp = canvas.clone();
	temp(src[n].currentRect).setTo(0);
	int distance_x =  src[n].candidateROI[src[n].level].cols-1 ; 
	int distance_y = src[n].candidateROI[src[n].level].rows-1 ;
	int minScore_x=0, minScore_y=0;
	double Score=0.5f;
	bool goodFlag = 0;
	for(int i = 0; i < CANVAS_WIDTH; i=i+ distance_x/2)
	{
		for(int j = 0; j < CANVAS_HEIGHT; j = j+distance_y/2)
			{	
				if(i+distance_x <temp.cols && j+distance_y <temp.rows)
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
		
		for(int i = CANVAS_WIDTH-1; i >= 0 ; i=i- distance_x/2)
		for(int j = CANVAS_HEIGHT-1 ; j >=0 ; j = j- distance_y/2)
			{	
				if(i+distance_x <temp.cols && j+distance_y <temp.rows)
				if( globalBlankAreaRatio(temp(Rect(i,j,distance_x,distance_y))) >= Score)
				{
					minScore_x = i;
					minScore_y = j; 
					Score = globalBlankAreaRatio(temp(Rect(i,j,distance_x,distance_y)));
					if(Score==1)
						break;
				}
			}
			
	output.x =  minScore_x + rand()%distance_x/16 ; 
	output.y =  minScore_y +rand()%distance_y/16;
	int dx = output.x-src[n].corners[0].x, dy = output.y-src[n].corners[0].y;
	src[n].move(canvas,dx ,dy);
	updateBoolMap(src , boolMap);
	draw(src , canvas);
	for(int i = 0 ; i<N;i++)
		src[i].updateVisbleAreaRatio(canvas);
		

}  // revised
double localMove(emotionData *src , Mat& canvas , Mat& boolMap , int number)
{
	draw(src , canvas);
	updateBoolMap(src ,boolMap);
	for(int i = 0 ; i< N ; ++i)
		src[i].updateVisbleAreaRatio(canvas);
	emotionData *p;
	int n  = 1  , side = 0 ,dx=0 ,dy=0 ; 


	double curGlobalBlankAreaRatio = 0 , nextGlobalBlankAreaRatio = 0;
	double curLocalVisbleAreaRatio = 0 , nextLocalVisbleAreaRatio = 0;
	double curLevelZeroBlockedROIRatio = 0 , nextLevelZeroBlockedROIRatio = 0;
	double curOtherLevelZeroBlockedROIRatioSum = 0 , nextOtherLevelZeroBlockedROIRatioSum = 0;
	//otherLevelZeroBlockedAreaSum(src ,canvas, number)
	bool goodFlag = 0 ,endFlag=0 , moveFlag = 0;
	int badCount = 0 ;
	
	std::cout <<"local move 1 \n";
	while(n++ <10)
	{
		moveFlag = 0 ;
		for(int j = 0 ; j< 4 ;j++)
		{

			std::cout <<"local move 2 \n";
				if(moveFlag)
					break;
				p = &src[number];
				side = j + 100 ; 
				Rect tmpRect = p->getAdjacentBlankArea(boolMap,side);
				curGlobalBlankAreaRatio = globalBlankAreaRatio(boolMap);
				curLocalVisbleAreaRatio = p->visbleAreaRatio;
				curLevelZeroBlockedROIRatio = p->LevelZeroROIBlockedRatio(canvas);
				curOtherLevelZeroBlockedROIRatioSum = otherLevelZeroBlockedAreaSum(src ,canvas, number);
				//imshow("before move" , canvas);
				if(rectIsValid(tmpRect))
				{
					dx = (tmpRect.x - p->currentRect.x) / ((p->level+1));
					dy = (tmpRect.y - p->currentRect.y) / ((p->level+1)) ;
					if(dx == 0)
						dx = mindx(side);
					if(dy == 0)
						dy = mindy(side);
					//std::cout << "dx = " <<dx <<std::endl;
					//std::cout << "dy = " <<dy <<std::endl;
					/*
					std::cout <<"picture = "<<order[i]<<std::endl;
					std::cout <<"side = "<<side<<std::endl;
					std::cout <<"dx = "<<dx<<std::endl;
					std::cout <<"dy = "<<dy<<std::endl;
					*/
					std::cout <<"local move 3 \n";

					Rect beforeMove = p->currentRect;	
					//std::cout<<"local move" <<std::endl;
					if(p->move(canvas , dx ,dy ))
						;
					else
					{
						dx = mindx(side);
						dy = mindy(side);
						p->move(canvas , dx ,dy );
					}
					//system("pause");
					updateBoolMap(src , boolMap);
					draw(src , canvas);
					for(int i = 0 ; i<N;i++)
						src[i].updateVisbleAreaRatio(canvas);
					std::cout <<"local move 4 \n";

					//imshow("tmp move" , canvas);
					nextGlobalBlankAreaRatio = globalBlankAreaRatio(boolMap);
					nextLocalVisbleAreaRatio = p->visbleAreaRatio;
					nextLevelZeroBlockedROIRatio = p->LevelZeroROIBlockedRatio(canvas);
					nextOtherLevelZeroBlockedROIRatioSum = otherLevelZeroBlockedAreaSum(src ,canvas, number);
					/*
						std::cout<<"curGlobalBlankAreaRatio = "<<curGlobalBlankAreaRatio<<std::endl;
						std::cout<<"nextGlobalBlankAreaRatio = "<<nextGlobalBlankAreaRatio<<std::endl;
						std::cout<<"curLocalVisbleAreaRatio = "<<curLocalVisbleAreaRatio<<std::endl;
						std::cout<<"nextLocalVisbleAreaRatio = "<<nextLocalVisbleAreaRatio<<std::endl;
					*/
					if( nextGlobalBlankAreaRatio  <= curGlobalBlankAreaRatio&& 
						nextLocalVisbleAreaRatio >= curLocalVisbleAreaRatio  &&
						nextLevelZeroBlockedROIRatio <= curLevelZeroBlockedROIRatio// &&
						//nextOtherLevelZeroBlockedROIRatioSum <= curOtherLevelZeroBlockedROIRatioSum
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
						if(beforeMove.x ==afterMove.x && beforeMove.y == afterMove.y)
						 ;
						else
							p->move(canvas , -dx ,-dy );
						updateBoolMap(src , boolMap);
						draw(src , canvas);
						for(int i = 0 ; i<N;i++)
							src[i].updateVisbleAreaRatio(canvas);
					}
					//imshow("rrr" , canvas);
					//waitKey(0);
					//std::cout<<"------------------------ "<<std::endl;
				}
				else
				{

					dx = mindx(side);
					dy = mindy(side);
					p->move(canvas , dx ,dy );
				}
			}
	}
	updateBoolMap(src , boolMap);
	draw(src , canvas);
	for(int i = 0 ; i<N;i++)
		src[i].updateVisbleAreaRatio(canvas);
	return globalBlankAreaRatio(boolMap);
}
int mindx(int side)
{
	switch(side)
	{
		case upAdjacent:
		case downAdjacent:
		{
			return 0 ; 
			break;
		}
		case leftAdjacent:
		{
			return -10 ;
			break;
		}
		case rightAdjacent:
		{
			return 10 ;
			break;
		}
	}

}
int mindy(int side)
{
	switch(side)
	{
		case rightAdjacent:
		case leftAdjacent:
		{
			return 0 ; 
			break;
		}
		case upAdjacent:
		{
			return -10 ;
			break;
		}
		case downAdjacent:
		{
			return 10 ;
			break;
		}	
	}
}
double maxinumVisibleAreaRatio(emotionData *src , Mat& canvas , Mat& boolMap)
{
	double canvasArea = canvas.cols * canvas.rows; 
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
			ans += src[i].LevelZeroROIBlockedRatio(canvas);
		}
	return ans ; 
}