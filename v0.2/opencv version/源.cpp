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
#include<stdlib.h>
using namespace std;
using namespace cv;

Mat g_srcImage;
Mat g_grayImage;

vector<vector<cv::Point>> g_vContours;
vector<Vec4i> g_vHierarchy;

HANDLE hCom;

static cv::Point2f midpoint(cv::Point2f& ptA, cv::Point2f& ptB);//求中点 
static float getDistance(Point2f pointA, Point2f pointB);//求距离
static bool ContoursSortFun(vector<cv::Point> contour1, vector<cv::Point> contour2);//按照 x坐标 排序

int main(int argc, const char** argv) {

	//获取摄像头图像
	cv::namedWindow("Camera", cv::WINDOW_AUTOSIZE);   // set windows auto
	cv::VideoCapture capture(0);                      // default 0 is the first of camera in computer
	cv::Mat bgr_frame;
	Mat img1;

	while (true) {
		double gpm = 0;
		hCom = CreateFile(TEXT("com3"),
			GENERIC_READ, //允许读
			0, //指定共享属性，由于串口不能共享，所以该参数必须为0
			NULL,
			OPEN_EXISTING, //打开而不是创建

			FILE_ATTRIBUTE_NORMAL, //属性描述，该值为FILE_FLAG_OVERLAPPED，表示使用异步I/O，该参数为0，表示同步I/O操作
			NULL);

		//		if (hCom != INVALID_HANDLE_VALUE) {
		DCB lpTest;
		GetCommState(hCom, &lpTest); 		//获取当前的参数设置
		lpTest.BaudRate = CBR_9600; 		//波特率为9600
		lpTest.ByteSize = 8; 			//数据位数为8
		lpTest.Parity = NOPARITY; 		//无校验
		lpTest.StopBits = ONESTOPBIT; 		//1位停止位
		SetCommState(hCom, &lpTest);		//设置通信参数
		DWORD btsIO;
		char dist[4];
		ReadFile(hCom, dist, strlen(dist), &btsIO, NULL);
		double distd = atof(dist);
		//		}
		Sleep(100);
		CloseHandle(hCom);			//关闭串口

		double pmf = distd / 20.;//图像修正值
		double g_dReferWidth = distd / pmf;	//像素比例
		printf("\n%f\n%f\n", distd, g_dReferWidth);

		//图像获取
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
			//判断框线点
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

			//绘制框线
			line(g_srcImage, pointA, pointC, Scalar(255, 0, 0));
			line(g_srcImage, pointD, pointB, Scalar(255, 0, 0));

			//框线像素数
			double dWidth = getDistance(pointA, pointC);
			double dHeight = getDistance(pointD, pointB);

			//if (g_bFirst) {
				//g_dPixelsPerMetric = dWidth / g_dReferWidth; //计算像素与 实际大小的比列
				//cout << "pixelPerMetric:" << dWidth << " " << g_dReferWidth << "  " << g_dPixelsPerMetric;
				//g_bFirst = false;
			//}
			if (gpm == 0) {
				gpm = dWidth / g_dReferWidth;
			}
			printf("%f\n", gpm);
			printf("%f\n%f\n", dWidth, dHeight);
			//修正实际比例
			dWidth = dWidth * 2.5;
			dHeight = dHeight * 2.5;
			//cout << "dWidth" << dWidth << "   " << dHeight << "      " << dWidth / g_dPixelsPerMetric << "    " << dHeight / g_dPixelsPerMetric;
			putText(g_srcImage, cv::format("(%.0f,%.0f)", dWidth / gpm, dHeight / gpm), boxPoints[2], FONT_HERSHEY_COMPLEX, 0.5, Scalar(0, 0, 255));

			//绘制框线
			for (int i = 0; i <= 3; i++) {
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