#include "TilingGroupCondition.h"

#include "TilingCondition.h"

size_t TilingGroupCondition::countApplies(cv::Mat input)
{
	std::list<TilingCondition> conds;

	for (auto& rp : relPos)
		conds.push_back(TilingCondition(rp, YES));

	size_t count = 0;
	for (auto& cond : conds)
		count += cond.applies(input);

	switch (op)
	{
	case GREATER:
		return (count > thresh) ? count : -1;
	case LESS:
		return (count < thresh) ? count : -1;
	case EQUALS:
		return (count == thresh) ? count : -1;
	case GREATER_OR_EQUAL:
		return (count >= thresh) ? count : -1;
	case LESS_OR_EQUAL:
		return (count <= thresh) ? count : -1;
	default:
		return -1;
	}
}

void TilingGroupCondition::rotate(cv::Size2i window, TilingRuleRotation rot)
{
	for (auto& pos : relPos)
	{
		int x = (rot == CONDROT_270) ? pos.y : (rot == CONDROT_180) ? (window.width - 1 - pos.x) : (window.height - 1 - pos.y);
		int y = (rot == CONDROT_90) ? pos.x : (rot == CONDROT_180) ? (window.height - 1 - pos.y) : (window.width - 1 - pos.x);
		pos = { x, y };
	}
}