#pragma once

#include "types.h"
#include "TilingCondition.h"
#include "TilingGroupCondition.h"
#include "TilingRuleReaction.h"

class TilingRule
{
public:
	
	TilingRule() = default;
	TilingRule(const std::list<std::string>& lines, cv::Mat tileset);

	bool applies(cv::Mat);
	cv::Mat apply(cv::Mat, cv::Mat roi);
	cv::Size2f getSizeModifier() const;
	cv::Rect getRectToReplace() const;
	cv::Rect getRectToCheck() const;
	void setReaction(TilingRuleReaction);
	const std::list<std::pair<TilingRuleRotation, TilingRuleReaction>>& getRotations() const;
	void rotate(TilingRuleRotation);

private:

	std::list<TilingCondition> _conditions;
	std::map<int, TilingGroupCondition> _groupConditions;
	std::list<std::pair<TilingRuleRotation, TilingRuleReaction>> _rotations;
	TilingRuleReaction _reaction;
	cv::Rect _rectToCheck;
	cv::Rect _rectToIter;
	cv::Rect _rectToReplace;
	cv::Size2f _sizeModifier;
};
