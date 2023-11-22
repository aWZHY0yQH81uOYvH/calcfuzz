/*
 Post-process captured images
*/

#include "Digit.hpp"

#include <iostream>
#include <vector>
#include <cstring>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

Mat frame;

// Store the center coordinates of all segments
vector<Point2i> segpts;

// Show selected points on top of frame
void show_select() {
	Mat show = frame.clone();

	for(Point2i p:segpts)
		circle(show, p, 2, Scalar(0, 255, 0), FILLED);

	imshow("Select Points", show);
}

// Add selections to the segpts array on click
void mouse_callback(int event, int x, int y, int flags, void *userdata) {
	(void)flags;
	(void)userdata;

	if(event == EVENT_LBUTTONDOWN) {
		segpts.emplace_back(x, y);
		show_select();
	}
}

int main(int argc, const char **argv) {
	const char *video_path = nullptr;
	bool help = false;

	for(int argn = 1; argn < argc; argn++) {
		const char *arg = argv[argn];

		if(strcmp(arg, "-h") == 0) {
			// Suppress warning printout
			video_path++;
			help = true;
		}

		else if(!video_path)
			video_path = arg;

		else {
			cerr << "Unknown argument \"" << arg << "\"" << endl;
			help = true;
		}
	}

	// Require inputs
	if(!video_path) {
		cerr << "Video path required" << endl << endl;
		help = true;
	}

	// Print help
	if(help) {
		cerr << "===== digitprocess =====" << endl
		<< "Usage: " << argv[0] << " [-h] [MP4 video path]" << endl
		<< "\t-h          : print help" << endl
		<< endl;
		return 1;
	}

	// Open video file
	VideoCapture vid(video_path);
	if(!vid.isOpened()) {
		cerr << "Could not open video file " << video_path << endl;
		return 1;
	}

	// Ask user to select segment points
	vid.read(frame);

	namedWindow("Select Points", WINDOW_NORMAL);
	setMouseCallback("Select Points", mouse_callback, nullptr);

	cerr << "Select points in standard 7-segment order (decimal last) starting from the rightmost digit." << endl
	     << "Select error segment last." << endl;

	// UI key processing loop
	while(true) {
		bool run = true;
		while(run) {
			show_select();

			char key = waitKey() & 0xFF;

			switch(key) {
				case 'd': // d to delete point
					if(segpts.size() > 0)
						segpts.pop_back();
					break;

				case 'e': // e to stop
					run = false;
					break;

				default:
					break;
			}
		}

		if(segpts.size() >= 9 && segpts.size() % 8 == 1)
			break;
		else
			cerr << "Invalid number of points" << endl;
	}

	// Create digit processor objects
	// First one will handle error segment
	vector<Digit> digits;
	while(segpts.size())
		digits.emplace_back(segpts, digits.size() == 0);

	// Frame processing loop
	while(!frame.empty()) {
		// Try to get value of each digit
		for(Digit &digit:digits)
			cout << digit.value(frame);

		cout << endl;

		vid.read(frame);
	}
}
