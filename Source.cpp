#include<opencv2/opencv.hpp>
#include<iostream>
#include<limits>

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

	bool equal(edge e1, edge e2);
	bool findInVector(std::vector<edge> edges, edge edgeToCheck);
	void segment(cv::Mat img, std::vector<edge> edges);
	ImageSegmentation(cv::Mat img); // constructor
private: 

};

ImageSegmentation::ImageSegmentation(cv::Mat img) : edges (img.size().height * img.size().width * 4, emptyEdge), imgRef (img)
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
		}
	);
	// vector edges should now have all of the relevant edges with corresponding weights

	std::cout << "Size of vector of edges before erasing: " << edges.size() << "\n";

	// go through vector edges and remove all edges that have weight -1, they are empty 
	// old, takes forever
	cv::Vec3i tempVec = cv::Vec3i(0, 0, 0);
	for (std::vector<edge>::iterator it = edges.begin(); it != edges.end(); )
	{
		if (it->weights == tempVec)
		{
			std::cout << "inside if \n";
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

//std::vector<std::set<cv::Point2d>> segmentation_algorithm(cv::Mat img_layer, std::vector<edge> edges)
void segmentation_algorithm(cv::Mat img_layer, std::vector<edge> edges)
{
	// segmentation algorithm and stuff
	// no forEach since it can not be done parallell 
}

// returns a list of sets of points (vertex positions) S = (C1, C2, C3, C4 ...) return S 
//std::vector<std::set<cv::Point2d>> segment(cv::Mat img, std::vector<edge> edges)
void ImageSegmentation::segment(cv::Mat img, std::vector<edge> edges)
{
	// do segmentation algorithm of each of the three images
	// get an array of the three segmentations, std::vector<std::set<cv::Point2d>>[3], one for each of the layers

	// do the intersection of the three segmentations -> final segmentation
	// return the final segmentation
}

void colour_img(cv::Mat img, std::vector<std::set<cv::Point2d>> segmentation)
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
	// edges now contains all of the edges for the image

	std::cout << "Size of image: " << img.size() << "\n";
	
	// do segmentation 
	//std::vector<std::set<cv::Point2d>> segmentation = segment(img, edges);

	// "colour" and show the image
	//colour_img(img, segmentation); 

	cv::waitKey(0);
	return 0;
}