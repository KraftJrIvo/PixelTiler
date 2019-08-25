#pragma once

#include <opencv2/opencv.hpp>

#include <vector>

class PixelTiler
{
public:

	PixelTiler();
	
	// This MUST be called before calling tilePixels(). Tiles must be images of equal size, 
	// their side length must be divisible by 2. if image is grayscale it adopts the pixel color.
	// Files must be named: <prefix>_outer.png, <prefix>_inner.png, <prefix>_flat.png.
	void loadTiles(const std::string& tilePrefix);
	
	// Can be called optionally to set the order of color layers to be processed.
	// If not called the order is set automatically.
	void setColorLayerOrder(const std::list<cv::Scalar>& colorLayers);
	
	// This version lets the user pick layer order manually.
	void setColorLayerOrderManually();

	// This does the tiling and returns the resulting 3-channeled image.
	cv::Mat tilePixels(const std::string& inputFilePath);

	// This version uses an already loaded cv::Mat.
	cv::Mat tilePixels(cv::Mat input);

	// Lets user correct the tiling
	cv::Mat correctTiledLayers();

private:

	bool _tilesSet;
	bool _layerOrderSet;
	
	cv::Mat _outerTile;
	cv::Mat _innerTile;
	cv::Mat _flatTile;

	std::list<cv::Scalar> _layerOrder;
	std::vector<std::vector<std::vector<cv::Mat>>> _pixelLayers;

	void _reset();
};
