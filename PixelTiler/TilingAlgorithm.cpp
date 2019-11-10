#include "TilingAlgorithm.h"

#include <fstream>

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
			if (lines.size())
			{
				_rulesets.push_back(std::make_pair(TilingRuleset(lines, tileset), nullptr));
				lines.clear();
			}
			continue;
		}
		while (line.substr(0, 3) == "rul")
		{
			std::string filename = line.substr(5, line.length() - 3) + ".txt";
			_rulesets.push_back(std::make_pair(TilingRuleset(), new TilingAlgorithm(filename, tileset)));
			while (line != "---" && !in.eof())
				std::getline(in, line);
			if (!in.eof())
				std::getline(in, line);
			while (line.empty() && !in.eof())
				std::getline(in, line);
			lines.clear();
		}
		lines.push_back(line);
	}
	_totalSizeModifier = { 1.0f, 1.0f };
}

cv::Mat TilingAlgorithm::apply(cv::Mat input)
{
	cv::Size2f curSizeMod = {1, 1};

	for (auto& rulesetOrAlgo : _rulesets)
	{
		if (rulesetOrAlgo.second == nullptr)
		{
			auto& ruleset = rulesetOrAlgo.first;
			ruleset.apply(input, _results);
			curSizeMod = ruleset.getSizeModifier();
			_totalSizeModifier = {
				_totalSizeModifier.width * curSizeMod.width,
				_totalSizeModifier.height * curSizeMod.height
			};

			cv::Mat resultImg(input.rows * curSizeMod.width, input.cols * curSizeMod.height, CV_8UC4, cv::Scalar(0, 0, 0, 0));

			for (auto& result : _results)
				result.first.copyTo(resultImg(
					cv::Rect(
						result.second.x * curSizeMod.width,
						result.second.y * curSizeMod.height,
						result.first.cols, result.first.rows)
				));
			_results.clear();

			input = resultImg;
		}
		else
		{
			TilingAlgorithm& algo = *((TilingAlgorithm*)rulesetOrAlgo.second);
			input = algo.apply(input);
			curSizeMod = algo._totalSizeModifier;
			_totalSizeModifier = {
				_totalSizeModifier.width * curSizeMod.width,
				_totalSizeModifier.height * curSizeMod.height
			};
		}
	}

	return input;
}