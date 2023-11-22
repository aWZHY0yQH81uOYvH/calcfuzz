/*
	Class for communicating with the Arduino to push the calculator's buttons
*/

#pragma once

#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <cassert>
#include <string>
#include <stdexcept>

class Calculator {
public:
	// Connect to serial port on object construction
	Calculator(const char *serial_port) {
		portfd = open(serial_port, O_RDWR | O_NOCTTY);
		if(portfd < 0)
			throw std::runtime_error(std::string("Port open error: ") + strerror(errno));

		if(!isatty(portfd)) {
			close(portfd);
			portfd = -1;
			throw std::runtime_error("Port is not a TTY");
		}

		struct termios config;
		config.c_iflag = 0;
		config.c_oflag = 0;
		config.c_cflag = CREAD; // Enable receiver
		config.c_lflag = 0;

		cfmakeraw(&config);

		if(cfsetspeed(&config, B9600) < 0)
			throw std::runtime_error(std::string("Set speed error: ") + strerror(errno));

		if(tcsetattr(portfd, TCSANOW, &config) < 0)
			throw std::runtime_error(std::string("Set config error: ") + strerror(errno));

		// Wait for ready
		char rx;
		size_t rx_count = read(portfd, &rx, 1);
		if(rx_count != 1 || rx != 'R')
			throw std::runtime_error("Arduino not ready");
	}

	// Disconnect on destruction
	~Calculator() {
		if(portfd >= 0)
			close(portfd);
	}

	// Button IDs the Arduino sketch understands
	enum BUTTON_ID {
		B_0,
		B_1,
		B_2,
		B_3,
		B_4,
		B_5,
		B_6,
		B_7,
		B_8,
		B_9,
		B_DOT,
		B_EQ,
		B_SUB,
		B_ADD,
		B_MUL,
		B_DIV,
		B_PERCENT,
		B_SQRT,
		B_MRC,
		B_MSUB,
		B_MADD,
		B_OFF,
		B_ON,
		B_MAX
	};

	// Press a button and wait for release
	// Return false on failure
	bool press_button(BUTTON_ID button) {
		if(portfd < 0) return false;

		std::string command = std::to_string(button) + "\n";
		write(portfd, command.c_str(), command.length());

		char rx;
		size_t rx_count = read(portfd, &rx, 1);
		if(rx_count != 1) return false;
		if(rx != 'K') return false;

		return true;
	}

	void test() {
		assert(press_button(B_ON));
		assert(press_button(B_1));
		assert(press_button(B_2));
		assert(press_button(B_3));
	}

private:
	int portfd;
};
