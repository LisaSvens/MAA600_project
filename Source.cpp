#include<opencv2/opencv.hpp>
#include<iostream>
#include<limits>
#include<set>
#include <algorithm>

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
	std::vector<int> intC; 
	int scaleOfObservation = 300; // = k, large k causes a preference for large components

	std::vector<std::vector<cv::Point2d>> segment(cv::Mat img, std::vector<edge> edges);
	void colour_img(cv::Mat img, std::vector<std::vector<cv::Point2d>> segmentation);
	ImageSegmentation(cv::Mat img); // constructor
private: 
	std::vector<std::vector<cv::Point2d>> segmentation_algorithm(std::vector<std::vector<cv::Point2d>> segmentation, int layer);
	bool equal(edge e1, edge e2);
	bool findInVector(std::vector<edge> edges, edge edgeToCheck);
	void sortEdgesByEdgeWeight(int layer);
	cv::Point2i inSame(std::vector<std::vector<cv::Point2d>> segmentation, cv::Point2d pt1, cv::Point2d pt2);
	double MInt(int c1size, int c2size, cv::Point2i pt);
	double tau(int componentSize);
	int recalculateIntC(std::vector<std::vector<cv::Point2d>> segmentation, int componentIndex, int layer);
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

void ImageSegmentation::sortEdgesByEdgeWeight(int layer)
{
	auto comparison = [layer](edge edge1, edge edge2)->bool 
	{
		return (edge1.weights[layer] < edge2.weights[layer]);
	};
	std::sort(edges.begin(), edges.end(), comparison);

	std::cout << "edges sorted \n";

	intC = std::vector<int>(edges.size(), 0);
	std::cout << "vector intC initialized \n";
}

cv::Point2i ImageSegmentation::inSame(std::vector<std::vector<cv::Point2d>> segmentation, cv::Point2d pt1, cv::Point2d pt2)
{
	int pt1Found, pt2Found, size;
	pt1Found = pt2Found = size = static_cast<int>(segmentation.size()) + 1;
	for (int i = 0; (pt1Found == size || pt2Found == size) && i < segmentation.size(); i++)
	{
		// finding first point
		if (pt1Found == size && std::find(segmentation[i].begin(), segmentation[i].end(), pt1) != segmentation[i].end())
			pt1Found = i;

		// finding second point
		if (pt2Found == size && std::find(segmentation[i].begin(), segmentation[i].end(), pt2) != segmentation[i].end())
			pt2Found = i;
	}

	// they are in the same component
	if (pt1Found == pt2Found != size)
		return cv::Point2i(-1, -1);
	// they are in disjoint components
	else
		return cv::Point2i(pt1Found, pt2Found);
}

double ImageSegmentation::tau(int componentSize)
{
	return scaleOfObservation / componentSize; 
}

double ImageSegmentation::MInt(int c1size, int c2size, cv::Point2i pt)
{
	return std::min(intC[pt.x] + tau(c1size), intC[pt.y] + tau(c2size));
}

int ImageSegmentation::recalculateIntC(std::vector<std::vector<cv::Point2d>> segmentation, int componentIndex, int layer)
{
	int maxIntC = 0; 
	for (std::vector<cv::Point2d>::iterator it = segmentation[componentIndex].begin(); it != segmentation[componentIndex].end(); ++it)
	{
		// for each element in the current component do the following: 

		std::vector<edge>::iterator edgeIt;
		// for finding the element where edges.endpoint[0] = *it
		for (edgeIt = edges.begin(); edgeIt != edges.end(); ++edgeIt)
			if (edgeIt->endpoints[0] == *it)
				break;
		// edgeIt pekar nu på det första elementet i edges som har .endpoints[0] lika med den punkten i komponenten vi studerar just nu
		// den går bara in till if-satsen om första ändpunkten är densamma som punkten vi tittar på just nu och om bägge ändpunkterna är i samma komponent och om dess vikt är större än maxIntC
		for (int i = 0; (edgeIt + i)->endpoints[0] != *it && inSame(segmentation, (edgeIt + i)->endpoints[0], (edgeIt + i)->endpoints[1]) == cv::Point2i(-1, -1) && maxIntC < (edgeIt + i)->weights[layer]; i++)
			maxIntC = (edgeIt + i)->weights[layer];
		// alla kanter som börjar i *it har nu jämförts 
	}
	return maxIntC;
}

