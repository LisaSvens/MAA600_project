#include<opencv2/opencv.hpp>
#include<iostream>
#include<limits>

#include"lib.h"

typedef struct edge
{
	cv::Point2d endpoints[2];
	cv::Vec3f weights = cv::Vec3f(0.0f, 0.0f, 0.0f); // in order b,g,r
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

ImageSegmentation::ImageSegmentation(cv::Mat img) : edges (img.size().height * img.size().width, emptyEdge), imgRef (img)
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
		
		// height = this->imgRef.row
		// width = this->imgRef.col
		// h = position[0]
		// w = position[1]
		
		// up
		temp.endpoints[1].x = position[0] - 1;
		temp.endpoints[1].y = position[1];
		if (temp.endpoints[1].x >= 0 && temp.endpoints[1].x <= this->imgRef.rows && temp.endpoints[1].y >= 0 && temp.endpoints[1].y <= this->imgRef.cols && !findInVector(edges, temp))
			edges[position[0] * imgRef.cols + position[1]] = temp;
		
		//// diag.up.right
		temp.endpoints[1].x = position[0] - 1;
		temp.endpoints[1].y = position[1] + 1;
		if (temp.endpoints[1].x >= 0 && temp.endpoints[1].x <= this->imgRef.rows && temp.endpoints[1].y >= 0 && temp.endpoints[1].y <= this->imgRef.cols && !findInVector(edges, temp))
			edges[position[0] * imgRef.cols + position[1]] = temp;
		
		//// right
		temp.endpoints[1].x = position[0];
		temp.endpoints[1].y = position[1] + 1;
		if (temp.endpoints[1].x >= 0 && temp.endpoints[1].x <= this->imgRef.rows && temp.endpoints[1].y >= 0 && temp.endpoints[1].y <= this->imgRef.cols && !findInVector(edges, temp))
			edges[position[0] * imgRef.cols + position[1]] = temp;
		
		//// diag.down.right
		temp.endpoints[1].x = position[0] + 1;
		temp.endpoints[1].y = position[1] + 1;
		if (temp.endpoints[1].x >= 0 && temp.endpoints[1].x <= this->imgRef.rows && temp.endpoints[1].y >= 0 && temp.endpoints[1].y <= this->imgRef.cols && !findInVector(edges, temp))
			edges[position[0] * imgRef.cols + position[1]] = temp;
		
		//// down
		temp.endpoints[1].x = position[0] + 1;
		temp.endpoints[1].y = position[1];
		if (temp.endpoints[1].x >= 0 && temp.endpoints[1].x <= this->imgRef.rows && temp.endpoints[1].y >= 0 && temp.endpoints[1].y <= this->imgRef.cols && !findInVector(edges, temp))
			edges[position[0] * imgRef.cols + position[1]] = temp;
		
		//// diag.down.left
		temp.endpoints[1].x = position[0] + 1;
		temp.endpoints[1].y = position[1] - 1;
		if (temp.endpoints[1].x >= 0 && temp.endpoints[1].x <= this->imgRef.rows && temp.endpoints[1].y >= 0 && temp.endpoints[1].y <= this->imgRef.cols && !findInVector(edges, temp))
			edges[position[0] * imgRef.cols + position[1]] = temp;
		
		//// left
		temp.endpoints[1].x = position[0];
		temp.endpoints[1].y = position[1] - 1;
		if (temp.endpoints[1].x >= 0 && temp.endpoints[1].x <= this->imgRef.rows && temp.endpoints[1].y >= 0 && temp.endpoints[1].y <= this->imgRef.cols && !findInVector(edges, temp))
			edges[position[0] * imgRef.cols + position[1]] = temp;
		
		//// diag.up.left
		temp.endpoints[1].x = position[0] - 1;
		temp.endpoints[1].y = position[1] - 1;
		if (temp.endpoints[1].x >= 0 && temp.endpoints[1].x <= this->imgRef.rows && temp.endpoints[1].y >= 0 && temp.endpoints[1].y <= this->imgRef.cols && !findInVector(edges, temp))
			edges[position[0] * imgRef.cols + position[1]] = temp;

		// weight all of the edges in all of the respective layers, less iterating to do it all at once
		}
	);
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

	std::cout << "Loaded image \n";

	// create an array of edges
	ImageSegmentation segmentation(img); 
	// edges now contains all of the edges for the image

	std::cout << "Size of vector of edges: " << segmentation.edges.size() << "\n";

	// do gaussian blur on img here, 0.8 factor

	// do segmentation 
	//std::vector<std::set<cv::Point2d>> segmentation = segment(img, edges);

	// "colour" and show the image
	//colour_img(img, segmentation); 

	cv::waitKey(0);
	return 0;
}