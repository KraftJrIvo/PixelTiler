#include "TilingRuleReaction.h"

#include "TilingRule.h"

TilingRuleReaction::TilingRuleReaction(std::list<std::string> lines, cv::Mat tileset)
{
	std::string type = lines.front().substr(0, 3);
	std::string rest = lines.front().substr(3, lines.back().length() - 3);
	std::stringstream str;
	lines.pop_front();
	str << rest;
	if (type == "img")
	{
		int w, h, x, y;
		str >> w >> h >> x >> y;
		_img = ReactionImage(cv::Size(w, h), cv::Size(x, y));
		int tileW = tileset.cols / _img.tilesCount.width;
		int tileH = tileset.rows / _img.tilesCount.height;
		size = { tileW, tileH };
		_type = IMG;
	}
	else if (type == "pix")
	{
		str << lines.front();

		std::vector<std::string> vals((std::istream_iterator<WordDelimitedBySpace>(str)),
			std::istream_iterator<WordDelimitedBySpace>());

		int w = vals.size();
		int h = lines.size();

		cv::Mat pixels(h, w, CV_8UC4, cv::Scalar(0, 0, 0, 0));

		str = std::stringstream();
		for (auto& line : lines)
			str << line << " ";

		std::vector<std::string> allVals((std::istream_iterator<WordDelimitedBySpace>(str)),
			std::istream_iterator<WordDelimitedBySpace>());

		for (int i = 0; i < h; ++i)
			for (int j = 0; j < w; ++j)
			{
				auto& pix = pixels.at<cv::Vec4b>(i, j);
				auto val = allVals[i * w + j];
				if (val == "+")
					pix = cv::Scalar(255, 255, 255, 255);
				else if (val == "?")
					pix = cv::Scalar(0, 0, 0, 127);
				else if (val == "=")
					pix = cv::Scalar(127, 127, 127, 255);
				else if (val != "-")
					pix = cv::Scalar(val[0] - '0', 255, 255, 254);
			}

		_pix = ReactionPixels(pixels);
		size = { w, h };
		_type = PIX;
	}
}

cv::Mat TilingRuleReaction::getResult(cv::Mat tileset)
{
	if (_type == IMG)
	{
		int tileW = tileset.cols / _img.tilesCount.width;
		int tileH = tileset.rows / _img.tilesCount.height;
		int tileX = tileW * _img.tileCoords.width;
		int tileY = tileH * _img.tileCoords.height;
		return tileset(cv::Rect(tileX, tileY, tileW, tileH));
	}
	return _pix.pixels.clone();
}