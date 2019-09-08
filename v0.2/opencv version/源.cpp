#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/background_segm.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "highgui.h"
#include <opencv\cxcore.hpp>
#include<opencv2\opencv.hpp>
#include<Windows.h> 
#include<string.h>
#include <stdio.h>
using namespace std;
using namespace cv;

Mat g_srcImage;
Mat g_grayImage;

const int g_dReferWidth = 19;//比例，串口数据更改！ 
double g_dPixelsPerMetric;
vector<vector<cv::Point>> g_vContours;
vector<Vec4i> g_vHierarchy;
bool g_bFirst = true;

HANDLE hComm;
char lpOutbuffer[100];
DWORD dwbyte = 100;

static cv::Point2f midpoint(cv::Point2f& ptA, cv::Point2f& ptB);//求中点 
static float getDistance(Point2f pointA, Point2f pointB);//求距离
static bool ContoursSortFun(vector<cv::Point> contour1, vector<cv::Point> contour2);//按照 x坐标 排序

int main(int argc, const char** argv)
{
	//获取摄像头图像
	cv::namedWindow("Camera", cv::WINDOW_AUTOSIZE);   // set windows auto
	cv::VideoCapture capture(0);                      // default 0 is the first of camera in computer
	cv::Mat bgr_frame;
	Mat img1;
	while (true) {
		COMSTAT Comstat;
		DWORD dwError;
		BOOL bWritestat;
		hComm = CreateFile("COM3", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, NULL);
		if (hComm == INVALID_HANDLE_VALUE)
		{
			cout << "FLASE";
			return -1;
		}
		else
		{
			cout << "TURE";
		}
		DCB dcb;
		GetCommState(hComm, &dcb);
		dcb.BaudRate = 9600;
		dcb.ByteSize = 8;
		dcb.Parity = NOPARITY;
		dcb.StopBits = TWOSTOPBITS;
		bool set = SetCommState(hComm, &dcb);
		bool sup = SetupComm(hComm, 1024, 1024);
		printf("\n%d\n", sup);

		capture >> g_srcImage;
		if (g_srcImage.empty())
			break;
		cv::imshow("Camera", g_srcImage);
		char c = cv::waitKey(1);
		if (c == 27)
			break;

		//灰度 降低计算量
		cvtColor(g_srcImage, g_grayImage, COLOR_BGR2GRAY);

		//高斯滤波 降噪
		GaussianBlur(g_grayImage, g_grayImage, Size(7, 7), 0);
		imshow("高斯滤波", g_grayImage);

		//经测试不使用直方图均衡化，更大图像反差获取物体边缘的成功率更高
		//equalizeHist(g_grayImage, g_grayImage);

		//边缘检测
		Canny(g_grayImage, g_grayImage, 50, 100);

		Mat element = getStructuringElement(MORPH_RECT, Size(15, 15)); //隔开物体
		dilate(g_grayImage, g_grayImage, element);//膨胀
		erode(g_grayImage, g_grayImage, element);//腐蚀
		imshow("形态学", g_grayImage);

		//寻找轮廓
		findContours(g_grayImage, g_vContours, g_vHierarchy, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		std::sort(g_vContours.begin(), g_vContours.end(), ContoursSortFun);//按照从左到右 排序
		for (unsigned i = 0; i < g_vContours.size(); i++) {

			if (contourArea(g_vContours[i]) < 100)//面积太小 则忽略
				continue;

			RotatedRect box = minAreaRect(g_vContours[i]);
			Point2f boxPoints[4];
			box.points(boxPoints);

			Point2f pointA = midpoint(boxPoints[0], boxPoints[1]);
			Point2f pointB = midpoint(boxPoints[1], boxPoints[2]);
			Point2f pointC = midpoint(boxPoints[2], boxPoints[3]);
			Point2f pointD = midpoint(boxPoints[3], boxPoints[0]);

			circle(g_srcImage, pointA, 2, Scalar(0, 0, 255));
			circle(g_srcImage, pointB, 2, Scalar(0, 0, 255));
			circle(g_srcImage, pointC, 2, Scalar(0, 0, 255));
			circle(g_srcImage, pointD, 2, Scalar(0, 0, 255));

			line(g_srcImage, pointA, pointC, Scalar(255, 0, 0));
			line(g_srcImage, pointD, pointB, Scalar(255, 0, 0));

			double dWidth = getDistance(pointA, pointC);
			double dHeight = getDistance(pointD, pointB);
			if (g_bFirst) {
				g_dPixelsPerMetric = dWidth / g_dReferWidth; //计算像素与 实际大小的比列
				cout << "pixelPerMetric:" << dWidth << " " << g_dReferWidth << "  " << g_dPixelsPerMetric;
				g_bFirst = false;
			}

			cout << "dWidth" << dWidth << "   " << dHeight << "      " << dWidth / g_dPixelsPerMetric << "    " << dHeight / g_dPixelsPerMetric;
			putText(g_srcImage, cv::format("(%.0f,%.0f)", dWidth / g_dPixelsPerMetric, dHeight / g_dPixelsPerMetric), boxPoints[2], FONT_HERSHEY_COMPLEX, 0.5, Scalar(0, 0, 255));

			for (int i = 0; i <= 3; i++)
			{
				line(g_srcImage, boxPoints[i], boxPoints[(i + 1) % 4], Scalar(0, 255, 0));
			}
		}


		cv::namedWindow("效果", CV_WINDOW_AUTOSIZE);
		cv::imshow("效果", g_srcImage);

		waitKey(0);
	}
	return 0;
}

Point2f midpoint(Point2f& ptA, Point2f& ptB) {
	return Point2f((ptA.x + ptB.x) * 0.5, (ptA.y + ptB.y) * 0.5);
}

float getDistance(Point2f pointA, Point2f pointB)
{
	float distance;
	distance = powf((pointA.x - pointB.x), 2) + powf((pointA.y - pointB.y), 2);
	distance = sqrtf(distance);
	return distance;
}

bool ContoursSortFun(vector<cv::Point> contour1, vector<cv::Point> contour2) {
	return  (contour1[0].x < contour2[0].x); // a.x < b.x;
}