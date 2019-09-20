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

double g_dPixelsPerMetric;
vector<vector<cv::Point>> g_vContours;
vector<Vec4i> g_vHierarchy;
bool g_bFirst = true;

HANDLE hCom;

static cv::Point2f midpoint(cv::Point2f& ptA, cv::Point2f& ptB);//���е� 
static float getDistance(Point2f pointA, Point2f pointB);//�����
static bool ContoursSortFun(vector<cv::Point> contour1, vector<cv::Point> contour2);//���� x���� ����

int main(int argc, const char** argv) {

	//��ȡ����ͷͼ��
	cv::namedWindow("Camera", cv::WINDOW_AUTOSIZE);   // set windows auto
	cv::VideoCapture capture(0);                      // default 0 is the first of camera in computer
	cv::Mat bgr_frame;
	Mat img1;

	while (true) {
		hCom = CreateFile(TEXT("com3"),
			GENERIC_READ, //�����
			0, //ָ���������ԣ����ڴ��ڲ��ܹ������Ըò�������Ϊ0
			NULL,
			OPEN_EXISTING, //�򿪶����Ǵ���

			FILE_ATTRIBUTE_NORMAL, //������������ֵΪFILE_FLAG_OVERLAPPED����ʾʹ���첽I/O���ò���Ϊ0����ʾͬ��I/O����
			NULL);

		//		if (hCom != INVALID_HANDLE_VALUE) {
		DCB lpTest;
		GetCommState(hCom, &lpTest); 		//��ȡ��ǰ�Ĳ�������
		lpTest.BaudRate = CBR_9600; 		//������Ϊ9600
		lpTest.ByteSize = 8; 			//����λ��Ϊ8
		lpTest.Parity = NOPARITY; 		//��У��
		lpTest.StopBits = ONESTOPBIT; 		//1λֹͣλ
		SetCommState(hCom, &lpTest);		//����ͨ�Ų���
		DWORD btsIO;
		char dist[4];
		ReadFile(hCom, dist, strlen(dist), &btsIO, NULL);
		double distd = atof(dist);
		//		}
		Sleep(100);
		CloseHandle(hCom);			//�رմ���

		double g_dReferWidth = distd * 4;	//���ر���
		printf("\n%f\n", g_dReferWidth);
		//ͼ���ȡ
		capture >> g_srcImage;
		if (g_srcImage.empty())
			break;
		cv::imshow("Camera", g_srcImage);
		char c = cv::waitKey(1);
		if (c == 27)
			break;

		//�Ҷ� ���ͼ�����
		cvtColor(g_srcImage, g_grayImage, COLOR_BGR2GRAY);

		//��˹�˲� ����
		GaussianBlur(g_grayImage, g_grayImage, Size(7, 7), 0);
		imshow("��˹�˲�", g_grayImage);

		//�����Բ�ʹ��ֱ��ͼ���⻯������ͼ�񷴲��ȡ�����Ե�ĳɹ��ʸ���
		equalizeHist(g_grayImage, g_grayImage);

		//��Ե���
		Canny(g_grayImage, g_grayImage, 50, 100);

		Mat element = getStructuringElement(MORPH_RECT, Size(15, 15)); //��������
		dilate(g_grayImage, g_grayImage, element);//����
		erode(g_grayImage, g_grayImage, element);//��ʴ
		imshow("��̬ѧ", g_grayImage);

		//Ѱ������
		findContours(g_grayImage, g_vContours, g_vHierarchy, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		std::sort(g_vContours.begin(), g_vContours.end(), ContoursSortFun);//���մ����� ����
		for (unsigned i = 0; i < g_vContours.size(); i++) {

			if (contourArea(g_vContours[i]) < 100)//���̫С �����
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
				g_dPixelsPerMetric = dWidth / g_dReferWidth; //���������� ʵ�ʴ�С�ı���
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


		cv::namedWindow("Ч��", CV_WINDOW_AUTOSIZE);
		cv::imshow("Ч��", g_srcImage);

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