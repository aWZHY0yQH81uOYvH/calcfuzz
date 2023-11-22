/*
 Generate random button presses using the Generator class
 and save images of the calculator for later processing
*/

#include "calccomm.hpp"
#include "generator.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <opencv2/opencv.hpp>

//#define CALIBRATE_MODE

using namespace std;
using namespace cv;

const chrono::milliseconds cam_delay(100);

int main(int argc, char **argv) {

	if(argc < 4)
		// TODO: add usage printout
		return 1;

	// Parse arguments
	const char *serial_port = argv[1];
	const char *video_path = argv[2];
	const int webcam_ind = atoi(argv[3]);

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
	// Might not need this with the CAP_V4L2 mode above, but the idea was to keep the camera frame buffer empty for minimal delay
	atomic<bool> run(true);
	bool new_frame = false;
	mutex frame_mutex;
	condition_variable frame_cond;
	thread reader_thread([&]() {
		while(run) {
			frame_mutex.lock();
			webcam >> frame;
			new_frame = true;
			frame_mutex.unlock();
			frame_cond.notify_one();
			this_thread::sleep_for(chrono::milliseconds(1)); // time for the other thread to lock the mutex I guess
		}
	});

#ifdef CALIBRATE_MODE

	for(int x = 0; x < 10; x++) {
		// Thread for printing out time to measure delay
		thread printer([&]() {
			for(int y = 0; y < 100; y++) {
				cout << "Frame index " << (x+1) << " time " << y*10 << endl;
				this_thread::sleep_for(chrono::milliseconds(10));
			}
		});

		// Wait for new frame
		unique_lock<mutex> lock(frame_mutex);
		frame_cond.wait(lock, [&]() {return new_frame;});
		new_frame = false;

		// Save frame
		videowriter << frame;

		frame_mutex.unlock();

		printer.join();
	}

#else

	Calculator calc(serial_port);
	Generator gen;

	while(!gen.done()) {
		const auto button_info = gen.generate();
		if(!calc.press_button(button_info.first, true))
			break;

		// Take a frame if requested
		if(button_info.second) {
			// Wait for camera delay
			// TODO: pipeline
			this_thread::sleep_for(cam_delay);

			// Wait for new frame
			unique_lock<mutex> lock(frame_mutex);
			frame_cond.wait(lock, [&]() {return new_frame;});
			new_frame = false;

			// Save frame
			videowriter << frame;

			frame_mutex.unlock();
		}
	}

#endif

	run = false;
	reader_thread.join();

}
