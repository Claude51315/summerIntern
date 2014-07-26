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
				candidateROI.push_back(	originImage(crop).clone());
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
void emotionData::updateCorners()
{
	corners[1] = Point(corners[0].x + currentRect.width ,corners[0].y); 
	corners[2] = Point(corners[0].x  ,corners[0].y + currentRect.height); 
	corners[3] = Point(corners[0].x + currentRect.width ,corners[0].y+ currentRect.height ); 
}
void emotionData::updateCurrentRect()
{
	int range_x =this->candidateROI[this->level].cols-1, 
		range_y =this->candidateROI[this->level].rows-1, 
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
Mat emotionData::getAdjacentBlankArea(Mat& boolMap , int side)
{

	Mat tmp;
	switch(side)
	{
		case upAdjacent:
		break;
		case downAdjacent:
			break;
		case leftAdjacent:
			break;
		case rightAdjacent:
			Rect rightSideRect(0,corners[0].y ,corners[0].x , currentRect.height); // right ;
			tmp = boolMap(rightSideRect).clone();
		break;
	}
}
void emotionData::move(Mat &canvas, int deltaX , int deltaY)
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
	}
	updateCurrentRect();
	updateCorners();			
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
double blankAreaRatio(Mat src)
{
	double output=0, src_score=0;
      int nr= src.rows; // number of rows  
      for (int j=0; j<nr; j++) {  
          uchar* data= src.ptr<uchar>(j);  
          for (int i=0; i<src.cols * src.channels(); i=i+3) {  
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
		src[i].candidateROI[src[i].level](showRect).copyTo(canvas(Rect(src[i].currentRect)));
	}
}