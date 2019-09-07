#pragma once

#include <opencv2/opencv.hpp>

#include <vector>

class IPixelTiler
{
public:
	virtual void setColorLayerOrder(const std::vector<cv::Scalar>& colorLayers) = 0;
	virtual void setColorLayerOrderManually() = 0;
	virtual cv::Mat tilePixels(const std::string& inputFilePath) = 0;
	virtual cv::Mat tilePixels(cv::Mat input) = 0;
	virtual cv::Mat correctTiledLayers() = 0;
};
