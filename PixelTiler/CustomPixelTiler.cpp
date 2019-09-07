#include "CustomPixelTiler.h"

CustomPixelTiler::CustomPixelTiler()
{
	_reset();
}

void CustomPixelTiler::loadTilesetAndAlgorithm(const std::string& algoPath, const std::string& tilesetPath)
{
	_reset();
	if (!tilesetPath.empty())
		_tileset = cv::imread(tilesetPath, CV_LOAD_IMAGE_UNCHANGED);
	_algo = TilingAlgorithm(algoPath, _tileset);
}

void CustomPixelTiler::setColorLayerOrder(const std::vector<cv::Scalar>& colorLayers)
{
	_layerOrder = colorLayers;
	_layerOrderSet = true;
}

void CustomPixelTiler::setColorLayerOrderManually()
{
	std::vector<cv::Scalar> colorLayers;
	//TODO
	setColorLayerOrder(colorLayers);
}

cv::Mat CustomPixelTiler::tilePixels(const std::string& inputFilePath)
{
	cv::Mat img = cv::imread(inputFilePath, CV_LOAD_IMAGE_UNCHANGED);

	// Making sure the input image has right amount of channels (4).
	switch (img.channels())
	{
	case 1:
		cv::cvtColor(img, img, CV_GRAY2BGRA);
		break;
	case 3:
		cv::cvtColor(img, img, CV_BGR2BGRA);
		break;
	default:
		break;
	}

	return tilePixels(img);
}

cv::Mat CustomPixelTiler::tilePixels(cv::Mat input)
{
	_imgWidth = input.cols;
	_imgHeight = input.rows;

	if (!_layerOrderSet)
		_setColorLayersAutomatically(input);

	const size_t& sz = _layerOrder.size();
	for (int i = 0; i < sz; ++i)
		_setPixelLayer(i, input, _layerOrder[i]);

	return _buildImage();
}

cv::Mat CustomPixelTiler::correctTiledLayers()
{
	//TODO
	return cv::Mat();
}

void CustomPixelTiler::_reset()
{
	_tilesSet = false;
	_layerOrderSet = false;
	_tintNeeded = false;

	_tileset = cv::Mat();
	_algo = TilingAlgorithm();
	_imgWidth = _imgHeight = 0;

	_layerOrder.clear();
}

void CustomPixelTiler::_setColorLayersAutomatically(cv::Mat img)
{
	std::vector<cv::Scalar> layerOrder;

	const size_t& w = img.cols;
	const size_t& h = img.rows;

	for (int i = 0; i < h; ++i)
		for (int j = 0; j < w; ++j)
		{
			auto bgra = img.at<cv::Vec4b>(i, j);

			if (uchar(bgra[0]) != uchar(bgra[1]) || uchar(bgra[1]) != uchar(bgra[2]))
				_tintNeeded = true;

			if (bgra[3] == 0)
				continue;

			cv::Scalar rgb = CV_RGB(bgra[2], bgra[1], bgra[0]);

			bool found = false;
			const size_t& sz = layerOrder.size();
			for (int ii = 0; ii < sz; ++ii)
				if (uchar(layerOrder[ii][0]) == uchar(bgra[0]) &&
					uchar(layerOrder[ii][1]) == uchar(bgra[1]) &&
					uchar(layerOrder[ii][2]) == uchar(bgra[2]))
				{
					found = true;
					break;
				}

			if (!found || !layerOrder.size())
				layerOrder.push_back(rgb);
		}
	setColorLayerOrder(layerOrder);
}

void CustomPixelTiler::_setPixelLayer(int id, cv::Mat img, const cv::Scalar& color, cv::Mat correction)
{
	const size_t& w = img.cols;
	const size_t& h = img.rows;

	cv::Mat pixelLayer(h, w, CV_8UC4, cv::Scalar(0,0,0,0));

	for (int i = 0; i < h; ++i)
		for (int j = 0; j < w; ++j)
		{
			auto bgra = img.at<cv::Vec4b>(i, j);
			cv::Scalar rgb = CV_RGB(bgra[2], bgra[1], bgra[0]);
			if (uchar(color[0]) == uchar(rgb[0]) &&
				uchar(color[1]) == uchar(rgb[1]) &&
				uchar(color[2]) == uchar(rgb[2]))
				pixelLayer.at<cv::Vec4b>(i, j) = cv::Scalar(255, 255, 255, 255);
		}

	if (id < _pixelLayers.size())
		_pixelLayers[id] = pixelLayer;
	else
		_pixelLayers.push_back(pixelLayer);
}

void CustomPixelTiler::_addTransparentLayer(cv::Mat bg, cv::Mat layer, cv::Point2i pos) {
	cv::Mat mask;
	std::vector<cv::Mat> layers;

	split(layer, layers); // seperate channels
	cv::Mat rgb[3] = { layers[0],layers[1],layers[2] };
	mask = layers[3]; // png's alpha channel used as mask
	layer.copyTo(bg.rowRange(pos.y, pos.y + layer.rows).colRange(pos.x, pos.x + layer.cols), mask);
}

cv::Mat CustomPixelTiler::_tintImage(cv::Mat input, cv::Scalar color)
{
	cv::Mat result = input.clone();
	const size_t& w = input.cols;
	const size_t& h = input.rows;

	for (int i = 0; i < h; ++i)
		for (int j = 0; j < w; ++j)
		{
			auto curVec = result.at<cv::Vec4b>(i, j);
			float ratioB = float(color[0]) / 255.0f;
			float ratioG = float(color[1]) / 255.0f;
			float ratioR = float(color[2]) / 255.0f;
			result.at<cv::Vec4b>(i, j) = cv::Vec4b(ratioB * curVec[0], ratioG * curVec[1], ratioR * curVec[2], curVec[3]);
		}

	return result;
}

cv::Mat CustomPixelTiler::_buildImage()
{
	cv::Mat result;

	const size_t& nLayers = _layerOrder.size();

	for (int z = 0; z < nLayers; ++z)
	{
		cv::Mat layer = _algo.apply(_pixelLayers[z]);

		if (_tintNeeded)
			layer = _tintImage(layer, _layerOrder[z]);

		if (result.empty())
			result = layer.clone();
		else
			_addTransparentLayer(result, layer, cv::Point2i(0, 0));
	}

	return result;
}
