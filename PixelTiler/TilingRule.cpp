#include "TilingRule.h"

TilingRule::TilingRule(const std::list<std::string>& lines, cv::Mat tileset)
{
	int h = 0;
	for (auto& line : lines)
	{
		if (line[0] == 'x' || line[0] == 'X' || line[0] == '?' || line[0] == '.' ||
			line[0] == '-' || line[0] == '+' || (line[0] >= '0' && line[0] <= '9'))
			h++;
		else
			break;
	}
	std::stringstream str;
	str << lines.front();
	std::vector<std::string> firstVals((std::istream_iterator<WordDelimitedBySpace>(str)),
		std::istream_iterator<WordDelimitedBySpace>());
	int w = firstVals.size();

	str = std::stringstream();
	int counter = 0;
	for (auto& line : lines)
	{
		if (counter < h)
		{
			str << line << " ";
			counter++;
		}
		else
			break;
	}

	std::vector<std::string> allVals((std::istream_iterator<WordDelimitedBySpace>(str)),
		std::istream_iterator<WordDelimitedBySpace>());

	std::map<int, std::list<cv::Point2i>> groups;

	int minX = INT_MAX, maxX = INT_MIN, minY = INT_MAX, maxY = INT_MIN;
	int i = 0, j = 0;
	bool xFound = false;
	for (auto& val : allVals)
	{
		if (val == "?" || val == ".")
			_conditions.push_back(TilingCondition({j, i}, ANY));
		else if (val == "+" || val == "X")
			_conditions.push_back(TilingCondition({j, i}, YES));
		else if (val == "-" || val == "x")
			_conditions.push_back(TilingCondition({j, i}, NO));
		
		if (val == "X" || val == "x" || val == ".")
		{
			xFound = true;
			if (j < minX) minX = j;
			if (j > maxX) maxX = j;
			if (i < minY) minY = i;
			if (i > maxY) maxY = i;
		}
		else if (val[0] >= '0' && val[0] <= '9')
		{
			int gid = std::stoi(val);
			groups[gid].push_back({j, i});
		}

		j++;
		if (j == w)
		{
			i++;
			j = 0;
		}
		if (i == h)
			break;
	}

	std::list<std::string> groupStrs;
	std::list<std::string> reactStrs;
	counter = 0;
	const size_t& gsz = groups.size();
	for (auto& line : lines)
	{
		if (counter >= h)
			if (counter < h + gsz)
				groupStrs.push_back(line);
			else
				reactStrs.push_back(line);
		counter++;
	}
	for (auto& groupCond : groupStrs)
	{
		str = std::stringstream();
		str << groupCond;
		std::vector<std::string> gCond((std::istream_iterator<WordDelimitedBySpace>(str)),
			std::istream_iterator<WordDelimitedBySpace>());
		int gid = std::stoi(gCond[1]);
		TilingConditionOperator op;
		if (gCond[2] == "=")
			op = EQUALS;
		else if (gCond[2] == ">")
			op = GREATER;
		else if (gCond[2] == "<")
			op = LESS;
		else if (gCond[2] == ">=")
			op = GREATER_OR_EQUAL;
		else if (gCond[2] == "<=")
			op = LESS_OR_EQUAL;
		_groupConditions[gid] = TilingGroupCondition(std::stoi(gCond[3]), groups[gid], op);
	}

	_reaction = TilingRuleReaction(reactStrs, tileset);

	for (auto& line : reactStrs)
	{
		if (line[0] == '$')
		{
			std::list<std::string> subline = { "img" + line.substr(line.find(' ')) };
			auto react = TilingRuleReaction(subline, tileset);
			if (line[1] == '9')
				_rotations.push_back(std::make_pair(CONDROT_90, react));
			else if (line[1] == '1')
				_rotations.push_back(std::make_pair(CONDROT_180, react));
			else
				_rotations.push_back(std::make_pair(CONDROT_270, react));
		}
	}

	_rectToCheck = cv::Rect(0, 0, w, h);
	_rectToReplace = xFound ? cv::Rect(minX, minY, maxX - minX + 1, maxY - minY + 1) : _rectToCheck;

	_sizeModifier = { 
		float(_reaction.size.width) / float(_rectToReplace.width),
		float(_reaction.size.height) / float(_rectToReplace.height)
	};
}

bool TilingRule::applies(cv::Mat input)
{
	for (auto& cond : _conditions)
	{
		if (!cond.applies(input))
			return false;
	}
	for (auto& cond : _groupConditions)
	{
		if (cond.second.countApplies(input) == -1)
			return false;
	}
	return true;
}

cv::Mat TilingRule::apply(cv::Mat tileset, cv::Mat roi)
{
	cv::Mat result = _reaction.getResult(tileset);

	if (_reaction._type == PIX)
	{
		int w = result.cols;
		int h = result.rows;
		for (int i = 0; i < h; ++i)
			for (int j = 0; j < w; ++j)
			{
				auto pix = result.at<cv::Vec4b>(i, j);
				if (pix[3] > 0 && pix[3] < 255 && pix[0] < 255)
				{
					if (pix[3] == 127)
					{
						int r = (rand() % 2) ? 255 : 0;
						result.at<cv::Vec4b>(i, j) = cv::Scalar(r, r, r, 255);
					}
					else
					{
						bool val = (_groupConditions[pix[0]].countApplies(roi) > 0);
						result.at<cv::Vec4b>(i, j) = val ? cv::Scalar(255, 255, 255, 255) : cv::Scalar(0, 0, 0, 0);
					}
				}
			}
	}

	return result;
}

cv::Size2f TilingRule::getSizeModifier() const
{
	return _sizeModifier;
}

cv::Rect TilingRule::getRectToReplace() const
{
	return _rectToReplace;
}

cv::Rect TilingRule::getRectToCheck() const
{
	return _rectToCheck;
}

void TilingRule::setReaction(TilingRuleReaction reaction)
{
	_reaction = reaction;
}

const std::list<std::pair<TilingRuleRotation, TilingRuleReaction>>& TilingRule::getRotations() const
{
	return _rotations;
}

void TilingRule::rotate(TilingRuleRotation rot)
{
	for (auto& cond : _conditions)
		cond.rotate(_rectToCheck.size(), rot);

	for (auto& gcond : _groupConditions)
		gcond.second.rotate(_rectToCheck.size(), rot);
	
	int x = (rot == CONDROT_270) ? _rectToReplace.y : (rot == CONDROT_180) ? 
		(_rectToCheck.width - _rectToReplace.width - _rectToReplace.x) : 
		(_rectToCheck.height - _rectToReplace.height - _rectToReplace.y);
	int y = (rot == CONDROT_90) ? _rectToReplace.x : (rot == CONDROT_180) ? 
		(_rectToCheck.height - _rectToReplace.height - _rectToReplace.y) : 
		(_rectToCheck.width - _rectToReplace.width - _rectToReplace.x);

	if (rot == CONDROT_90)
		_rectToReplace = cv::Rect(x, y, _rectToReplace.height, _rectToReplace.width);
	else if (rot == CONDROT_180)
		_rectToReplace = cv::Rect(x, y,	_rectToReplace.width, _rectToReplace.height);
	else 
		_rectToReplace = cv::Rect(x, y,	_rectToReplace.height, _rectToReplace.width);

	if (rot == CONDROT_90 || rot == CONDROT_270)
		_rectToCheck = cv::Rect(0, 0, _rectToCheck.height, _rectToCheck.width);
}