#pragma once

#include <opencv2/opencv.hpp>

#include "types.h"

struct ReactionImage
{
	ReactionImage() = default;
	ReactionImage(cv::Size _tilesCount, cv::Size _tileCoords) :
		tilesCount(_tilesCount),
		tileCoords(_tileCoords)
	{}
	cv::Size tilesCount;
	cv::Size tileCoords;
};

struct ReactionPixels
{
	ReactionPixels() = default;
	ReactionPixels(cv::Mat px) :
		pixels(px)
	{}
	cv::Mat pixels;
};

class TilingRuleReaction
{
	friend class TilingRule;
public:
	cv::Size2i size;

	TilingRuleReaction() = default;
	TilingRuleReaction(std::list<std::string> line, cv::Mat tileset);
	cv::Mat getResult(cv::Mat tileset);

private:
	TilingReactionType _type;
	ReactionImage _img;
	ReactionPixels _pix;
};