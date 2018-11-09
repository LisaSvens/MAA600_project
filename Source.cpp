#include<opencv2/opencv.hpp>
#include<iostream>
#include<limits>
#include<set>
#include<algorithm>
#include<time.h>
#include<stdlib.h>

typedef cv::Point3_<uint8_t> Pixel;

struct edge
{
	edge()
		: weights(-1, -1, -1)
	{}

	cv::Point2i endpoints[2];
	cv::Vec3i weights; // in order b,g,r
};

class ImageSegmentation
{
public: 
	cv::Mat &imgRef;
	std::vector<edge> edges;
	edge emptyEdge;
	std::vector<std::vector<cv::Point2i>> segmentationB;
	std::vector<std::vector<cv::Point2i>> segmentationG;
	std::vector<std::vector<cv::Point2i>> segmentationR;
	std::vector<cv::Point2i> emptyVec; 
	std::vector<int> intC;		// to keep track of the internal difference of each component
	std::vector<int> compInd;	// to keep track of which component each pixel is in
	double scaleOfObservation = 45.0; // = k, large k causes a preference for large components

	std::vector<std::vector<cv::Point2i>> segment(cv::Mat img, std::vector<edge> edges);		// function called by user to segment an image
	cv::Mat colour_img(cv::Mat img, const std::vector<std::vector<cv::Point2i>>& segmentation);	// colours the components in different colours and presents
	ImageSegmentation(cv::Mat& img); // constructor
private: 
	std::vector<std::vector<cv::Point2i>> segmentation_algorithm(std::vector<std::vector<cv::Point2i>> segmentation, int layer);	// segmentation algorithm is in here
	void sortEdgesByEdgeWeight(int layer);
	double MInt(int c1size, int c2size, cv::Point2i pt);
	double tau(int componentSize);
	int recalculateIntC(const std::vector<std::vector<cv::Point2i>>& segmentation, int componentIndex, int layer);
};

// constructor
ImageSegmentation::ImageSegmentation(cv::Mat& img) : edges (img.size().height * img.size().width * 4, emptyEdge), imgRef (img), segmentationB (img.cols * img.rows, emptyVec), segmentationG(img.cols * img.rows, emptyVec), segmentationR(img.cols * img.rows, emptyVec), compInd(img.cols * img.rows)
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
		segmentationB[pos] = segmentationG[pos] = segmentationR[pos] = std::vector<cv::Point2i>(1, temp.endpoints[0]);
		compInd[pos] = pos;
		}
	);
	// vector edges should now have all of the relevant edges with corresponding weights

	/*for (int i = 0; i < edges.size(); i++)
		std::cout << "i = " << i << " edges = " << edges[i].endpoints[0] << ", " << edges[i].endpoints[1] << "\n";*/

	std::cout << "Size of vector of edges before erasing: " << edges.size() << "\n";
	// go through vector edges and remove all edges that have weight 0, they are empty 
	for (std::vector<edge>::iterator it = edges.begin(); it != edges.end(); )
	{
		if (it->endpoints[0] == it->endpoints[1])
		{
			it = edges.erase(it);
		}
		else 
			++it;
	}
	std::cout << "Size of vector of edges after erasing: " << edges.size() << "\n";
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

double ImageSegmentation::tau(int componentSize)
{
	return scaleOfObservation / static_cast<double>(componentSize); 
}

double ImageSegmentation::MInt(int c1size, int c2size, cv::Point2i pt)
{
	return std::min(intC[pt.x] + tau(c1size), intC[pt.y] + tau(c2size));
}

