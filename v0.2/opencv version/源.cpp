#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;
using namespace std;
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
/*
Mat dilatei(Mat ogimg) {

}*/

int main() {
	//��ȡͼ��
	Mat img = imread("D:\\opencvtest\\00.jpg");
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
	//����
	//Mat dilateimg = dilatei(ptimg);
	//��ʾ
	namedWindow("opencv test", CV_WINDOW_AUTOSIZE);
	imshow("opencv test", ptimg);
	imshow("imgorg", gryimg);
	waitKey(0);
	return 0;
}