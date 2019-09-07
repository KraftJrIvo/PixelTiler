#include "../PixelTiler/PixelTiler8Dirs.h"

#define ARG_COUNT 3

void printNeededArguments()
{
	std::cout << "Usage: PixelTiler8Dirs-test.exe <.png image file to process (w/o extension)> <desired tileset .png paths separated by space in desired order (w/o extension)>";
}

int main(int argc, char *argv[])
{
	std::string imagePath;
	std::vector<std::string> tilesets;

	// Handling input parameters.
	if (argc < ARG_COUNT)
	{
		imagePath = "test-image.png";
		tilesets.push_back("test-tileset");

		printNeededArguments();
		std::cout << "Too few parameters. Using default settings.";
	}
	else
	{
		imagePath = argv[1];
		for (int i = 2; i < argc; ++i)
			tilesets.push_back(argv[i]);
	}

	PixelTiler8Dirs pt;

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
	//pt.setColorLayerOrderManually();

	// Doing the tiling.
	const size_t& sz = tilesets.size();
	for (int i = 0; i < sz; ++i)
	{
		pt.loadTileset(tilesets[i] + ".png");
		img = pt.tilePixels(img);
	}

	// Letting user correct the final tiling.
	//img = pt.correctTiledLayers();

	// Forming the output file name and saving.
	std::string outName = imagePath.substr(0, imagePath.find('.'));
	for (int i = 0; i < sz; ++i)
		outName += "_" + tilesets[i];
	outName += ".png";
	cv::imwrite(outName, img);

	return 0;
}