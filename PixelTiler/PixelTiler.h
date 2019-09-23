#pragma once

#include <opencv2/opencv.hpp>

#include <vector>
#include <stdlib.h>
#include <time.h>

#include "TilingAlgorithm.h"

class PixelTiler
{
public:

	PixelTiler();

	void loadTilesetAndAlgorithm(const std::string& algoPath, const std::string& tilesetPath);
	
	void setColorLayerOrder(const std::vector<cv::Scalar>& colorLayers);

	void setColorLayerOrderManually();

	cv::Mat tilePixels(const std::string& inputFilePath);

	cv::Mat tilePixels(cv::Mat input);

	cv::Mat correctTiledLayers(cv::Size2i windowSize);

private:

	bool _tilesSet;
	bool _layerOrderSet;

	cv::Mat _tileset;
	TilingAlgorithm _algo;
	size_t _imgWidth, _imgHeight;

	std::vector<cv::Scalar> _layerOrder;
	std::vector<cv::Mat> _pixelLayers;

	void _reset();
	void _setColorLayersAutomatically(cv::Mat img);
	void _setPixelLayer(int id, cv::Mat img, const cv::Scalar& color, cv::Mat correction = cv::Mat());
	void _addTransparentLayer(cv::Mat bg, cv::Mat layer, cv::Point2i pos);
	cv::Mat _tintImage(cv::Mat input, cv::Scalar color);
	cv::Mat _buildImage();
};
