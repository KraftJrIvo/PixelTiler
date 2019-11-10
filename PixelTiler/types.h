#pragma once

#include <opencv2/opencv.hpp>

enum TilingConditionMetric
{
	ANY,
	NO,
	YES
};

enum TilingRuleRotation
{
	CONDROT_90,
	CONDROT_180,
	CONDROT_270
};

enum TilingReactionType
{
	NO_TYPE,
	IMG,
	PIX
};

enum TilingConditionOperator
{
	GREATER,
	LESS,
	EQUALS,
	GREATER_OR_EQUAL,
	LESS_OR_EQUAL
};

typedef std::pair<cv::Mat, cv::Point2i> TilingRuleResult;

class WordDelimitedBySpace : public std::string {};
std::istream& operator>>(std::istream& is, WordDelimitedBySpace& output);