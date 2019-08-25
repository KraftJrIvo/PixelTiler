#pragma once

#include <opencv2/opencv.hpp>

#include <vector>

enum PixelTileType
{
	NO_TYPE,
	OUTER,
	INNER,
	FLAT,
	FULL
};

enum TileDirection
{
	N,
	E,
	S,
	W,
	NW,
	NE,
	SW,
	SE, 
	NO_DIR
};

typedef std::pair<PixelTileType, TileDirection> TileDescription;

class PixelTiler
{
public:

	PixelTiler();
	
	// This MUST be called before calling tilePixels(). 
	// Tileset must have side lengths which are divisible by 4 and be like this:
	// 
	//  /^^\
	//  |/\|
	//  |\/|
	//  \__/
	//
	void loadTileset(const std::string& tilesetPath);
	
	// Can be called optionally to set the order of color layers to be processed.
	// If not called the order is set automatically.
	void setColorLayerOrder(const std::vector<cv::Scalar>& colorLayers);
	
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
	bool _tintNeeded;
	bool _layerOrderSet;
	
	cv::Mat _tileset;
	size_t _tileWidth, _tileHeight;
	size_t _imgWidth, _imgHeight;

	std::vector<cv::Scalar> _layerOrder;
	std::vector<std::vector<std::vector<uchar>>> _pixelLayers;
	std::vector<std::vector<std::vector<TileDescription>>> _tileLayers;

	void _reset();
	void _setColorLayersAutomatically(cv::Mat img);
	void _setTileLayer(int id, cv::Mat img, const cv::Scalar& color, cv::Mat correction = cv::Mat());
	void _addTransparentLayer(cv::Mat bg, cv::Mat layer, cv::Point2i pos);
	cv::Rect getRectFromDiagDir(TileDescription&);
	cv::Mat _tintTileset(cv::Scalar color);
	cv::Mat _buildImage();
};
