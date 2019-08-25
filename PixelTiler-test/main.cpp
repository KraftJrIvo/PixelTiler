#include "../PixelTiler/PixelTiler.h"

#define ARG_COUNT 4

void printNeededArguments()
{
	std::cout << "Usage: PixelTiler-test.exe <image file to process> <tiles folder path w/o last slash> <desired tiles prefixes separated by space in desired order>";
}

int main(int argc, char *argv[])
{
	std::string imagePath, tilesPath;
	std::vector<std::string> tilesPrefixes;

	// Handling input parameters.
	if (argc < ARG_COUNT)
	{
		imagePath = "test-image.png";
		tilesPath = "test-tiles";
		tilesPrefixes.push_back("default");

		printNeededArguments();
		std::cout << "Too few parameters. Using default settings.";
	}
	else
	{
		imagePath = argv[1];
		tilesPath = argv[2];
		for (int i = 3; i < argc; ++i)
			tilesPrefixes.push_back(argv[i]);
	}

	PixelTiler pt;

	// Making sure the input image has right amount of channels.
	cv::Mat img = cv::imread(imagePath, CV_LOAD_IMAGE_UNCHANGED);
	switch (img.channels())
	{
	case 1:
		cv::cvtColor(img, img, CV_GRAY2BGRA); 
		break;
	case 3:
		cv::cvtColor(img, img, CV_BGR2BGRA); 
		break;
	default:
		break;
	}

	// Letting user to manually set the color layer order.
	pt.setColorLayerOrderManually();

	// Doing the tiling.
	const size_t& sz = tilesPrefixes.size();
	for (int i = 0; i < sz; ++i)
	{
		pt.loadTiles(tilesPrefixes[i]);
		img = pt.tilePixels(img);
	}

	// Letting user correct the final tiling.
	img = pt.correctTiledLayers();

	// Forming the output file name and saving.
	std::string outName = imagePath.substr(0, imagePath.find('.'));
	for (int i = 0; i < sz; ++i)
		outName += tilesPrefixes[i];
	outName += ".png";
	cv::imwrite(outName, img);

	return 0;
}