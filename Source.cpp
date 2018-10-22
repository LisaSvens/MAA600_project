#include<opencv2/opencv.hpp>
#include<iostream>
#include<limits>
#include<set>

#include"lib.h"

typedef struct edge
{
	cv::Point2d endpoints[2];
	cv::Vec3i weights = cv::Vec3i(0, 0, 0); // in order b,g,r
} edge;

typedef cv::Point3_<uint8_t> Pixel;

class ImageSegmentation
{
public: 
	cv::Mat &imgRef;
	std::vector<edge> edges;
	edge emptyEdge;
	std::vector<std::vector<cv::Point2d>> segmentationB;
	std::vector<std::vector<cv::Point2d>> segmentationG;
	std::vector<std::vector<cv::Point2d>> segmentationR;
	std::vector<cv::Point2d> emptyVec; 

	std::vector<std::vector<cv::Point2d>> segment(cv::Mat img, std::vector<edge> edges);
	void colour_img(cv::Mat img, std::vector<std::vector<cv::Point2d>> segmentation);
	ImageSegmentation(cv::Mat img); // constructor
private: 
	std::vector<std::vector<cv::Point2d>> segmentation_algorithm(std::vector<std::vector<cv::Point2d>> segmentation);
	bool equal(edge e1, edge e2);
	bool findInVector(std::vector<edge> edges, edge edgeToCheck);

};

// constructor
ImageSegmentation::ImageSegmentation(cv::Mat img) : edges (img.size().height * img.size().width * 4, emptyEdge), imgRef (img), segmentationB (img.cols * img.rows, emptyVec), segmentationG(img.cols * img.rows, emptyVec), segmentationR(img.cols * img.rows, emptyVec)
{
	// initiate vector edges and fill it with all the edges and weight them and shit
	img.forEach<Pixel>
	(
		[this](Pixel &pixel, const int * position) -> void
		{
		// create all of the vertices 
		edge temp;
		temp.endpoints[0].x = position[0];
		temp.endpoints[0].y = position[1];
		cv::Vec3b intensity;
			
		int index = 0;
		for (int i = 0; i < 3; i++)
			for (int j = 1; j < 3; j++)
			{
				temp.endpoints[1].x = position[0] - 1 + i;
				temp.endpoints[1].y = position[1] - 1 + j;
				if (temp.endpoints[1].x >= 0 && temp.endpoints[1].x < this->imgRef.rows && temp.endpoints[1].y >= 0 && temp.endpoints[1].y < this->imgRef.cols && !(i == 1 && j == 1) && !(i == 2 && j == 1))
				{
					if(temp.endpoints[0].x <= temp.endpoints[1].x && temp.endpoints[0].y <= temp.endpoints[1].y)
					intensity = this->imgRef.at<cv::Vec3b>(temp.endpoints[1].x, temp.endpoints[1].y);
					temp.weights[0] = abs(static_cast<int>(pixel.x) - static_cast<int>(intensity.val[0]));
					temp.weights[1] = abs(static_cast<int>(pixel.x) - static_cast<int>(intensity.val[1]));
					temp.weights[2] = abs(static_cast<int>(pixel.x) - static_cast<int>(intensity.val[2]));
					edges[(position[0] * imgRef.cols + position[1]) * 4 + index] = temp;
					index++; 
				}
			}
		int pos = this->imgRef.cols * position[0] + position[1];
		segmentationB[pos] = segmentationG[pos] = segmentationR[pos] = std::vector<cv::Point2d>(1, temp.endpoints[0]);
		}
	);
	// vector edges should now have all of the relevant edges with corresponding weights

	std::cout << "Size of vector of edges before erasing: " << edges.size() << "\n";
	// go through vector edges and remove all edges that have weight 0, they are empty 
	cv::Vec3i tempVec = cv::Vec3i(0, 0, 0);
	for (std::vector<edge>::iterator it = edges.begin(); it != edges.end(); )
	{
		if (it->weights == tempVec)
		{
			it = edges.erase(it);
		}
		else 
			++it;
	}
	std::cout << "Size of vector of edges after erasing: " << edges.size() << "\n";
}

bool ImageSegmentation::equal(edge e1, edge e2)
{
	if (e1.endpoints[0] == e2.endpoints[0] && e1.endpoints[1] == e2.endpoints[1])
		return true;

	else if (e1.endpoints[0] == e2.endpoints[1] && e1.endpoints[1] == e2.endpoints[0])
		return true;

	else
		return false; 
}

bool ImageSegmentation::findInVector(std::vector<edge> edges, edge edgeToCheck)
{
	if (edges.size() == 0) return false; 

	for (std::vector<edge>::iterator it = edges.begin(); it != edges.end(); ++it)
		if (equal((*it), edgeToCheck))
			return true; 

	return false; 
}

std::vector<std::vector<cv::Point2d>> ImageSegmentation::segmentation_algorithm(std::vector<std::vector<cv::Point2d>> segmentation)
{
	// segmentation algorithm and stuff
	// no forEach since it can not be done parallell 

	//return a vector of sets of points
	std::vector<std::vector<cv::Point2d>> randomCrap;
	return randomCrap;
}

// returns a list of sets of points (vertex positions) S = (C1, C2, C3, C4 ...), return S 
std::vector<std::vector<cv::Point2d>> ImageSegmentation::segment(cv::Mat img, std::vector<edge> edges)
{
	// do segmentation algorithm of each of the three images

	std::vector<std::vector<std::vector<cv::Point2d>>> segmentations; 
	segmentationB = segmentation_algorithm(segmentationB);
	segmentationG = segmentation_algorithm(segmentationG);
	segmentationR = segmentation_algorithm(segmentationR);
	
	// make intersection function

	// do the intersection of the three segmentations -> final segmentation
	// return final segmentation
	segmentations.push_back(segmentationB);
	return segmentations[0];
}

void ImageSegmentation::colour_img(cv::Mat img, std::vector<std::vector<cv::Point2d>> segmentation)
{
	// colour each component of the segmentation in a distinct colour
}

int main()
{
	cv::Mat img = cv::imread("orange_flower_small.jpg");
	cv::Mat imgBlurred; 

	std::cout << "Loaded image \n";
	
	// do gaussian blur on img here, 0.8 factor
	cv::GaussianBlur(img, imgBlurred, cv::Size(3, 3), 0.8, 0, cv::BORDER_DEFAULT);
	cv::imshow("Original image", img);
	cv::imshow("Blurred image", imgBlurred);

	// create an array of edges
	ImageSegmentation segmentation(imgBlurred); 
	// edges now contains all of the edges with weights for the image

	std::cout << "Size of image: " << img.size() << "\n";
	
	// do segmentation 
	std::vector<std::vector<cv::Point2d>> finalSegmentation = segmentation.segment(imgBlurred, segmentation.edges);

	// "colour" and show the image
	//colour_img(img, segmentation); 

	cv::waitKey(0);
	return 0;
}