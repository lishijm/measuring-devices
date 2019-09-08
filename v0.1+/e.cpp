#include<opencv2\opencv.hpp> 
#include<opencv\cv.h> 
#include<opencv\highgui.h> 
#include<math.h> 
#include<Windows.h> 
#include<string.h> 
 
 
using namespace std; 
using namespace cv; 
 
#define WINDOW_NAME "【程序窗口】"    
 
void on_MouseHandle(int event, int x, int y, int flags, void* param); 
void DrawRectangle( cv::Mat& img, cv::Rect box ); 
void tracking(Mat &frame,vector<Point2f> temp); 
 
HANDLE hComm; 
LPCWSTR pStr=L"COM4"; 
char lpOutbuffer[100]; 
DWORD dwbyte=100; 
Mat srcImage,grayImage,tempImage1,tempImage,imageROI,grayprev; 
int g_maxCornerNumber = 1; 
double qualityLevel = 0.01; 
double minDistance = 10; 
int blockSize = 3; 
double k = 0.04; 
vector<Point2f> corners; 
vector<Point2f> pre_corners; 
vector<Point2f> counts; 
vector<uchar> status; 
vector<float> err; 
Rect g_rectangle; 
Rect g_temprectangle; 
bool g_bDrawingBox = false; 
 
 
int main( int argc, char** argv )  
{ 
  Mat frame; 
  Mat result; 
 
  COMSTAT Comstat; 
  DWORD dwError; 
  BOOL bWritestat; 
  hComm=CreateFile(pStr,GENERIC_READ | GENERIC_WRITE,0,0,OPEN_EXISTING, 0,NULL); 
  if (hComm == INVALID_HANDLE_VALUE) 
  { 
    cout<<"FLASE"; 
    return -1; 
  } 
  else 
  { 
    cout<<"TURE"; 
  } 
  DCB dcb; 
  GetCommState(hComm,&dcb); 
  dcb.BaudRate=9600; 
  dcb.ByteSize=8; 
  dcb.Parity=NOPARITY; 
  dcb.StopBits=TWOSTOPBITS; 
  bool set=SetCommState(hComm,&dcb); 
  bool sup=SetupComm(hComm,1024,1024); 
 
  VideoCapture capture(0); 
 
  namedWindow( WINDOW_NAME ); 
  setMouseCallback(WINDOW_NAME,on_MouseHandle,(void*)&frame); 
 
  while(1) 
  { 
    capture >> frame; 
 
      if(!frame.empty()) 
      {  
        cvtColor(frame,grayImage,CV_RGB2GRAY); 
        if( g_bDrawingBox )  
        rectangle(frame,g_rectangle.tl(),g_rectangle.br(),Scalar(255,255,255)); 
        if (corners.size()!=0) 
        { 
          bool can=PurgeComm(hComm,PURGE_TXCLEAR); 
          if (corners[0].x>(frame.cols/2+100)) 
          { 
            lpOutbuffer[0]='a'; 
            bool ne=WriteFile(hComm,lpOutbuffer,dwbyte,&dwbyte,NULL); 
          } 
          else if (corners[0].x<(frame.cols/2-100)) 
          { 
            lpOutbuffer[0]='b'; 
            bool ne=WriteFile(hComm,lpOutbuffer,dwbyte,&dwbyte,NULL); 
          } 
          tracking(frame,corners); 
          rectangle(frame,Point(corners[0].x-10,corners[0].y-10),Point(corners[0].x+10,corners[0].y+10),Scalar(255,255,255));          
        } 
        imshow( WINDOW_NAME, frame ); 
      } 
      else 
      {  
        printf(" --(!) No captured frame -- Break!"); 
        break; 
      } 
 
      int c = waitKey(50); 
      if( (char)c == 27 ) 
      { 
        break;  
      }  
  } 
  return 0; 
} 
void on_MouseHandle(int event, int x, int y, int flags, void* param) 
{ 
 
  Mat& image = *(cv::Mat*) param; 
  switch( event) 
  { 
  case EVENT_MOUSEMOVE:  
    { 
      if( g_bDrawingBox ) 
      { 
        g_rectangle.width = x-g_rectangle.x; 
        g_rectangle.height = y-g_rectangle.y; 
      } 
    } 
    break; 
 
  case EVENT_LBUTTONDOWN:  
    { 
      g_bDrawingBox = true; 
      g_rectangle =Rect( x, y, 0, 0 ); 
    } 
    break; 
 
  case EVENT_LBUTTONUP:  
    { 
      g_bDrawingBox = false; 
      if( g_rectangle.width < 0 ) 
      { 
        g_rectangle.x += g_rectangle.width; 
        g_rectangle.width *= -1; 
      } 
 
      if( g_rectangle.height < 0 )  
      { 
        g_rectangle.y += g_rectangle.height; 
        g_rectangle.height *= -1; 
      } 
      imageROI=grayImage(g_rectangle); 
      goodFeaturesToTrack( imageROI,corners,g_maxCornerNumber,qualityLevel,minDistance,Mat(),blockSize,false,k ); 
      for (int i = 0; i < corners.size(); i++) 
      { 
        corners[i].x=corners[i].x+g_rectangle.x; 
        corners[i].y=corners[i].y+g_rectangle.y; 
      } 
    } 
    break; 
 
  } 
} 
 
void tracking(Mat &frame,vector<Point2f> temp) 
{ 
  cvtColor(frame, tempImage1, COLOR_BGR2GRAY); 
 
  if (grayprev.empty()) 
  { 
    tempImage1.copyTo(grayprev); 
  } 
 
  calcOpticalFlowPyrLK(grayprev, tempImage1, temp, pre_corners, status, err); 
 
  for (size_t i=0; i<pre_corners.size(); i++) 
  { 
    line(frame, temp[i], pre_corners[i], Scalar(0, 0, 255)); 
    circle(frame, pre_corners[i], 4, Scalar(0, 255, 0), -1,8,0); 
  } 
 
  swap(pre_corners, corners); 
  swap(grayprev, tempImage1); 
} 