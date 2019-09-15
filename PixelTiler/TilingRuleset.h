#pragma once

#include "TilingRule.h"

class TilingRuleset
{
public:
	TilingRuleset() = default;
	TilingRuleset(const std::list<std::string>& lines, cv::Mat tileset);

	void _addRule(std::list<std::string>& _lines, cv::Mat& tileset);

	void apply(cv::Mat, std::list<TilingRuleResult>&);
	cv::Size2f getSizeModifier();

private:
	cv::Mat _tileset;
	std::list<TilingRule> _rules;
	cv::Size2f _sizeModifier;
	
	cv::Mat _prepareROI(cv::Mat, cv::Rect);
};