#include "TilingCondition.h"

TilingCondition::TilingCondition(cv::Point2i p, TilingConditionMetric m) :
	relPos(p),
	metric(m)
{
}

bool TilingCondition::applies(cv::Mat input)
{
	const auto& pix = input.at<cv::Vec4b>(relPos.y, relPos.x);
	return (metric != NO || pix[0] == 0) && (metric != YES || pix[0] > 0);
}

void TilingCondition::rotate(cv::Size2i window, TilingRuleRotation rot)
{
	int x = (rot == CONDROT_270) ? relPos.y : (rot == CONDROT_180) ? (window.width - 1 - relPos.x) : (window.height - 1 - relPos.y);
	int y = (rot == CONDROT_90) ? relPos.x : (rot == CONDROT_180) ? (window.height - 1 - relPos.y) : (window.width - 1 - relPos.x);
	relPos = { x, y };
}