#include<opencv2/opencv.hpp>
#include<opencv2/core.hpp>
#include<opencv2/imgproc.hpp>

#include<iostream>
#include<vector>

using namespace std;
using namespace cv;

//声明全局阈值分割函数,输入图像，输出图像，T0，初始阈值T
void global_threshold_segmentation(Mat& input_image, Mat& output_image, int T0, int T);

int main()
{
	Mat image, image_gray, image_bw;
	image = imread("chip.png");
	if (image.empty())
	{
		cout << "读取图像出错" << endl;
		return -1;
	}

	cvtColor(image, image_gray, COLOR_BGR2GRAY);


	//1 在这里使用图像的平均值作为初始值T, T0=5
	Scalar image_meam = cv::mean(image_gray); //使用opencv的mean函数求平均值
	int T = (int)image_meam[0];//图像的平均值作为初始值T
	global_threshold_segmentation(image_gray, image_bw, 5, T);
	namedWindow("image_bw", 0);
	imshow("image_bw", image_bw);

	//2 使用任意值作为初始阈值
	//Mat image_bw2;
	//int T1 = 5; //任意值
	//global_threshold_segmentation(image_gray, image_bw2, 5, T1);
	//namedWindow("image_bw2", 0);
	//imshow("image_bw2", image_bw2);

	// 1 thresold
	//threshold(image_gray, image_bw, 100, 255, THRESH_BINARY);

	// 3 find contours
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(image_bw, contours, hierarchy,RETR_LIST,CHAIN_APPROX_SIMPLE);

	Mat mask = Mat::zeros(image_gray.size(), CV_8UC3);
	drawContours(mask, contours, -1, Scalar(0, 255, 255), 2);

	// 4 locate position
	// 5 count
	int area;
	int count = 0;
	
	for (int i = 0; i < contours.size(); i++) {
		area = contourArea(contours[i]);
		if (area < 700 || area>1000) continue;

		count++;
		//cout << "i:" << count<< " " << area << endl;
		
		Rect r = boundingRect(contours[i]);
		rectangle(mask, r, Scalar(255, 0, 0), 2);
		putText(mask, to_string(count), Point(r.x,r.y), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 255, 0), 1);
	}
	cout << "Test 1" << endl;
	cout << "Count: " << count << endl;

	imshow("image_gray", image_gray);
	imshow("image_out", mask);

	waitKey();

	return 0;
}

//定义全局阈值分割函数
void global_threshold_segmentation(Mat& input_image, Mat& output_image, int T0, int T)
{
	//使用初始值T进行分组并求每组的平均值m1和m2，并计算新的阈值T2
	int width = input_image.cols; //图像列数
	int height = input_image.rows; //图像行数
	uchar* Img = input_image.data; //图像指针
	int G1_mean, G2_mean; //定义每组像素的均值
	int G1_num = 1, G2_num = 1; //定义每组像素的数量，初始值设为1，以免后面出现除以0的问题
	int G1_sum = 0, G2_sum = 0; //定义每组灰度值之和

	for (int i = 0; i < height; i++)
	{
		uchar* Img = input_image.ptr(i); //图像每行数据的指针
		for (int j = 0; j < width; j++)
		{
			if (Img[j] < T)
			{
				G1_sum += Img[j];
				G1_num += 1;
			}
			else
			{
				G2_sum += Img[j];
				G2_num += 1;
			}
		}
	}
	G1_mean = G1_sum / G1_num;
	G2_mean = G2_sum / G2_num;
	int T2 = (G1_mean + G2_mean) * 0.5; //新阈值

	//迭代计算T
	if (abs(T2 - T) > T0)
	{
		global_threshold_segmentation(input_image, output_image, T0, T2);
	}
	else
	{
		threshold(input_image, output_image, T2, 255, 1);
	}

}
