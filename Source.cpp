#include<opencv2/opencv.hpp>
#include<iostream>
#include<limits>



int main()
{
	cv::Mat img = cv::imread("purple_flower.jpg");
	//cv::namedWindow("image", cv::WINDOW_NORMAL);
	//cv::imshow("image", img);
	
	cv::Vec3b intensity = img.at<cv::Vec3b>(400, 1000);
	uchar blue = intensity.val[0];
	uchar green = intensity.val[1];
	uchar red = intensity.val[2];

	//std::cout << "Blue: " << static_cast<int>(blue) << "\nGreen: " << static_cast<int>(green) << "\nRed: " << static_cast<int>(red) << "\nSize: " << img.size;

	cv::Mat bgr[3];   //destination array
	cv::split(img, bgr);//split source  
	
	cv::namedWindow("blue.png", cv::WINDOW_NORMAL);
	cv::namedWindow("green.png", cv::WINDOW_NORMAL);
	cv::namedWindow("red.png", cv::WINDOW_NORMAL);
	//Note: OpenCV uses BGR color order
	cv::imshow("blue.png", bgr[0]); //blue channel
	cv::imshow("green.png", bgr[1]); //green channel
	cv::imshow("red.png", bgr[2]); //red channel

	std::numeric_limits<float>::max();

	cv::waitKey(0);
	return 0;
}