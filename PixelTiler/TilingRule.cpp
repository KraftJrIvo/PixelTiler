#include "TilingRule.h"

class WordDelimitedBySpace : public std::string {};
std::istream& operator>>(std::istream& is, WordDelimitedBySpace& output)
{
	std::getline(is, output, ' ');
	return is;
}

TilingRuleReaction::TilingRuleReaction(std::list<std::string> lines, cv::Mat tileset)
{
	std::string type = lines.front().substr(0, 3);
	std::string rest = lines.front().substr(3, lines.back().length() - 3);
	std::stringstream str;
	lines.pop_front();
	str << rest;
	if (type == "img")
	{
		_type = IMG;
		int w, h, x, y;
		str >> w >> h >> x >> y;
		_img = ReactionImage(cv::Size(w, h), cv::Size(x, y));
		int tileW = tileset.cols / _img.tilesCount.width;
		int tileH = tileset.rows / _img.tilesCount.height;
		size = { tileW, tileH };
	}
	else if (type == "pix")
	{
		_type = PIX;
		
		str << lines.front();

		std::vector<std::string> vals((std::istream_iterator<WordDelimitedBySpace>(str)),
			std::istream_iterator<WordDelimitedBySpace>());

		int w = vals.size();
		int h = lines.size();

		cv::Mat pixels(h, w, CV_8UC4, cv::Scalar(0,0,0,0));

		str = std::stringstream();
		for (auto& line : lines)
			str << line << " ";

		std::vector<std::string> allVals((std::istream_iterator<WordDelimitedBySpace>(str)),
			std::istream_iterator<WordDelimitedBySpace>());

		for (int i = 0; i < h; ++i)
			for (int j = 0; j < w; ++j)
			{
				auto& pix = pixels.at<cv::Vec4b>(i, j);
				if (allVals[i * w + j] == "+")
					pix = cv::Scalar(255, 255, 255, 255);
			}

		_pix = ReactionPixels(pixels);
		size = {w, h};
	}
}

cv::Mat TilingRuleReaction::getResult(cv::Mat tileset)
{
	if (_type == IMG)
	{
		int tileW = tileset.cols / _img.tilesCount.width;
		int tileH = tileset.rows / _img.tilesCount.height;
		int tileX = tileW * _img.tileCoords.width;
		int tileY = tileH * _img.tileCoords.height;
		return tileset(cv::Rect(tileX, tileY, tileW, tileH));
	}
	return _pix.pixels;
}

TilingRule::TilingRule(const std::list<std::string>& lines, cv::Mat tileset)
{
	int h = 0;
	for (auto& line : lines)
	{
		if (line[0] == 'x' || line[0] == 'X' || line[0] == '?' ||
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
		if (val == "?")
			_conditions.push_back(TilingCondition({j, i}, ANY));
		else if (val == "+" || val == "X")
			_conditions.push_back(TilingCondition({j, i}, YES));
		else if (val == "-" || val == "x")
			_conditions.push_back(TilingCondition({j, i}, NO));
		
		if (val == "X" || val == "x")
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
		if (!cond.second.applies(input))
			return false;
	}
	return true;
}

cv::Mat TilingRule::apply(cv::Mat tileset)
{
	return _reaction.getResult(tileset);
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
	
	int x = (rot == CONDROT_270) ? _rectToReplace.y : (rot == CONDROT_180) ? (_rectToCheck.width - 1 - _rectToReplace.x) : (_rectToCheck.height - 1 - _rectToReplace.y);
	int y = (rot == CONDROT_90) ? _rectToReplace.x : (rot == CONDROT_180) ? (_rectToCheck.height - 1 - _rectToReplace.y) : (_rectToCheck.width - 1 - _rectToReplace.x);

	if (rot == CONDROT_90)
		_rectToReplace = cv::Rect(x, y, _rectToReplace.height, _rectToReplace.width);
	else if (rot == CONDROT_180)
		_rectToReplace = cv::Rect(x, y,	_rectToReplace.width, _rectToReplace.height);
	else 
		_rectToReplace = cv::Rect(x, y,	_rectToReplace.height, _rectToReplace.width);

	if (rot == CONDROT_90 || rot == CONDROT_270)
		_rectToCheck = cv::Rect(0, 0, _rectToCheck.height, _rectToCheck.width);
}

bool TilingCondition::applies(cv::Mat input)
{
	const auto& pix = input.at<cv::Vec4b>(relPos.y, relPos.x);
	return (metric != NO || pix[0] == 0) && (metric != YES || pix[0] > 0);
}

void TilingCondition::rotate(cv::Size2i window, TilingRuleRotation rot)
{
	int x = (rot == CONDROT_270) ? relPos.y : (rot == CONDROT_180) ? (window.width - 1 - relPos.x) : (window.height - 1 - relPos.y);
	int y = (rot == CONDROT_90) ? relPos.x : (rot == CONDROT_180) ? (window.height - 1 - relPos.y): (window.width - 1 - relPos.x);
	relPos = { x, y };
}

bool TilingGroupCondition::applies(cv::Mat input)
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
		return count > thresh;
	case LESS:
		return count < thresh;
	case EQUALS:
		return count == thresh;
	case GREATER_OR_EQUAL:
		return count >= thresh;
	case LESS_OR_EQUAL:
		return count <= thresh;
	default:
		return false;
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
