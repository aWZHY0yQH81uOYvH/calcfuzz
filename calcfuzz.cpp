#include <iostream>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main() {
	Mat frame;

	VideoCapture webcam(0, CAP_V4L2);
	if(!webcam.isOpened()) {
		cerr << "Could not open webcam" << endl;
		return 1;
	}

	// Get frame to figure out video size
	webcam >> frame;

	VideoWriter videowriter("calc.mp4", VideoWriter::fourcc('m', 'p', '4', 'v'), 30, frame.size());
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

	run = false;
	reader_thread.join();

}
