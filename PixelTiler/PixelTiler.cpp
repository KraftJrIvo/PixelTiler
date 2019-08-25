#include "PixelTiler.h"

PixelTiler::PixelTiler()
{
	_reset();
}

void PixelTiler::loadTiles(const std::string& tilePrefix)
{
	std::string outerName = tilePrefix + "_outer.png";
	std::string innerName = tilePrefix + "_inner.png";
	std::string flatName = tilePrefix + "_flat.png";
	_outerTile = cv::imread(outerName, CV_LOAD_IMAGE_UNCHANGED);
	_innerTile = cv::imread(innerName, CV_LOAD_IMAGE_UNCHANGED);
	_flatTile = cv::imread(flatName, CV_LOAD_IMAGE_UNCHANGED);
}

void PixelTiler::setColorLayerOrder(const std::list<cv::Scalar>& colorLayers)
{
	_layerOrder = colorLayers;
}

void PixelTiler::setColorLayerOrderManually()
{
	std::list<cv::Scalar> colorLayers;
	//TODO
	setColorLayerOrder(colorLayers);
}

cv::Mat PixelTiler::tilePixels(const std::string& inputFilePath)
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
