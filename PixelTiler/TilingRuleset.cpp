#include "TilingRuleset.h"

TilingRuleset::TilingRuleset(const std::list<std::string>& lines, cv::Mat tileset)
{
	_sizeModifier = { 0, 0 };
	std::list<std::string> _lines;
	for (auto& line : lines)
	{
		if (line.empty())
		{
			if (lines.size())
			{
				_rules.push_back(TilingRule(_lines, tileset));
				_lines.clear();
				if (_sizeModifier.width == 0 && _sizeModifier.height == 0)
					_sizeModifier = _rules.back().getSizeModifier();
				else if (_sizeModifier != _rules.back().getSizeModifier())
				{
					// bad
				}
			}
			continue;
		}
		_lines.push_back(line);
	}
	_rules.push_back(TilingRule(_lines, tileset));
}

void TilingRuleset::apply(cv::Mat input, std::list<TilingRuleResult>& results)
{
	if (!_rules.size())
		return;

	auto windowSize = _rules.back().getRectToReplace();
	const int yTo = input.rows - windowSize.height;
	const int xTo = input.cols - windowSize.width;
	for (int i = 0; i <= yTo; i += windowSize.height)
		for (int j = 0; j <= xTo; j += windowSize.width)
			for (auto& rule : _rules)
			{
				auto roi = input(cv::Rect(j, i, windowSize.width, windowSize.height));
				if (rule.applies(roi))
				{
					results.push_back(std::make_pair(rule.apply(roi), cv::Point2i(j, i)));
					break;
				}
			}
}

cv::Size2f TilingRuleset::getSizeModifier()
{
	return _sizeModifier;
}
