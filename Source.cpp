#include<opencv2/opencv.hpp>
#include<iostream>
#include<limits>

#include"lib.h"

typedef struct edge
{
	cv::Point2d endpoints[2];
	cv::Vec3f weights = cv::Vec3b(0, 0, 0); // in order b,g,r
} edge;

bool equal(edge e1, edge e2)
{
	if (e1.endpoints[0] == e2.endpoints[0] && e1.endpoints[1] == e2.endpoints[1])
		return true;

	else if (e1.endpoints[0] == e2.endpoints[1] && e1.endpoints[1] == e2.endpoints[0])
		return true;

	else
		return false; 
}

bool findInVector(std::vector<edge> edges, edge edgeToCheck)
{
	if (edges.size() == 0) return false; 

	for (std::vector<edge>::iterator it = edges.begin(); it != edges.end(); ++it)
		if (equal((*it), edgeToCheck))
			return true; 

	return false; 
}

// create the initial edges from the image
std::vector<edge> createEdges(cv::Size size)
{
	std::vector<edge> edges;

	/*edge something; 
	edges.push_back(something); */

	int height = size.height;
	int width = size.width;

	// this will make it go row by row
	for (int h = 0; h < height; h++)
	{
		for (int w = 0; w < width; w++)
		{
			edge temp; 
			temp.endpoints[0].x = h;
			temp.endpoints[0].y = w;

			std::cout << "1 \n";

			// up
			temp.endpoints[1].x = h - 1;
			temp.endpoints[1].y = w;
			if (temp.endpoints[1].x >= 0 && temp.endpoints[1].x <= height && temp.endpoints[1].y >= 0 && temp.endpoints[1].y <= width && !findInVector(edges, temp))
				edges.push_back(temp);

			std::cout << "2 \n";

			//// diag.up.right
			temp.endpoints[1].x = h - 1;
			temp.endpoints[1].y = w + 1;
			if (temp.endpoints[1].x >= 0 && temp.endpoints[1].x <= height && temp.endpoints[1].y >= 0 && temp.endpoints[1].y <= width && !findInVector(edges, temp))
				edges.push_back(temp);

			//// right
			temp.endpoints[1].x = h;
			temp.endpoints[1].y = w + 1;
			if (temp.endpoints[1].x >= 0 && temp.endpoints[1].x <= height && temp.endpoints[1].y >= 0 && temp.endpoints[1].y <= width && !findInVector(edges, temp))
				edges.push_back(temp);

			//// diag.down.right
			temp.endpoints[1].x = h + 1;
			temp.endpoints[1].y = w + 1;
			if (temp.endpoints[1].x >= 0 && temp.endpoints[1].x <= height && temp.endpoints[1].y >= 0 && temp.endpoints[1].y <= width && !findInVector(edges, temp))
				edges.push_back(temp);

			//// down
			temp.endpoints[1].x = h + 1;
			temp.endpoints[1].y = w;
			if (temp.endpoints[1].x >= 0 && temp.endpoints[1].x <= height && temp.endpoints[1].y >= 0 && temp.endpoints[1].y <= width && !findInVector(edges, temp))
				edges.push_back(temp);

			//// diag.down.left
			temp.endpoints[1].x = h + 1;
			temp.endpoints[1].y = w - 1;
			if (temp.endpoints[1].x >= 0 && temp.endpoints[1].x <= height && temp.endpoints[1].y >= 0 && temp.endpoints[1].y <= width && !findInVector(edges, temp))
				edges.push_back(temp);

			//// left
			temp.endpoints[1].x = h;
			temp.endpoints[1].y = w - 1;
			if (temp.endpoints[1].x >= 0 && temp.endpoints[1].x <= height && temp.endpoints[1].y >= 0 && temp.endpoints[1].y <= width && !findInVector(edges, temp))
				edges.push_back(temp);

			//// diag.up.left
			temp.endpoints[1].x = h - 1;
			temp.endpoints[1].y = w - 1;
			if (temp.endpoints[1].x >= 0 && temp.endpoints[1].x <= height && temp.endpoints[1].y >= 0 && temp.endpoints[1].y <= width && !findInVector(edges, temp))
				edges.push_back(temp);

			std::cout << "w = " << w << "\n";
		}
		std::cout << "h = " << h << "\n";
	}

	std::cout << "4 \n";
	
	return edges;
}

void weight_edges(cv::Mat img, std::vector<edge> *edges)
{
	// weights stuff, obviously
	// forEach here
	// how to access individual elements: (*edges)[0].
}

//std::vector<std::set<cv::Point2d>> segmentation_algorithm(cv::Mat img_layer, std::vector<edge> edges)
void segmentation_algorithm(cv::Mat img_layer, std::vector<edge> edges)
{
	// segmentation algorithm and stuff
	// no forEach since it can not be done parallell 
}

// returns a list of sets of points (vertex positions) S = (C1, C2, C3, C4 ...) return S 
//std::vector<std::set<cv::Point2d>> segment(cv::Mat img, std::vector<edge> edges)
void segment(cv::Mat img, std::vector<edge> edges)
{
	// weight all of the edges in all of the respective layers, less iterating to do it all at once
	weight_edges(img, &edges);

	// split img into 3 "layers" of bgr parts
	cv::Mat bgr[3];   //destination array
	cv::split(img, bgr);//split source 
	// bgr[3] now has each of the three bgr layers 

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
	cv::Mat img = cv::imread("purple_flower.jpg");

	std::cout << "Loaded image \n";

	// create an array of edges
	std::vector<edge> edges = createEdges(img.size());
	// edges now contains all of the edges for the image

	std::cout << "Size of vector of edges: " << edges.size() << "\n";

	// do gaussian blur on img here, 0.8 factor

	// do segmentation 
	//std::vector<std::set<cv::Point2d>> segmentation = segment(img, edges);

	// "colour" and show the image
	//colour_img(img, segmentation); 

	cv::waitKey(0);
	return 0;
}