#include "../PixelTiler/PixelTiler.h"

#define MIN_ARG_COUNT 3

void printNeededArguments()
{
	std::cout << "rules - .txt file path (w/o extension)\n";
	std::cout << "tileset, image - .png file path (w/o extension)\n";
	std::cout << "Usage: image rules:tileset rules:tileset ...";
}

std::vector<cv::Scalar> setColorLayerOrderManually(cv::Size2i windowSize, cv::Mat input)
{
	float horRatio = float(input.cols) / float(windowSize.width);
	float verRatio = float(input.rows) / float(windowSize.height);

	int width = (verRatio > horRatio) ?
		(input.cols * windowSize.height / input.rows) : windowSize.width;
	int height = (verRatio <= horRatio) ?
		(input.rows * windowSize.width / input.cols) : windowSize.height;
	int xPos = (windowSize.width - width) / 2;
	int yPos = (windowSize.height - height) / 2;
	cv::Rect resultRect(xPos, yPos, width, height);
	cv::Mat curImg(windowSize.height + 100, windowSize.width, CV_8UC4, cv::Scalar(0, 0, 0, 255));

	cv::Size2f cursorSize = { float(width) / float(input.cols), float(height) / float(input.rows) };

	std::vector<cv::Scalar> colors;

	typedef std::tuple<cv::Rect, cv::Size2f, cv::Point2i, int> MouseData;

	auto onMouse = [](int event, int x, int y, int, void* _data)
	{
		MouseData& data = *((MouseData*)_data);
		const cv::Rect& rect = std::get<0>(data);
		const cv::Size2f& cursorSize = std::get<1>(data);

		float layerX = (x - rect.x) / cursorSize.width;
		float layerY = (y - rect.y) / cursorSize.height;

		if (x > rect.x && x < rect.x + rect.width && y > rect.y && y < rect.y + rect.height)
		{
			std::get<2>(data) = cv::Point2i(layerX, layerY);
			if (event == cv::EVENT_LBUTTONDOWN)
				std::get<3>(data) = 1;
			else if (event == cv::EVENT_RBUTTONDOWN)
				std::get<3>(data) = -1;
			else
				std::get<3>(data) = 0;
		}
	};

	int status = 0;
	MouseData data(resultRect, cursorSize, cv::Point2f(0, 0), status);

	while (true)
	{

		int curStatus = std::get<3>(data);
		if (curStatus)
		{
			auto& pt = std::get<2>(data);
			if (pt.x >= 0 && pt.y >= 0 && pt.x < input.cols && pt.y < input.rows)
			{
				auto& pixel = input.at<cv::Vec4b>(pt.y, pt.x);
				bool found = false;
				if (curStatus > 0 && pixel[3] > 0)
				{
					const size_t& sz = colors.size();
					for (int i = 0; i < sz; ++i)
						if (colors[i][0] == pixel[0] && colors[i][1] == pixel[1] && colors[i][2] == pixel[2] && colors[i][3] == pixel[3])
						{
							found = true;
							break;
						}
					if (!found)
						colors.push_back(pixel);
				}
				else if (curStatus < 0 && colors.size())
					colors.pop_back();
			}
		}

		curImg.setTo(cv::Scalar(0, 0, 0, 0));
		cv::resize(input, curImg(resultRect), resultRect.size(), 0, 0, cv::InterpolationFlags::INTER_NEAREST);
		cv::Point2i cursorPos = {
			int(resultRect.x + std::get<2>(data).x * cursorSize.width),
			int(resultRect.y + std::get<2>(data).y * cursorSize.height)
		};
		cv::rectangle(curImg, cv::Rect(cursorPos.x, cursorPos.y, cursorSize.width, cursorSize.height), cv::Scalar(0, 0, 255, 255));
		const size_t& sz = colors.size();
		for (int i = 0; i < sz; ++i)
			cv::rectangle(curImg, cv::Rect(i * 100, windowSize.height, 100, 100), colors[i], -1);
		cv::imshow("layer order", curImg);
		cv::setMouseCallback("layer order", onMouse, &data);
		int key = cv::waitKey(10);
		if (key == 13)
			break;
	}

	return colors;
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
		tilesets.push_back("");
		imagePath = "test";

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