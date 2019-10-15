#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include<Windows.h> 

using namespace cv;
using namespace std;
//串口
HANDLE hComm;
LPCWSTR pStr = L"COM4";
char lpOutbuffer[100];
DWORD dwbyte = 100;
Mat srcImage, grayImage, tempImage1, tempImage, imageROI, grayprev;
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

// 灰度转换
// Mat gryi(Mat ogimg) {
// 	int row = ogimg.rows;
// 	int col = ogimg.cols;
// 	Mat gryimg = Mat(row, col, CV_8UC1);//构造mat矩阵单通道用于存放灰度图

// 	for (int i = 0; i < row; i++) {
// 		for (int j = 0; j < col; j++) {
// 			uchar b = ogimg.at<Vec3b>(i, j)[0];//注意rgb的存放顺序
// 			uchar g = ogimg.at<Vec3b>(i, j)[1];
// 			uchar r = ogimg.at<Vec3b>(i, j)[2];
// 			gryimg.at<uchar>(i, j) = r * 0.299 + g * 0.587 + b * 0.114;//灰度转换公式
// 		}
// 	}
// 	return gryimg;
// }
// prewitt算子边缘检测
Mat prewitt(Mat ogimg) {
	float prewittx[9] = {
		-1,0,1,
		-1,0,1,
		-1,0,1
	};
	float prewitty[9] = {
		1,1,1,
		0,0,0,
		-1,-1,-1
	};
	Mat px = Mat(3, 3, CV_32F, prewittx);
	cout << px << endl;
	Mat py = Mat(3, 3, CV_32F, prewitty);
	cout << py << endl;
	Mat dstx = Mat(ogimg.size(), ogimg.type(), ogimg.channels());
	Mat dsty = Mat(ogimg.size(), ogimg.type(), ogimg.channels());
	Mat dst = Mat(ogimg.size(), ogimg.type(), ogimg.channels());
	filter2D(ogimg, dstx, ogimg.depth(), px);
	filter2D(ogimg, dsty, ogimg.depth(), py);
	float tempx, tempy, temp;
	for (int i = 0; i < ogimg.rows; i++) {
		for (int j = 0; j < ogimg.cols; j++) {
			tempx = dstx.at<uchar>(i, j);
			tempy = dsty.at<uchar>(i, j);
			temp = sqrt(tempx * tempx + tempy * tempy);
			dst.at<uchar>(i, j) = temp;
		}
	}
	return dst;
}
//膨胀函数
Mat dilatei(Mat ogimg) {
	Mat dst;
	Mat structure_element = getStructuringElement(MORPH_RECT, Size(3, 3)); //设置膨胀/腐蚀的核为矩形，大小为3*3
	dilate(ogimg, dst, structure_element); //膨胀
	//erode(dst, dst, structure_element);//腐蚀
	return dst;
}
//霍夫变换
Mat houghi(Mat ogimg, Mat ogoimg) {

	vector<Vec4i>lines;
	HoughLinesP(ogimg, lines, 1, CV_PI / 180, 180, 50, 10);

	//一次在图中绘制出每条线段
	for (size_t i = 0; i < lines.size(); i++)
	{
		Vec4i l = lines[i];
		line(ogoimg, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 1, LINE_AA);

	}/*
	vector<Vec2f>lines;
	HoughLines(ogimg, lines, 1, CV_PI / 180, 150, 0, 0);
	for (size_t i = 0; i < lines.size(); i++){
		float rho = lines[i][0];
		float theta = lines[i][1];
		Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a * rho, y0 = b * rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		line(ogoimg, pt1, pt2, Scalar(55, 100, 195), 1, LINE_AA);

	}*/
	return ogoimg;
}
//canny算子
Mat cannyi(Mat ogimg) {
	Mat dst, dst2;
	Canny(ogimg, dst, 50, 100);
	cvtColor(dst, dst2, CV_GRAY2BGR);
	return dst;
}

int main() {
	//读取图像
	Mat img = imread("D:\\opencvtest\\01.jpg");
	//if (img.empty()) {
	//	cout << "fail to load image !" << endl;
	//	return -1;
	//}
	////读取串口信息
	//hComm = CreateFile(pStr, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, NULL);
	//if (hComm == INVALID_HANDLE_VALUE)
	//{
	//	cout << "FLASE";
	//	return -1;
	//}
	//else
	//{
	//	cout << "TURE";
	//}
	//DCB dcb;
	//GetCommState(hComm, &dcb);
	//dcb.BaudRate = 9600;
	//dcb.ByteSize = 8;
	//dcb.Parity = NOPARITY;
	//dcb.StopBits = TWOSTOPBITS;
	//bool set = SetCommState(hComm, &dcb);
	//bool sup = SetupComm(hComm, 1024, 1024);

	//灰度转换，每个像素挨个转换？
	Mat gryimg;
	cvtColor(img, gryimg,COLOR_BGR2GRAY);
	//高斯滤波
	Mat gsimg;
	GaussianBlur(gryimg, gsimg, Size(7, 7), 0);//数字为孔径参数，为大于1的奇数
	//直方图均衡化
	Mat blimg;
	equalizeHist(gsimg, blimg);
	//边缘检测
	Mat ptimg = prewitt(blimg);
	Mat canimg = cannyi(blimg);
	//隔开物体
	Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
	//膨胀后腐蚀
	Mat dilateimg = dilatei(ptimg);
	Mat dilateimg2 = dilatei(canimg);
	//hough变换检测直线
	Mat houimg = houghi(dilateimg2, img);
	//显示
	namedWindow("opencv test", CV_WINDOW_AUTOSIZE);
	imshow("opencv test", houimg);
	imshow("imgorg", gsimg);
	imshow("canimg", dilateimg2);
	waitKey(0);
	return 0;
}