int ImageSegmentation::recalculateIntC(const std::vector<std::vector<cv::Point2i>>& segmentation, int componentIndex, int layer)
{
	int maxIntC = 0; 
	for (std::vector<cv::Point2i>::const_iterator it = segmentation[componentIndex].cbegin(); it != segmentation[componentIndex].cend(); ++it)
	{
		// for each element in the current component do the following: 

		std::vector<edge>::iterator edgeIt;
		// for finding the element where edges.endpoint[0] = *it
		for (edgeIt = edges.begin(); edgeIt != edges.end() && edgeIt->endpoints[0] != *it; ++edgeIt);
		// edgeIt pekar nu på det första elementet i edges som har .endpoints[0] lika med den punkten i komponenten vi studerar just nu

		// den går bara in till for-loopen om första ändpunkten är densamma som punkten vi tittar på just nu och om bägge ändpunkterna är i samma komponent och om dess vikt är större än maxIntC
		for (int i = 0; (edgeIt + i) != edges.end() && (edgeIt + i)->endpoints[0] == *it; i++)
		{
			cv::Point2i pt1(edges[edgeIt + i - edges.begin()].endpoints[0]);
			cv::Point2i pt2(edges[edgeIt + i - edges.begin()].endpoints[1]);
			if (compInd[imgRef.cols*pt1.x + pt1.y] == compInd[imgRef.cols*pt2.x + pt2.y] && maxIntC < (edgeIt + i)->weights[layer])
				maxIntC = (edgeIt + i)->weights[layer];
		}
		// alla kanter som börjar i *it har nu jämförts 
	}
	return maxIntC;
}

std::vector<std::vector<cv::Point2i>> ImageSegmentation::segmentation_algorithm(std::vector<std::vector<cv::Point2i>> segmentation, int layer)
{
	// sort edges into vector of non-decreasing edge weight, blue layer
	sortEdgesByEdgeWeight(layer); 

	for (int i = 0; i < compInd.size(); ++i)
		compInd[i] = i;

	int insideIf = 0;
	std::cout << "Before for-loop in segmentation algorithm \n";
	for (int q = 0; q < edges.size(); q++)
	{
		cv::Point2i pt1(edges[q].endpoints[0]);
		cv::Point2i pt2(edges[q].endpoints[1]);
		
		int compIndPt1 = compInd[imgRef.cols*pt1.x + pt1.y];
		int compIndPt2 = compInd[imgRef.cols*pt2.x + pt2.y];
		
		if (compIndPt1 != compIndPt2 && edges[q].weights[layer] <= MInt(static_cast<int>(segmentation[compIndPt1].size()), static_cast<int>(segmentation[compIndPt2].size()), cv::Point2i(compIndPt1, compIndPt2)))
		{
			insideIf++;
			// if we get all the way here we want to merge the components in pt
			auto comparison = [](cv::Point2i pt1, cv::Point2i pt2)->bool
			{
				return (pt1.x < pt2.x || (pt1.x == pt2.x && pt1.y < pt2.y));
			};
			
			int min, max;
			if (segmentation[compIndPt1].size() <= segmentation[compIndPt2].size())
			{
				min = compIndPt1;
				max = compIndPt2;
			}
			else
			{
				min = compIndPt2;
				max = compIndPt1;
			}
			std::vector<cv::Point2i> tempVec(segmentation[compIndPt1].size() + segmentation[compIndPt2].size(), cv::Point2i(-1, -1));

			for (auto it = segmentation[max].begin(); it != segmentation[max].end(); ++it)
				compInd[imgRef.cols*it->x + it->y] = compInd[min];

			std::merge(segmentation[compIndPt1].begin(), segmentation[compIndPt1].end(), segmentation[compIndPt2].begin(), segmentation[compIndPt2].end(), tempVec.begin(), comparison);
			segmentation[min].swap(tempVec);
			
			// clear segmentation[max] AND set intC[max] to 0
			segmentation[max].clear();
			intC[max] = 0;
			
			// recalculate intC[min]
			intC[min] = recalculateIntC(segmentation, min, layer);
		}
	}
	std::cout << "Segmented a layer! insideIf = " << insideIf << "\n";
	return segmentation;
}

