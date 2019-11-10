#pragma once

#include <opencv2/opencv.hpp>

#include "types.h"

struct TilingGroupCondition
{
	int thresh;
	std::list<cv::Point2i> relPos;
	TilingConditionOperator op;

	TilingGroupCondition() {};
	TilingGroupCondition(int th, std::list<cv::Point2i> tls, TilingConditionOperator tco) :
		thresh(th),
		relPos(tls),
		op(tco)
	{}

	size_t countApplies(cv::Mat);
	void rotate(cv::Size2i, TilingRuleRotation);
};
