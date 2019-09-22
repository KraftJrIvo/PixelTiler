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
				_addRule(_lines, tileset);
				_lines.clear();
			}
			continue;
		}
		_lines.push_back(line);
	}
	_addRule(_lines, tileset);
	if (_sizeModifier.width == 0 && _sizeModifier.height == 0)
		_sizeModifier = _rules.back().getSizeModifier();
	_tileset = tileset;
}

void TilingRuleset::_addRule(std::list<std::string>& _lines, cv::Mat& tileset)
{
	_rules.push_back(TilingRule(_lines, tileset));

	auto& rule = _rules.back();

	auto& rots = rule.getRotations();

	for (auto& rot : rots)
	{
		_rules.push_back(rule);
		_rules.back().rotate(rot.first);
		_rules.back().setReaction(rot.second);
	}
}

void TilingRuleset::apply(cv::Mat input, std::list<TilingRuleResult>& results)
{
	if (!_rules.size())
		return;

	auto replaceWindow = _rules.back().getRectToReplace();
	auto replaceWindowSize = cv::Size2i(replaceWindow.width, replaceWindow.height);
	const int yTo = input.rows - replaceWindowSize.height;
	const int xTo = input.cols - replaceWindowSize.width;
	for (int i = 0; i <= yTo; i += replaceWindowSize.height)
		for (int j = 0; j <= xTo; j += replaceWindowSize.width)
		{
			for (auto& rule : _rules)
			{
				replaceWindow = rule.getRectToReplace();
				auto checkWindowSize = rule.getRectToCheck();
				auto roi = _prepareROI(input, cv::Rect(
					j - replaceWindow.x,
					i - replaceWindow.y,
					checkWindowSize.width,
					checkWindowSize.height
				));
				if (rule.applies(roi))
				{
					results.push_back(std::make_pair(rule.apply(_tileset, roi), cv::Point2i(j, i)));
					break;
				}
			}

		}
}

cv::Size2f TilingRuleset::getSizeModifier()
{
	return _sizeModifier;
}

cv::Mat TilingRuleset::_prepareROI(cv::Mat input, cv::Rect rect)
{
	auto left = std::min(std::max(rect.x, 0), input.cols - 1);
	auto right = std::min(std::max(rect.x + rect.width, 0), input.cols);
	auto top = std::min(std::max(rect.y, 0), input.rows - 1);
	auto bottom = std::min(std::max(rect.y + rect.height, 0), input.rows);
	cv::Mat actualRect = input(cv::Rect(left, top, right-left, bottom-top));
	cv::Mat result(rect.height, rect.width, CV_8UC4, cv::Scalar(0,0,0,0));
	actualRect.copyTo(result(cv::Rect(left - rect.x, top - rect.y, actualRect.cols, actualRect.rows)));
	return result;
}