std::vector<std::vector<cv::Point2d>> ImageSegmentation::segmentation_algorithm(std::vector<std::vector<cv::Point2d>> segmentation, int layer)
{
	// sort edges into vector of non-decreasing edge weight, blue layer
	sortEdgesByEdgeWeight(layer); 

	std::cout << "Before for-loop in segmentation algorithm \n";
	for (int q = 0; q < edges.size(); q++)
	{
		// if they are not in the same component, pt will contain the indexes of vector segmentation of the components where the points are located
		cv::Point2i pt = inSame(segmentation, edges[q].endpoints[0], edges[q].endpoints[1]);
		// if they are not in the same component, pt == (-1, -1) if they are in the same component
		if (pt != cv::Point2i(-1, -1) && edges[q].weights[layer] <= MInt(static_cast<int>(segmentation[pt.x].size()), static_cast<int>(segmentation[pt.y].size()), pt))
		{
			// if we get all the way here we want to merge the components in pt
			auto comparison = [](cv::Point2d pt1, cv::Point2d pt2)->bool
			{
				return (pt1.x < pt2.x || (pt1.x == pt2.x && pt1.y < pt2.y));
			};
			std::merge(segmentation[pt.x].begin(), segmentation[pt.x].end(), segmentation[pt.y].begin(), segmentation[pt.y].end(), segmentation[pt.x].begin(), comparison);
			// erase segmentation[pt.y] AND intC[pt.y]
			segmentation.erase(segmentation.begin() + pt.y);
			intC.erase(intC.begin() + pt.y);
			// recalculate intC[pt.x]
			intC[pt.x] = recalculateIntC(segmentation, pt.x, layer);
		}
	}
	std::cout << "Segmented a layer! \n";
	return segmentation;
}

// returns a list of sets of points (vertex positions) S = (C1, C2, C3, C4 ...), return S 
std::vector<std::vector<cv::Point2d>> ImageSegmentation::segment(cv::Mat img, std::vector<edge> edges)
{
	// do segmentation algorithm of each of the three imagesst
	segmentationB = segmentation_algorithm(segmentationB, 0);
	segmentationG = segmentation_algorithm(segmentationG, 1);
	segmentationR = segmentation_algorithm(segmentationR, 2);

	std::cout << "Done with the segmentation in the three layers! \n";
	std::cout << "Size of segmentationB: " << segmentationB.size() << "\n" << "Size of segmentationG: " << segmentationG.size() << "\n" << "Size of segmentationR: " << segmentationR.size() << "\n";
	
	std::vector<std::vector<cv::Point2d>> finalSegmentation(edges.size(), emptyVec); 
	/*
	std::vector<std::vector<cv::Point2d>>::iterator it;

	// make intersection function
	auto comparison = [](cv::Point2d pt1, cv::Point2d pt2)->bool
	{
		return (pt1.x < pt2.x || (pt1.x == pt2.x && pt1.y < pt2.y));
	};
	it = std::set_intersection(segmentationB.begin(), segmentationB.end(), segmentationG.begin(), segmentationG.end(), finalSegmentation.begin(), comparison);
	//std::set_intersection(finalSegmentation.begin(), finalSegmentation.end(), segmentationR.begin(), segmentationR.end(), finalSegmentation.begin(), comparison); 
	finalSegmentation.resize(it - finalSegmentation.begin());
	*/ 
	// do the intersection of the three segmentations -> final segmentation
	// return final segmentation
	return finalSegmentation;
}

void ImageSegmentation::colour_img(cv::Mat img, std::vector<std::vector<cv::Point2d>> segmentation)
{
	// colour each component of the segmentation in a distinct colour
}

int main()
{
	cv::Mat img = cv::imread("tiny_pic.png");
	cv::Mat imgBlurred; 

	std::cout << "Loaded image \n";
	
	// do gaussian blur on img here, 0.8 factor
	cv::GaussianBlur(img, imgBlurred, cv::Size(1, 1), 0.8, 0, cv::BORDER_DEFAULT);
	/*cv::imshow("Original image", img);
	cv::imshow("Blurred image", imgBlurred);*/

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