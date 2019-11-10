#pragma once

#include <opencv2/opencv.hpp>

#include "types.h"

struct TilingCondition
{
	cv::Point2i relPos;
	TilingConditionMetric metric;

	TilingCondition() = default;
	TilingCondition(cv::Point2i p, TilingConditionMetric m);

	bool applies(cv::Mat);
	void rotate(cv::Size2i, TilingRuleRotation);
};
