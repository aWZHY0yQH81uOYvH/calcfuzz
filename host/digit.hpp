/*
 Class to handle seven-segment digit coordinates
*/

#pragma once

#include <stdexcept>
#include <vector>
#include <string>

#include <opencv2/opencv.hpp>

class Digit {
public:
	static constexpr int seg_2_digit[11] = {
		0b0111111,
		0b0000110,
		0b1011011,
		0b1001111,
		0b1100110,
		0b1101101,
		0b1111101,
		0b0000111,
		0b1111111,
		0b1101111,
		0b1000000
	};

	Digit(std::vector<cv::Point2i> &segpts, bool handle_error = false, int avg_size = 5, double thresh = 0.75):
	handle_error(handle_error), avg_size(avg_size), thresh(thresh) {
		int npts = 8;

		if(handle_error)
			npts++;

		if((int)segpts.size() < npts)
			throw std::invalid_argument("Not enough points");

		// Grab the last eight points
		for(int x = npts - 1; x >= 0; x--) {
			pts[x] = *segpts.rbegin();
			segpts.pop_back();
		}
	}

	// Return average brightness around area
	int avg_box(const cv::Mat &image, cv::Point2i center) {
		// Extract region of interest
		const cv::Mat roi{image,
			cv::Rect{
				center - cv::Point2i{avg_size, avg_size},
				center + cv::Point2i{avg_size, avg_size}
			}};

		// Average RGB over area
		cv::Scalar avg = cv::mean(roi);
		return avg[0] + avg[1] + avg[2];
	}

	// Get background brightness of this segment
	// Determine by checking in middle of segment squares
	int bg_color(const cv::Mat &image) {
		const cv::Point2i top_center = (pts[0] + pts[1] + pts[5] + pts[6]) / 4;
		const cv::Point2i bot_center = (pts[2] + pts[3] + pts[4] + pts[6]) / 4;

		return (avg_box(image, top_center)
		      + avg_box(image, bot_center)) / 2;
	}

	// Get digit value
	std::string value(const cv::Mat &image) {
		const int background = bg_color(image);
		const int threshold = background * thresh;

		// Detect state of each segment
		int state = 0;
		for(int x = 0; x < 7; x++)
			state |= (avg_box(image, pts[x]) < threshold) << x;

		// Search for a matching segment configuration
		std::string ret = "?";
		for(int x = 0; x < 10; x++)
			if(state == seg_2_digit[x])
				ret = std::to_string(x);

		// Check for minus sign
		if(state == seg_2_digit[10])
			ret = "-";

		// Check for decimal
		if(avg_box(image, pts[7]) < threshold)
			ret += ".";

		// Check for error
		if(handle_error
		   && avg_box(image, pts[8]) < threshold)
			ret = "E" + ret;

		return ret;
	}

private:
	// Coordinates in image of our segments
	cv::Point2i pts[9];

	// If we are going to handle the error segment
	bool handle_error;

	// How many pixels to average over
	const int avg_size;

	// Must be at least this much darker than background to count as on
	const double thresh;
};
