/*
 Generate random button presses using the Generator class
 and save images of the calculator for later processing
*/

#include "Calculator.hpp"
#include "Generator.hpp"
#include "GeneratorStream.hpp"

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <atomic>
#include <set>
#include <memory>
#include <cstring>

#include <opencv2/opencv.hpp>

//#define CALIBRATE_MODE

using namespace std;
using namespace cv;

const chrono::milliseconds cam_delay(100);
const chrono::milliseconds inter_op_delay(100);

int main(int argc, char **argv) {

	const char *serial_port = nullptr;
	const char *video_path = nullptr;
	int webcam_ind = 0;
	bool help = false;

	std::unique_ptr<Generator> gen(new Generator());
	std::unique_ptr<istream> input_file;

	for(int argn = 1; argn < argc; argn++) {
		const char *arg = argv[argn];

		if(strcmp(arg, "-h") == 0) {
			// Suppress warning printout
			serial_port++;
			video_path++;
			help = true;
		}

		else if(strstr(arg, "-w") == arg) {
			if(strlen(arg) > 2)
				webcam_ind = atoi(arg + 2);
			else if(++argn < argc)
				webcam_ind = atoi(argv[argn]);
			else {
				cerr << "Missing webcam index" << endl << endl;
				help = true;
			}
		}

		else if(strcmp(arg, "-g") == 0) {
			if(++argn < argc) {
				arg = argv[argn];

				if(strcmp(arg, "default") == 0)
					gen.reset(new Generator());

				else if(strcmp(arg, "file") == 0) {
					if(++argn < argc) {
						arg = argv[argn];

						if(strcmp(arg, "-") == 0)
							gen.reset(new GeneratorStream(cin));

						else {
							input_file.reset(new ifstream(arg));
							if(!input_file->good()) {
								cerr << "Could not open file " << arg << endl;
								return 1;
							}

							gen.reset(new GeneratorStream(*input_file));
						}
					}

					else {
						cerr << "Missing generator file" << endl << endl;
						help = true;
					}
				}

				else {
					cerr << "Unknown generator type" << endl << endl;
					help = true;
				}
			}

			else {
				cerr << "Missing generator type" << endl << endl;
				help = true;
			}
		}

		else if(!video_path)
			video_path = arg;

		else if(!serial_port)
			serial_port = arg;

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

#ifndef CALIBRATE_MODE
	if(!serial_port) {
		cerr << "Serial port required" << endl << endl;
		help = true;
	}
#endif

	// Print help
	if(help) {
		cerr << "===== calcfuzz =====" << endl
		<< "Usage: " << argv[0] << " [-h] [-w webcam index] [-g generator] [MP4 video path] [serial port]" << endl
		<< "\t-h             : print help" << endl
		<< "\t-w [number]    : select webcam" << endl
		<< "\t-g [generator] : specify a generator type" << endl
		<< "\t                     default : press buttions 0-9" << endl
		<< "\t                     file    : read button presses from a file; following argument must be a file; specify \"-\" to use stdin" << endl
		<< endl;
		return 1;
	}

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

	while(!gen->done()) {
		const auto button_info = gen->generate_print();
		if(!calc->press_button(button_info.first))
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
