#include "PixelTiler.h"

PixelTiler::PixelTiler()
{
	_reset();
}

void PixelTiler::loadTilesetAndAlgorithm(const std::string& algoPath, const std::string& tilesetPath)
{
	_reset();
	if (!tilesetPath.empty())
		_tileset = cv::imread(tilesetPath, cv::IMREAD_UNCHANGED);
	_algo = TilingAlgorithm(algoPath, _tileset);
}

void PixelTiler::setColorLayerOrder(const std::vector<cv::Scalar>& colorLayers)
{
	_layerOrder = colorLayers;
	_layerOrderSet = true;
}

void PixelTiler::setColorLayerOrderManually()
{
	std::vector<cv::Scalar> colorLayers;
	//TODO
	setColorLayerOrder(colorLayers);
}

cv::Mat PixelTiler::tilePixels(const std::string& inputFilePath)
{
	cv::Mat img = cv::imread(inputFilePath, cv::IMREAD_UNCHANGED);

	// Making sure the input image has right amount of channels (4).
	switch (img.channels())
	{
	case 1:
		cv::cvtColor(img, img, cv::COLOR_GRAY2BGRA);
		break;
	case 3:
		cv::cvtColor(img, img, cv::COLOR_BGR2BGRA);
		break;
	default:
		break;
	}

	return tilePixels(img);
}

cv::Mat PixelTiler::tilePixels(cv::Mat input)
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

cv::Mat PixelTiler::correctTiledLayers(cv::Size2i windowSize)
{
	cv::Mat lastImage = _buildImage();

	float horRatio = float(lastImage.cols)/float(windowSize.width);
	float verRatio = float(lastImage.rows)/float(windowSize.height);

	int width = (verRatio > horRatio) ?
		(lastImage.cols * windowSize.height / lastImage.rows) : windowSize.width;
	int height = (verRatio <= horRatio) ?
		(lastImage.rows * windowSize.width / lastImage.cols) : windowSize.height;
	int xPos = (windowSize.width - width) / 2;
	int yPos = (windowSize.height - height) / 2;
	cv::Rect resultRect(xPos, yPos, width, height);
	cv::Mat curImg(windowSize.height + 100, windowSize.width, CV_8UC4, cv::Scalar(0, 0, 0, 255));

	int curLayer = 0;
	auto& layer = _pixelLayers[curLayer];
	cv::Size2f cursorSize = { float(width) / float(layer.cols), float(height) / float(layer.rows) };

	typedef std::tuple<cv::Rect, cv::Size2f, cv::Point2i, bool, bool, int, int> MouseData;

	auto onMouse = [](int event, int x, int y, int, void* _data)
	{
		static bool lmb = false;
		if (event == cv::EVENT_LBUTTONDOWN) lmb = true;
		if (event == cv::EVENT_LBUTTONUP) lmb = false;

		static bool rmb = false;
		if (event == cv::EVENT_RBUTTONDOWN) rmb = true;
		if (event == cv::EVENT_RBUTTONUP) rmb = false;

		MouseData& data = *((MouseData*)_data);
		const cv::Rect& rect = std::get<0>(data);
		const cv::Size2f& cursorSize = std::get<1>(data);

		float layerX = (x - rect.x) / cursorSize.width;
		float layerY = (y - rect.y) / cursorSize.height;
		
		if (x > rect.x && x < rect.x + rect.width && y > rect.y && y < rect.y + rect.height)
		{
			if (event == cv::EVENT_MOUSEMOVE)
				std::get<2>(data) = cv::Point2i(layerX, layerY);
			std::get<3>(data) = (lmb || rmb);
			std::get<4>(data) = (lmb);
		}

		if (y > rect.height && lmb)
		{
			int layerID = x / 100;
			if (layerID < std::get<5>(data))
				std::get<6>(data) = layerID;
		}
	};

	MouseData data(resultRect, cursorSize, cv::Point2f(0,0), false, false, _layerOrder.size(), curLayer);

	int updateRadius = 2;

	while (true)
	{
		curLayer = std::get<6>(data);
		auto& curLayerMat = _pixelLayers[curLayer];
		if (std::get<3>(data))
		{
			auto& pt = std::get<2>(data);
			if (pt.x >= 0 && pt.y >= 0 && pt.x < curLayerMat.cols && pt.y < curLayerMat.rows)
			{
				bool val = std::get<4>(data);
				auto& pixel = curLayerMat.at<cv::Vec4b>(pt.y, pt.x);
				if ((pixel[3] == 0 && val) || (pixel[3] > 0 && !val))
				{
					pixel = val ? cv::Scalar(255, 255, 255, 255) : cv::Scalar(0, 0, 0, 0);

					lastImage = _buildImagePart(pt, 1);
				}
			}
		}
		
		curImg.setTo(cv::Scalar(0, 0, 0, 0));
		cv::resize(lastImage, curImg(resultRect), resultRect.size(), 0, 0, cv::InterpolationFlags::INTER_NEAREST);
		cv::Point2i cursorPos = { 
			int(resultRect.x + std::get<2>(data).x * cursorSize.width),
			int(resultRect.y + std::get<2>(data).y * cursorSize.height)
		};
		cv::rectangle(curImg, cv::Rect(cursorPos.x, cursorPos.y, cursorSize.width, cursorSize.height), cv::Scalar(0, 0, 255, 255));
		const size_t& sz = _layerOrder.size();
		for (int i = 0; i < sz; ++i)
			cv::rectangle(curImg, cv::Rect(i * 100, windowSize.height, 100, 100), _layerOrder[i], -1);
		cv::rectangle(curImg, cv::Rect(curLayer * 100, windowSize.height, 100, 100), cv::Scalar(0, 0, 255, 255));
		cv::imshow("correction", curImg);
		cv::setMouseCallback("correction", onMouse, &data);
		int key = cv::waitKey(10);
		if (key == 13)
			break;
	}

	return lastImage;
}

