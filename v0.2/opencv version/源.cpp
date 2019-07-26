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
	float prewittx[9] ={
		-1,0,1,
		-1,0,1,
		-1,0,1
	};
	float prewitty[9] ={
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
	for (int i = 0; i < ogimg.rows; i++){
		for (int j = 0; j < ogimg.cols; j++){
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
	erode(dst, dst, structure_element);//腐蚀
	return dst;
}
//霍夫变换
Mat houghi(Mat ogimg,Mat ogoimg) {
	/*vector<Vec4f> line_data;
	HoughLinesP(ogimg, line_data, 1, CV_PI / 180.0, 80, 10, 20);
	//把得到的直线显示在图中
	Scalar color = Scalar(255, 0, 0);
	for (size_t i = 0; i < line_data.size(); i++) {
		Vec4f temp = line_data[i];
		line(ogoimg, Point(temp[0], temp[1]), Point(temp[2], temp[3]), color, 2);
	}
	return ogoimg;*/
	vector<Vec2f> lines;
	HoughLines(ogimg, lines, 1, CV_PI / 5, 10);
	std::vector<Vec2f>::const_iterator it = lines.begin();
	while (it != lines.end())
	{
		float rho = (*it)[0];
		float theta = (*it)[1];
		if (theta < CV_PI / 4 || theta > 3 * CV_PI / 4){
			//画垂直线
			Point pt1(rho / cos(theta), 0);
			Point pt2((rho - ogimg.rows * sin(theta)) / cos(theta), ogimg.rows);
			line(ogoimg, pt1, pt2, Scalar(0, 0, 255), 1, LINE_AA);
		}
		else{
			//画水平线
			Point pt1(0, rho / sin(theta));
			Point pt2(ogimg.cols, (rho - ogimg.cols * cos(theta)) / sin(theta));
			line(ogoimg, pt1, pt2, Scalar(0, 0, 255), 1, LINE_AA);
		}
		++it;
	}
	return ogoimg;
}

int main(){
	//读取图像
	Mat img = imread("D:\\opencvtest\\03.jpg");
	if (img.empty()){
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
	//膨胀后腐蚀
	Mat dilateimg = dilatei(ptimg);
	//hough变换检测直线
	Mat houimg = houghi(dilateimg,img);
	//显示
	namedWindow("opencv test", CV_WINDOW_AUTOSIZE);
	imshow("opencv test", houimg);
	imshow("imgorg", gryimg);
	waitKey(0);
	return 0;
}