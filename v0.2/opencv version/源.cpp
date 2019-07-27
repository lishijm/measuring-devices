#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

//灰度转换
Mat gryi(Mat ogimg) {
	int row = ogimg.rows;
	int col = ogimg.cols;
	Mat gryimg = Mat(row, col, CV_8UC1);//构造mat矩阵单通道用于存放灰度图

	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			uchar b = ogimg.at<Vec3b>(i, j)[0];//注意rgb的存放顺序
			uchar g = ogimg.at<Vec3b>(i, j)[1];
			uchar r = ogimg.at<Vec3b>(i, j)[2];
			gryimg.at<uchar>(i, j) = r * 0.299 + g * 0.587 + b * 0.114;//灰度转换公式
		}
	}
	return gryimg;
}
//prewitt算子边缘检测
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
		line(ogoimg, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(186, 88, 255), 1, LINE_AA);

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
Mat cannyi(Mat ogimg) {
	Mat dst, dst2;
	Canny(ogimg, dst, 50, 200, 3);
	cvtColor(dst, dst2, CV_GRAY2BGR);
	return dst;
}

int main() {
	//读取图像
	Mat img = imread("D:\\opencvtest\\03.jpg");
	if (img.empty()) {
		cout << "fail to load image !" << endl;
		return -1;
	}
	//灰度转换，每个像素挨个转换？
	Mat gryimg = gryi(img);
	//中值滤波
	Mat mbimg;
	medianBlur(gryimg, mbimg, 3);//数字为孔径参数，为大于1的奇数
	//直方图均衡化
	Mat blimg;
	equalizeHist(mbimg, blimg);
	//边缘检测
	Mat ptimg = prewitt(blimg);
	Mat canimg = cannyi(blimg);
	//膨胀后腐蚀
	Mat dilateimg = dilatei(ptimg);
	Mat dilateimg2 = dilatei(canimg);
	//hough变换检测直线
	Mat houimg = houghi(dilateimg2, img);
	//显示
	namedWindow("opencv test", CV_WINDOW_AUTOSIZE);
	imshow("opencv test", houimg);
	imshow("imgorg", gryimg);
	imshow("canimg", dilateimg2);
	waitKey(0);
	return 0;
}