void PixelTiler::_reset()
{
	_tilesSet = false;
	_layerOrderSet = false;

	_tileset = cv::Mat();
	_algo = TilingAlgorithm();
	_imgWidth = _imgHeight = 0;

	_layerOrder.clear();

	srand(time(NULL));
}

void PixelTiler::_setColorLayersAutomatically(cv::Mat img)
{
	std::vector<cv::Scalar> layerOrder;

	const size_t& w = img.cols;
	const size_t& h = img.rows;

	for (int i = 0; i < h; ++i)
		for (int j = 0; j < w; ++j)
		{
			auto bgra = img.at<cv::Vec4b>(i, j);

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

void PixelTiler::_setPixelLayer(int id, cv::Mat img, const cv::Scalar& color, cv::Mat correction)
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

void PixelTiler::_addTransparentLayer(cv::Mat bg, cv::Mat layer, cv::Point2i pos) {
	cv::Mat mask;
	std::vector<cv::Mat> layers;

	split(layer, layers); // seperate channels
	cv::Mat rgb[3] = { layers[0],layers[1],layers[2] };
	mask = layers[3]; // png's alpha channel used as mask
	layer.copyTo(bg.rowRange(pos.y, pos.y + layer.rows).colRange(pos.x, pos.x + layer.cols), mask);
}

cv::Mat PixelTiler::_tintImage(cv::Mat input, cv::Scalar color)
{
	cv::Mat result = input.clone();
	const size_t& w = input.cols;
	const size_t& h = input.rows;

	for (int i = 0; i < h; ++i)
		for (int j = 0; j < w; ++j)
		{
			auto curVec = result.at<cv::Vec4b>(i, j);
			if (curVec[0] == curVec[1] && curVec[1] == curVec[2])
			{
				float ratioB = float(color[0]) / 255.0f;
				float ratioG = float(color[1]) / 255.0f;
				float ratioR = float(color[2]) / 255.0f;
				result.at<cv::Vec4b>(i, j) = cv::Vec4b(ratioB * curVec[0], ratioG * curVec[1], ratioR * curVec[2], curVec[3]);
			}
		}

	return result;
}

cv::Mat PixelTiler::_buildImage()
{
	cv::Mat result;

	const size_t& nLayers = _layerOrder.size();
	for (int z = 0; z < nLayers; ++z)
	{
		cv::Mat layer = _algo.apply(_pixelLayers[z]);

		layer = _tintImage(layer, _layerOrder[z]);

		if (result.empty())
			result = layer.clone();
		else
			_addTransparentLayer(result, layer);
	}

	_lastResult = result.clone();

	return result;
}

cv::Mat PixelTiler::_buildImagePart(const cv::Point2i& px, int radius)
{
	if (_lastResult.empty())
		return _buildImage();

	radius += 1;

	cv::Rect partRect;
	partRect.x = std::max(px.x - radius, 0);
	partRect.y = std::max(px.y - radius, 0);
	partRect.width = std::min(px.x + radius, _pixelLayers[0].cols - 1) - px.x + radius + 1 + (px.x - radius < 0 ? px.x - radius : 0);
	partRect.height = std::min(px.y + radius, _pixelLayers[0].rows - 1) - px.y + radius + 1 + (px.y - radius < 0 ? px.y - radius : 0);

	cv::Rect cutSides;
	cutSides.x = std::max(radius - abs((px.x - radius < 0) ? px.x - radius : 0) - 1, 0);
	cutSides.y = std::max(radius - abs((px.y - radius < 0) ? px.y - radius : 0) - 1, 0);
	cutSides.width = 1 + radius - ((px.x + radius - 1 >= _pixelLayers[0].cols) ? std::min(px.x + radius + 1 - _pixelLayers[0].cols, radius - 1) : 0) + (cutSides.x == 0 ? px.x - radius + 1 : 0);
	cutSides.height = 1 + radius - ((px.y + radius - 1 >= _pixelLayers[0].rows) ? std::min(px.y + radius + 1 - _pixelLayers[0].rows, radius - 1) : 0) + (cutSides.y == 0 ? px.y - radius + 1 : 0);

	cv::Mat result = _lastResult;

	const size_t& nLayers = _layerOrder.size();
	for (int z = 0; z < nLayers; ++z)
	{
		cv::Mat layerPart = _pixelLayers[z](partRect);

		cv::Mat layerPartResult = _algo.apply(layerPart);
		cv::Size2i scaleOffset = { layerPartResult.cols / layerPart.cols, layerPartResult.rows / layerPart.rows };
		
		cv::Rect rectWithFrame = cv::Rect(scaleOffset.width * cutSides.x, scaleOffset.height * cutSides.y, scaleOffset.width * cutSides.width, scaleOffset.height * cutSides.height);
		layerPartResult = layerPartResult(rectWithFrame);

		layerPartResult = _tintImage(layerPartResult, _layerOrder[z]);

		cv::Rect resultPart(scaleOffset.width * (partRect.x + cutSides.x), scaleOffset.height * (partRect.y + cutSides.y), scaleOffset.width * cutSides.width, scaleOffset.height * cutSides.height);
		result(resultPart).setTo(cv::Scalar(0,0,0,0));
		_addTransparentLayer(result, layerPartResult, { resultPart.x, resultPart.y });
	}
	
	_lastResult = result.clone();

	return result;
}
