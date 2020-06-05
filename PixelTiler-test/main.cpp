#include <PixelTiler.h>

#include "manual_color_order.h"

#define MIN_ARG_COUNT 3

void printNeededArguments()
{
	std::cout << "rules - .txt file path (w/o extension)\n";
	std::cout << "tileset, image - .png file path (w/o extension)\n";
	std::cout << "Usage: image rules:tileset rules:tileset ...";
}

void makeTransparentStuffBlack(cv::Mat img)
{
	for (int i = 0; i < img.rows; ++i)
		for (int j = 0; j < img.rows; ++j)
			if (img.at<cv::Vec4b>(i, j)[3] == 0)
			{
				img.at<cv::Vec4b>(i, j)[0] = img.at<cv::Vec4b>(i, j)[1] = img.at<cv::Vec4b>(i, j)[2] = 0;
			}
}

int main(int argc, char* argv[])
{
	std::string imagePath;
	std::vector<std::string> rules;
	std::vector<std::string> tilesets;

	// Handling input parameters.
	if (argc < MIN_ARG_COUNT)
	{
		rules.push_back("test");
		tilesets.push_back("template");
		imagePath = "img";

		printNeededArguments();
		std::cout << "Too few parameters. Using default settings.";
	}
	else
	{
		imagePath = std::string(argv[1]);
		for (int i = 2; i < argc; ++i)
		{
			size_t colonLoc = std::string(argv[i]).find(':');
			if (colonLoc == std::string::npos)
			{
				rules.push_back(std::string(argv[i]));
				tilesets.push_back("");
			}
			else
			{
				rules.push_back(std::string(argv[i]).substr(0, colonLoc));
				tilesets.push_back(std::string(argv[i]).substr(
					colonLoc + 1,
					std::string(argv[i]).length() - colonLoc - 1
				));
			}
		}
	}

	PixelTiler pt;
	cv::Size2i windowSize = { 1280, 720 };

	// Making sure the input image has right amount of channels.
	cv::Mat img = cv::imread(imagePath + ".png", cv::IMREAD_UNCHANGED);
	switch (img.channels())
	{
	case 1:
		cv::cvtColor(img, img, cv::COLOR_GRAY2BGRA);
		break;
	case 3:
		cv::cvtColor(img, img, cv::COLOR_BGR2BGRA);
		break;
	default:
		break;
	}
	makeTransparentStuffBlack(img);

	// Letting user to manually set the color layer order.
	auto colors = setColorLayerOrderManually(windowSize, img);

	// Doing the tiling.
	const size_t& sz = rules.size();
	for (int i = 0; i < sz; ++i)
	{
		pt.loadTilesetAndAlgorithm(rules[i] + ".txt", tilesets[i] + ".png");
		if (i == 0 && colors.size()) pt.setColorLayerOrder(colors);
		img = pt.tilePixels(img);
	}

	// Letting user correct the final tiling.
	img = pt.correctTiledLayers(windowSize);;

	// Forming the output file name and saving.
	std::string outName = imagePath.substr(0, imagePath.find('.'));
	for (int i = 0; i < sz; ++i)
		outName += "_" + rules[i];
	outName += ".png";
	cv::imwrite(outName, img);

	return 0;
}