/*
 Generate random button presses using the Generator class
 and save images of the calculator for later processing
*/

#include "calccomm.hpp"
#include "generator.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <set>
#include <memory>

#include <opencv2/opencv.hpp>

//#define CALIBRATE_MODE

using namespace std;
using namespace cv;

const chrono::milliseconds cam_delay(100);
const chrono::milliseconds inter_op_delay(100);

int main(int argc, char **argv) {

	if(argc < 4)
		// TODO: add usage printout
		return 1;

	// Parse arguments
	const char *serial_port = argv[1];
	const char *video_path = argv[2];
	const int webcam_ind = atoi(argv[3]);

#ifndef CALIBRATE_MODE
	unique_ptr<Calculator> calc;
	try {
		calc.reset(new Calculator(serial_port));
	} catch(std::runtime_error &e) {
		cerr << e.what() << endl;
		return 1;
	}
#endif

	Mat frame;

#ifdef __linux__
	const int api_pref = CAP_V4L2;
#else
	const int api_pref = CAP_ANY;
#endif

	VideoCapture webcam(webcam_ind, api_pref);
	if(!webcam.isOpened()) {
		cerr << "Could not open webcam" << endl;
		return 1;
	}

	// Get frame to figure out video size
	webcam >> frame;

	VideoWriter videowriter(video_path, VideoWriter::fourcc('m', 'p', '4', 'v'), 1, frame.size());
	if(!videowriter.isOpened()) {
		cerr << "Could not open video file" << endl;
		return 1;
	}

	// Start another thread to continuously read from the webcam
	// and save a frame at the requested time
	atomic<bool> run(true);
	mutex frame_mutex;
	set<chrono::steady_clock::time_point> frame_times;

	thread reader_thread([&]() {
		while(run) {
			// Get a frame
			webcam >> frame;

			// Check if it's time to save a frame
			frame_mutex.lock();
			auto now = chrono::steady_clock::now();
			auto it = frame_times.begin();
			while(it != frame_times.end() && *it <= now) {
				videowriter << frame;
				it = frame_times.erase(it);
			}

			frame_mutex.unlock();

			this_thread::sleep_for(chrono::milliseconds(1));
		}
	});

#ifdef CALIBRATE_MODE

	for(int x = 0; x < 10; x++) {
		// Thread for printing out time to measure delay
		thread printer([&]() {
			for(int y = 0; y < 100; y++) {
				cout << "Frame index " << (x+1) << " time " << (y*10) << endl;
				this_thread::sleep_for(chrono::milliseconds(10));
			}
		});

		// Schedule a new frame to be taken now
		frame_mutex.lock();
		frame_times.emplace(chrono::steady_clock::now());
		frame_mutex.unlock();

		printer.join();
	}

#else

	Generator gen;

	while(!gen.done()) {
		const auto button_info = gen.generate();
		if(!calc->press_button(button_info.first, true))
			break;

		// Take a frame if requested
		if(button_info.second) {
			// Schedule a new frame to be taken cam_delay in the future
			frame_mutex.lock();
			frame_times.emplace(chrono::steady_clock::now() + cam_delay);
			frame_mutex.unlock();

			// Wait for the inter-operation delay (to account for inaccuracies in cam_delay)
			this_thread::sleep_for(inter_op_delay);
		}
	}

#endif

	run = false;
	reader_thread.join();

}
