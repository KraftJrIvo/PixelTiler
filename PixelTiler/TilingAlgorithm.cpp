#include "TilingAlgorithm.h"

TilingAlgorithm::TilingAlgorithm(std::string filepath, cv::Mat tileset)
{
	std::ifstream in(filepath);
	std::list<std::string> lines;
	std::string line;
	while (!in.eof())
	{
		std::getline(in, line);
		if (line == "---" || in.eof())
		{
			_rulesets.push_back(std::make_pair(TilingRuleset(lines, tileset), nullptr));
			lines.clear();
			continue;
		}
		if (line.substr(0, 3) == "rul")
		{
			std::string filename = line.substr(3, line.length() - 4) + ".txt";
			_rulesets.push_back(std::make_pair(TilingRuleset(), new TilingAlgorithm(filename, tileset)));
		}
		lines.push_back(line);
	}
	_sizeModifier = { 1.0f, 1.0f };
}

cv::Mat TilingAlgorithm::apply(cv::Mat input)
{
	for (auto& rulesetOrAlgo : _rulesets)
	{
		if (rulesetOrAlgo.second == nullptr)
		{
			auto& ruleset = rulesetOrAlgo.first;
			ruleset.apply(input, _results);
			auto sizeMod = ruleset.getSizeModifier();
			_sizeModifier = {
				_sizeModifier.width * sizeMod.width,
				_sizeModifier.height * sizeMod.height
			};
		}
		else
		{
			auto& algo = *((TilingAlgorithm*)rulesetOrAlgo.second);
			algo.apply(input);
			for (auto& result : algo._results)
				_results.push_back(result);
			_sizeModifier = {
				_sizeModifier.width * algo._sizeModifier.width,
				_sizeModifier.height * algo._sizeModifier.height
			};
		}
	}

	cv::Mat resultImg(input.rows * _sizeModifier.width,
		input.cols * _sizeModifier.height, CV_8UC4);
	for (auto& result : _results)
		result.first.copyTo(resultImg(
			cv::Rect(result.second.x * result.first.cols, result.second.y * result.first.rows, result.first.cols, result.first.rows)
		));

	return resultImg;
}