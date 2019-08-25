#include "PixelTiler.h"

PixelTiler::PixelTiler()
{
	_reset();
}

void PixelTiler::loadTiles(std::string tilePrefix)
{
	//TODO
}

void PixelTiler::setCustomColorLayerOrder(std::list<cv::Scalar> colorLayers)
{
	//TODO
}

void PixelTiler::setCustomColorLayerOrder()
{
	//TODO
}

cv::Mat PixelTiler::tilePixels(std::string inputFilePath)
{
	//TODO
	return cv::Mat();
}

cv::Mat PixelTiler::tilePixels(cv::Mat input)
{
	//TODO
	return cv::Mat();
}

cv::Mat PixelTiler::correctTiledLayers()
{
	//TODO
	return cv::Mat();
}

void PixelTiler::_reset()
{
	_tilesSet = false;
	_layerOrderSet = false;

	_outerTile = cv::Mat();
	_innerTile = cv::Mat();
	_flatTile = cv::Mat();

	_layerOrder.clear();
	_pixelLayers.clear();
}
