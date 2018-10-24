#include<opencv2/opencv.hpp>
#include<iostream>
#include<limits>
#include<set>
#include<algorithm>
#include<time.h>
#include<stdlib.h>

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
	int scaleOfObservation = 200; // = k, large k causes a preference for large components

	std::vector<std::vector<cv::Point2d>> segment(cv::Mat img, std::vector<edge> edges);
	void colour_img(cv::Mat img, const std::vector<std::vector<cv::Point2d>>& segmentation);
	ImageSegmentation(cv::Mat img); // constructor
private: 
	std::vector<std::vector<cv::Point2d>> segmentation_algorithm(std::vector<std::vector<cv::Point2d>> segmentation, int layer);
	bool equal(edge e1, edge e2);
	bool findInVector(std::vector<edge> edges, edge edgeToCheck);
	void sortEdgesByEdgeWeight(int layer);
	cv::Point2i inSame(const std::vector<std::vector<cv::Point2d>>& segmentation, cv::Point2d pt1, cv::Point2d pt2);
	double MInt(int c1size, int c2size, cv::Point2i pt);
	double tau(int componentSize);
	int recalculateIntC(const std::vector<std::vector<cv::Point2d>>& segmentation, int componentIndex, int layer);
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

cv::Point2i ImageSegmentation::inSame(const std::vector<std::vector<cv::Point2d>>& segmentation, cv::Point2d pt1, cv::Point2d pt2)
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
	if (pt1Found == pt2Found && pt1Found != size && pt2Found != size)
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

int ImageSegmentation::recalculateIntC(const std::vector<std::vector<cv::Point2d>>& segmentation, int componentIndex, int layer)
{
	int maxIntC = 0; 
	for (std::vector<cv::Point2d>::const_iterator it = segmentation[componentIndex].cbegin(); it != segmentation[componentIndex].cend(); ++it)
	{
		// for each element in the current component do the following: 

		//std::cout << "1 \n";
		std::vector<edge>::iterator edgeIt;
		// for finding the element where edges.endpoint[0] = *it
		for (edgeIt = edges.begin(); edgeIt != edges.end() && edgeIt->endpoints[0] != *it; ++edgeIt);
		// edgeIt pekar nu p� det f�rsta elementet i edges som har .endpoints[0] lika med den punkten i komponenten vi studerar just nu
		
		//if (edgeIt == edges.end())
		//	std::cout << "no occurence in edges \n";

		// den g�r bara in till for-loopen om f�rsta �ndpunkten �r densamma som punkten vi tittar p� just nu och om b�gge �ndpunkterna �r i samma komponent och om dess vikt �r st�rre �n maxIntC
		for (int i = 0; (edgeIt + i) != edges.end() && (edgeIt + i)->endpoints[0] == *it; i++)
		{
			//std::cout << "maxIntC = " << maxIntC << " (edgeIt + i)->weights[layer] = " << (edgeIt + i)->weights[layer] << "\n";
			//std::cout << "inSame = " << inSame(segmentation, (edgeIt + i)->endpoints[0], (edgeIt + i)->endpoints[1]) << "\n";
			if(inSame(segmentation, (edgeIt + i)->endpoints[0], (edgeIt + i)->endpoints[1]) == cv::Point2i(-1, -1) && maxIntC < (edgeIt + i)->weights[layer])
				maxIntC = (edgeIt + i)->weights[layer];
			//std::cout << "3 \n";
		}
		// alla kanter som b�rjar i *it har nu j�mf�rts 
	}
	return maxIntC;
}

