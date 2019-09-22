#pragma once

#include <opencv2/opencv.hpp>

enum TilingConditionMetric
{
	ANY,
	NO,
	YES
};

enum TilingRuleRotation
{
	CONDROT_90,
	CONDROT_180,
	CONDROT_270
};

enum TilingReactionType
{
	NO_TYPE,
	IMG,
	PIX
};

enum TilingConditionOperator
{
	GREATER,
	LESS,
	EQUALS,
	GREATER_OR_EQUAL,
	LESS_OR_EQUAL
};

struct TilingCondition
{
	cv::Point2i relPos;
	TilingConditionMetric metric;

	TilingCondition() {};
	TilingCondition(cv::Point2i p, TilingConditionMetric m) :
		relPos(p),
		metric(m)
	{}
	bool applies(cv::Mat);
	void rotate(cv::Size2i, TilingRuleRotation);
};

struct TilingGroupCondition
{
	int thresh;
	std::list<cv::Point2i> relPos;
	TilingConditionOperator op;

	TilingGroupCondition() {};
	TilingGroupCondition(int th, std::list<cv::Point2i> tls, TilingConditionOperator tco) :
		thresh(th),
		relPos(tls),
		op(tco)
	{}

	size_t countApplies(cv::Mat);
	void rotate(cv::Size2i, TilingRuleRotation);
};

struct ReactionImage
{
	ReactionImage() = default;
	ReactionImage(cv::Size _tilesCount, cv::Size _tileCoords) :
		tilesCount(_tilesCount),
		tileCoords(_tileCoords)
	{}
	cv::Size tilesCount;
	cv::Size tileCoords;
};

struct ReactionPixels
{
	ReactionPixels() = default;
	ReactionPixels(cv::Mat px) :
		pixels(px)
	{}
	cv::Mat pixels;
};

typedef std::pair<cv::Mat, cv::Point2i> TilingRuleResult;

class TilingRuleReaction
{
	friend class TilingRule;
public:
	cv::Size2i size;

	TilingRuleReaction() = default;
	TilingRuleReaction(std::list<std::string> line, cv::Mat tileset);
	cv::Mat getResult(cv::Mat tileset);

private:
	TilingReactionType _type;
	ReactionImage _img;
	ReactionPixels _pix;
};

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
	cv::Rect _rectToReplace;
	cv::Size2f _sizeModifier;
};
