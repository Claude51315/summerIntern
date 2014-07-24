#include "emotionROI.h"
bool readImage( std::fstream& emotionFiles, emotionData& output ) 
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
		
		std::cout<<text<<std::endl;

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
			std::cout<<"QQ"<<std::endl;
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
double emotionData::emotionROI_ratio(Mat stimuiusMap,int min_x, int min_y, int max_x, int max_y)
{
	double output=0, stimuiusMap_score=0, cropROI_score=0;

	for(int i = 0 ; i < stimuiusMap.cols ; i++)
			for(int j = 0 ; j < stimuiusMap.rows ; j++)
			{
						stimuiusMap_score += stimuiusMap.at<uchar>(j,i);
						if(i>=min_x && i<=max_x && j<=max_y && j>=min_y)
							cropROI_score += stimuiusMap.at<uchar>(j,i);
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