// returns a list of sets of points (vertex positions) S = (C1, C2, C3, C4 ...), return S 
std::vector<std::vector<cv::Point2i>> ImageSegmentation::segment(cv::Mat img, std::vector<edge> edges)
{
	// do segmentation algorithm of each of the three imagesst
	segmentationB = segmentation_algorithm(segmentationB, 0);
	/*segmentationG = segmentation_algorithm(segmentationG, 1);
	segmentationR = segmentation_algorithm(segmentationR, 2);*/

	std::cout << "Done with the segmentation in the three layers! \n";
	std::cout << "Size of segmentationB: " << segmentationB.size() << "\n" << "Size of segmentationG: " << segmentationG.size() << "\n" << "Size of segmentationR: " << segmentationR.size() << "\n";
	
	std::vector<std::vector<cv::Point2i>> finalSegmentation(edges.size(), emptyVec); 
	
	/*cv::Mat bluePart = colour_img(img, segmentationB);
	cv::Mat greenPart = colour_img(img, segmentationG);
	cv::Mat redPart = colour_img(img, segmentationR);

	cv::imwrite("bluePart.jpg", bluePart);
	cv::imwrite("greenPart.jpg", greenPart);
	cv::imwrite("redPart.jpg", redPart);

	cv::namedWindow("blue", cv::WINDOW_NORMAL);
	cv::imshow("blue", bluePart);
	cv::namedWindow("green", cv::WINDOW_NORMAL);
	cv::imshow("green", greenPart);
	cv::namedWindow("red", cv::WINDOW_NORMAL);
	cv::imshow("red", redPart);
	cv::waitKey(0);*/

	cv::Mat huePart = colour_img(img, segmentationB);
	cv::imwrite("huePart.jpg", huePart);
	cv::namedWindow("hue", cv::WINDOW_NORMAL);
	cv::imshow("hue", huePart);

	return finalSegmentation;
}

cv::Mat ImageSegmentation::colour_img(cv::Mat img, const std::vector<std::vector<cv::Point2i>>& segmentation)
{
	// colour each component of the segmentation in a distinct colour
	srand(time(NULL));
	cv::Mat imgSeg;
	img.copyTo(imgSeg);
	for (int i = 0; i < segmentation.size(); ++i)
	{
		int blue = rand() % 256;
		int green = rand() % 256;
		int red = rand() % 256;
		/*std::cout << "Colour vector = " << cv::Vec3b(blue, green, red) << "\n";
		std::cout << imgSeg.channels() << "\n";*/
		for (int j = 0; j < segmentation[i].size(); ++j)
			imgSeg.at<cv::Vec3b>(segmentation[i][j].x, segmentation[i][j].y) = cv::Vec3b(blue, green, red);
	}

	return imgSeg;
}

int main()
{
	cv::Mat img = cv::imread("red_flower.jpg");
	cv::Mat imgSmaller, imgBlurred, hsv; 

	std::cout << "Loaded image \n";
	
	// do gaussian blur on img here, 0.8 factor
	cv::GaussianBlur(img, imgBlurred, cv::Size(7, 7), 0.8, 0, cv::BORDER_DEFAULT);
	//cv::imshow("Original image", img);
	//cv::imshow("Blurred image", imgBlurred);

	cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);

	cv::resize(hsv, imgSmaller, cv::Size2i(0, 0), 0.2, 0.2);

	cv::namedWindow("resized", cv::WINDOW_NORMAL);
	cv::imshow("resized", imgSmaller);
	std::cout << "Size of resized image = " << imgSmaller.size() << "\n";
	cv::waitKey(0);

	// create an array of edges
	ImageSegmentation segmentation(imgSmaller); 
	//ImageSegmentation segmentation(croppedImage);
	// edges now contains all of the edges with weights for the image

	//std::cout << "Size of image: " << img.size() << "\n";
	
	// do segmentation 
	std::vector<std::vector<cv::Point2i>> finalSegmentation = segmentation.segment(imgSmaller, segmentation.edges);
	//std::vector<std::vector<cv::Point2i>> finalSegmentation = segmentation.segment(croppedImage, segmentation.edges);

	cv::waitKey(0);
	return 0;
}