std::vector<std::vector<cv::Point2d>> ImageSegmentation::segmentation_algorithm(std::vector<std::vector<cv::Point2d>> segmentation, int layer)
{
	// sort edges into vector of non-decreasing edge weight, blue layer
	sortEdgesByEdgeWeight(layer); 

	int insideIf = 0;

	std::cout << "Before for-loop in segmentation algorithm \n";
	for (int q = 0; q < edges.size(); q++)
	{
		// if they are not in the same component, pt will contain the indexes of vector segmentation of the components where the points are located
		cv::Point2i pt = inSame(segmentation, edges[q].endpoints[0], edges[q].endpoints[1]);
		// if they are not in the same component, pt == (-1, -1) if they are in the same component
		//std::cout << "pt = " << pt << "\n";
		//if(pt != cv::Point2i(-1, -1))
			//std::cout << "segmentation[pt.x].size() = " << segmentation[pt.x].size() << " segmentation[pt.y].size() = " << segmentation[pt.y].size() << "\n";
		if (pt != cv::Point2i(-1, -1) && edges[q].weights[layer] <= MInt(static_cast<int>(segmentation[pt.x].size()), static_cast<int>(segmentation[pt.y].size()), pt))
		{
			insideIf++;
			// if we get all the way here we want to merge the components in pt
			auto comparison = [](cv::Point2d pt1, cv::Point2d pt2)->bool
			{
				return (pt1.x < pt2.x || (pt1.x == pt2.x && pt1.y < pt2.y));
			};

			int min = std::min(segmentation[pt.x].size(), segmentation[pt.y].size());
			int max = std::max(segmentation[pt.x].size(), segmentation[pt.y].size());
			cv::Point2d tempPt(-1, -1);
			std::vector<cv::Point2d> tempVec(segmentation[pt.x].size() + segmentation[pt.y].size(), tempPt);
			std::vector<cv::Point2d>::iterator it = std::merge(segmentation[pt.x].begin(), segmentation[pt.x].end(), segmentation[pt.y].begin(), segmentation[pt.y].end(), tempVec.begin(), comparison);
			tempVec.resize(it - tempVec.begin());
			segmentation[min].swap(tempVec);
			// erase segmentation[pt.y] AND intC[pt.y]
			segmentation[max].clear();
			intC.erase(intC.begin() + max);
			//std::cout << "erased intC part \n";
			// recalculate intC[pt.x]
			intC[min] = recalculateIntC(segmentation, min, layer);
			//std::cout << "recalculated intC \n";
		}
	}
	std::cout << "Segmented a layer! insideIf = " << insideIf << "\n";
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
	
	// running this far didn't give an error :D 
	// no touchie touchie above this point

	cv::Mat bgr[3];   //destination array
	cv::split(img, bgr);//split source
	std::cout << "Blue segmentation \n";
	colour_img(bgr[0], segmentationB);

	// make intersection function
	
	std::cout << "finalSegmentation.size() = " << finalSegmentation.size() << "\n";

	return finalSegmentation;
}

void ImageSegmentation::colour_img(cv::Mat img, const std::vector<std::vector<cv::Point2d>>& segmentation)
{
	// colour each component of the segmentation in a distinct colour
	srand(time(NULL));
	cv::Mat imgSeg(img); 
	for (int i = 0; i < segmentation.size(); ++i)
	{
		int blue = rand() % 256;
		int green = rand() % 256;
		int red = rand() % 256;
		std::cout << "Colour vector = " << cv::Vec3b(blue, green, red) << "\n";

		for (int j = 0; j < segmentation[i].size(); ++j)
			imgSeg.at<cv::Vec3b>(segmentation[i][j].x, segmentation[i][j].y) = cv::Vec3b(blue, green, red);
	}

	//cv::imshow("image", imgSeg);
}

int main()
{
	cv::Mat img = cv::imread("tiny_pic.png");
	cv::Mat imgBlurred; 

	std::cout << "Loaded image \n";
	
	// do gaussian blur on img here, 0.8 factor
	cv::GaussianBlur(img, imgBlurred, cv::Size(3, 3), 0.8, 0, cv::BORDER_DEFAULT);
	//cv::imshow("Original image", img);
	//cv::imshow("Blurred image", imgBlurred);

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