#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

//�Ҷ�ת��
Mat gryi(Mat ogimg) {
	int row = ogimg.rows;
	int col = ogimg.cols;
	Mat gryimg = Mat(row, col, CV_8UC1);//����mat����ͨ�����ڴ�ŻҶ�ͼ

	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			uchar b = ogimg.at<Vec3b>(i, j)[0];//ע��rgb�Ĵ��˳��
			uchar g = ogimg.at<Vec3b>(i, j)[1];
			uchar r = ogimg.at<Vec3b>(i, j)[2];
			gryimg.at<uchar>(i, j) = r * 0.299 + g * 0.587 + b * 0.114;//�Ҷ�ת����ʽ
		}
	}
	return gryimg;
}
//prewitt���ӱ�Ե���
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
//���ͺ���
Mat dilatei(Mat ogimg) {
	Mat dst;
	Mat structure_element = getStructuringElement(MORPH_RECT, Size(3, 3)); //��������/��ʴ�ĺ�Ϊ���Σ���СΪ3*3
	dilate(ogimg, dst, structure_element); //����
	//erode(dst, dst, structure_element);//��ʴ
	return dst;
}
//����任
Mat houghi(Mat ogimg, Mat ogoimg) {

	vector<Vec4i>lines;
	HoughLinesP(ogimg, lines, 1, CV_PI / 180, 180, 50, 10);

	//һ����ͼ�л��Ƴ�ÿ���߶�
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
	//��ȡͼ��
	Mat img = imread("D:\\opencvtest\\03.jpg");
	if (img.empty()) {
		cout << "fail to load image !" << endl;
		return -1;
	}
	//�Ҷ�ת����ÿ�����ذ���ת����
	Mat gryimg = gryi(img);
	//��ֵ�˲�
	Mat mbimg;
	medianBlur(gryimg, mbimg, 3);//����Ϊ�׾�������Ϊ����1������
	//ֱ��ͼ���⻯
	Mat blimg;
	equalizeHist(mbimg, blimg);
	//��Ե���
	Mat ptimg = prewitt(blimg);
	Mat canimg = cannyi(blimg);
	//���ͺ�ʴ
	Mat dilateimg = dilatei(ptimg);
	Mat dilateimg2 = dilatei(canimg);
	//hough�任���ֱ��
	Mat houimg = houghi(dilateimg2, img);
	//��ʾ
	namedWindow("opencv test", CV_WINDOW_AUTOSIZE);
	imshow("opencv test", houimg);
	imshow("imgorg", gryimg);
	imshow("canimg", dilateimg2);
	waitKey(0);
	return 0;
}