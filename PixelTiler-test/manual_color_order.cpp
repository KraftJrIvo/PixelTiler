#include "manual_color_order.h"

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

		if (x > rect.x&& x < rect.x + rect.width && y > rect.y&& y < rect.y + rect.height)
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