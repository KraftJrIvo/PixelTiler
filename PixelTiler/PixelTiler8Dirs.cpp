#include "PixelTiler8Dirs.h"

PixelTiler8Dirs::PixelTiler8Dirs()
{
	_reset();
}

void PixelTiler8Dirs::loadTileset(const std::string& tilesetPath)
{
	_reset();
	_tileset = cv::imread(tilesetPath, cv::IMREAD_UNCHANGED);
	assert(_tileset.cols % 4 == 0 && _tileset.rows % 4 == 0, "Tiles must have sides which are divisible by 4!");
	_tileWidth = _tileset.cols / 4;
	_tileHeight = _tileset.rows / 4;
}

void PixelTiler8Dirs::setColorLayerOrder(const std::vector<cv::Scalar>& colorLayers)
{
	_layerOrder = colorLayers;
	_layerOrderSet = true;
}

void PixelTiler8Dirs::setColorLayerOrderManually()
{
	std::vector<cv::Scalar> colorLayers;
	//TODO
	setColorLayerOrder(colorLayers);
}

cv::Mat PixelTiler8Dirs::tilePixels(const std::string& inputFilePath)
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

cv::Mat PixelTiler8Dirs::tilePixels(cv::Mat input)
{
	_imgWidth = input.cols;
	_imgHeight = input.rows;

	if (!_layerOrderSet)
		_setColorLayersAutomatically(input);

	const size_t& sz = _layerOrder.size();
	for (int i = 0; i < sz; ++i)
		_setTileLayer(i, input, _layerOrder[i]);
	
	return _buildImage();
}

cv::Mat PixelTiler8Dirs::correctTiledLayers()
{
	//TODO
	return cv::Mat();
}

void PixelTiler8Dirs::_reset()
{
	_tilesSet = false;
	_layerOrderSet = false;
	_tintNeeded = false;

	_tileset = cv::Mat();
	_tileWidth = _tileHeight = 0;
	_imgWidth = _imgHeight = 0;

	_layerOrder.clear();
	_tileLayers.clear();
}

void PixelTiler8Dirs::_setColorLayersAutomatically(cv::Mat img)
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

void PixelTiler8Dirs::_setTileLayer(int id, cv::Mat img, const cv::Scalar& color, cv::Mat correction)
{
	const size_t& w = img.cols;
	const size_t& h = img.rows;
	const size_t hx2 = 2 * h;
	const size_t wx2 = 2 * w;

	std::vector<std::vector<uchar>> pixelLayer(hx2, std::vector<uchar>(wx2));

	std::vector<std::vector<TileDescription>>
		tileLayer(hx2, std::vector<TileDescription>());
	for (int i = 0; i < hx2; ++i)
		tileLayer[i].resize(wx2, std::pair<PixelTileType, TileDirection>(NO_TYPE, NO_DIR));

	for (int i = 0; i < h; ++i)
		for (int j = 0; j < w; ++j)
		{
			int ix2 = 2 * i;
			int jx2 = 2 * j;

			auto bgra = img.at<cv::Vec4b>(i, j);
			cv::Scalar rgb = CV_RGB(bgra[2], bgra[1], bgra[0]);

			if (uchar(color[0]) == uchar(rgb[0]) &&
				uchar(color[1]) == uchar(rgb[1]) &&
				uchar(color[2]) == uchar(rgb[2]))
			{
				pixelLayer[ix2][jx2] = pixelLayer[ix2 + 1][jx2] = pixelLayer[ix2][jx2 + 1] = pixelLayer[ix2 + 1][jx2 + 1] = OUTER;
				tileLayer[ix2][jx2] = tileLayer[ix2 + 1][jx2] = tileLayer[ix2][jx2 + 1] = tileLayer[ix2 + 1][jx2 + 1] =
					std::pair<PixelTileType, TileDirection>(FULL, NO_DIR);
			}
			else
			{
				pixelLayer[ix2][jx2] = pixelLayer[ix2 + 1][jx2] = pixelLayer[ix2][jx2 + 1] = pixelLayer[ix2 + 1][jx2 + 1] = NO_TYPE;
				tileLayer[ix2][jx2] = tileLayer[ix2 + 1][jx2] = tileLayer[ix2][jx2 + 1] = tileLayer[ix2 + 1][jx2 + 1] =
					std::pair<PixelTileType, TileDirection>(NO_TYPE, NO_DIR);
			}
		}

	for (int i = 0; i < hx2; ++i)
		for (int j = 0; j < wx2; ++j)
		{
			if (pixelLayer[i][j] == OUTER || (i == 0) || (j == 0) || (j == wx2 - 1) || (i == hx2 - 1))
				continue;

			bool north = (pixelLayer[i - 1][j] == OUTER);
			bool west = (pixelLayer[i][j - 1] == OUTER);
			bool south = (pixelLayer[i + 1][j] == OUTER);
			bool east = (pixelLayer[i][j + 1] == OUTER);

			if ((north && (west || east)) || (south && (west || east)))
				pixelLayer[i][j] = INNER;
		}

	for (int i = 0; i < hx2; ++i)
		for (int j = 0; j < wx2; ++j)
		{
			if ((i == 0) || (j == 0) || (j == wx2 - 1) || (i == hx2 - 1))
				continue;

			uchar northVal = pixelLayer[i - 1][j];
			uchar westVal = pixelLayer[i][j - 1];
			uchar southVal = pixelLayer[i + 1][j];
			uchar eastVal = pixelLayer[i][j + 1];
			bool north = (northVal == OUTER);
			bool west = (westVal == OUTER);
			bool south = (southVal == OUTER);
			bool east = (eastVal == OUTER);

			if (!north && !west && !south && !east)
				continue;
			if (north && west && south && east)
			{
				//tileLayer[i][j].first = FULL;
				continue;
			}

			TileDirection dir = NO_DIR;

			if (northVal == OUTER && southVal == OUTER)
				if (westVal == OUTER)
					dir = E;
				else
					dir = W;
			else if (westVal == OUTER && eastVal == OUTER)
				if (northVal == OUTER)
					dir = S;
				else
					dir = N;
			if (dir == NO_DIR)
			{
				if (west)
				{
					if (north)
						dir = NW;
					else if (south)
						dir = SW;
				}
				else if (east)
				{
					if (north)
						dir = NE;
					else if (south)
						dir = SE;
				}
			}
			else
			{
				if (north + east + west + south == 3 && (northVal && eastVal && westVal && southVal))
					tileLayer[i][j].first = FULL;
				else
				{
					tileLayer[i][j].second = dir;
					tileLayer[i][j].first = FLAT;
				}
				continue;
			}

			if (dir != NO_DIR)
			{
				if (dir > W)
					tileLayer[i][j].first = (PixelTileType)pixelLayer[i][j];
				tileLayer[i][j].second = dir;
			}
		}

	if (id < _tileLayers.size())
		_tileLayers[id] = tileLayer;
	else
		_tileLayers.push_back(tileLayer);
}

