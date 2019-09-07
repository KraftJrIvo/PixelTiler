#pragma once

#include "TilingRuleset.h"

class TilingAlgorithm
{
public:
	TilingAlgorithm() = default;
	TilingAlgorithm(std::string filepath, cv::Mat tileset);

	cv::Mat apply(cv::Mat input);

private:
	std::list<std::pair<TilingRuleset, void*>> _rulesets;
	std::list<TilingRuleResult> _results;
	cv::Size2f _sizeModifier;
};