void PixelTiler8Dirs::_addTransparentLayer(cv::Mat bg, cv::Mat layer, cv::Point2i pos) {
	cv::Mat mask;
	std::vector<cv::Mat> layers;

	split(layer, layers); // seperate channels
	cv::Mat rgb[3] = { layers[0],layers[1],layers[2] };
	mask = layers[3]; // png's alpha channel used as mask
	//merge(rgb, 3, transp);  // put together the RGB channels, now transp insn't transparent 
	layer.copyTo(bg.rowRange(pos.y, pos.y + layer.rows).colRange(pos.x, pos.x + layer.cols), mask);
}

cv::Rect PixelTiler8Dirs::getRectFromDiagDir(TileDescription& descr)
{
	auto type = descr.first;
	auto dir = descr.second;
	if (type == FULL)
		return cv::Rect(_tileWidth * 2, 0, _tileWidth, _tileHeight);
	if (dir == NO_DIR)
		return cv::Rect(0, 0, 0, 0);
	if (dir < NW)
		switch (dir)
		{
		case N:
			return cv::Rect(_tileWidth, 0, _tileWidth, _tileHeight);
		case E:
			return cv::Rect(_tileWidth * 3, _tileHeight, _tileWidth, _tileHeight);
		case S:
			return cv::Rect(_tileWidth, _tileHeight * 3, _tileWidth, _tileHeight);
		case W:
			return cv::Rect(0, _tileHeight, _tileWidth, _tileHeight);
		}
	dir = TileDirection(int(dir) - 4);
	if (type == OUTER)
		return cv::Rect((1 - (dir % 2)) * _tileWidth * 3, (1 - (dir / 2)) * _tileHeight * 3, _tileWidth, _tileHeight);
	return cv::Rect(_tileWidth + (dir % 2) * _tileWidth, _tileHeight + (dir / 2) * _tileHeight, _tileWidth, _tileHeight);
}

cv::Mat PixelTiler8Dirs::_tintTileset(cv::Scalar color)
{
	cv::Mat result = _tileset.clone();
	const size_t& w = _tileset.cols;
	const size_t& h = _tileset.rows;
	
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

cv::Mat PixelTiler8Dirs::_buildImage()
{
	cv::Mat result(_imgHeight * _tileHeight * 2, _imgWidth * _tileWidth * 2, CV_8UC4, cv::Scalar(0, 0, 0, 0));

	const size_t& nLayers = _layerOrder.size();

	const size_t wx2 = _imgWidth * 2;
	const size_t hx2 = _imgHeight * 2;

	for (int z = 0; z < nLayers; ++z)
	{
		cv::Mat usingTileset;
		auto color = _layerOrder[z];
		if (_tintNeeded)
			usingTileset = _tintTileset(color);
		else
			usingTileset = _tileset;

		cv::Mat layer(_imgHeight * _tileHeight * 2, _imgWidth * _tileWidth * 2, CV_8UC4, cv::Scalar(0, 0, 0, 0));
		for (int i = 0; i < hx2; ++i)
			for (int j = 0; j < wx2; ++j)
			{
				cv::Rect tileRect = getRectFromDiagDir(_tileLayers[z][i][j]);
				if (tileRect.width > 0)
				{
					cv::Mat tile = usingTileset(tileRect);
					cv::Point2i p(j * _tileWidth, i * _tileHeight);
					tile.copyTo(layer(cv::Rect(p.x, p.y, _tileWidth, _tileHeight)));
				}
			}
		_addTransparentLayer(result, layer, cv::Point2i(0, 0));
	}

	return result;
}
