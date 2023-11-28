/*
	Class for communicating with the Arduino to push the calculator's buttons
*/

#pragma once

#include "Buttons.hpp"

#include <termios.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <cassert>
#include <string>
#include <stdexcept>
#include <iostream>

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
		memset(&config, 0, sizeof(termios));
		config.c_cflag = CREAD; // Enable receiver

		cfmakeraw(&config);

		if(cfsetspeed(&config, B9600) < 0)
			throw std::runtime_error(std::string("Set speed error: ") + strerror(errno));

		if(tcsetattr(portfd, TCSANOW, &config) < 0)
			throw std::runtime_error(std::string("Set config error: ") + strerror(errno));

		// Wait for ready
		if(read_char() != 'R')
			throw std::runtime_error("Arduino not ready");
	}

	// Disconnect on destruction
	~Calculator() {
		if(portfd >= 0)
			close(portfd);
	}

	// Wait for a character on the serial port
	char read_char() {
		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(portfd, &fdset);

		// Wait (since read command doesn't seem to block under MINGW64)
		int ret = select(portfd + 1, &fdset, nullptr, nullptr, nullptr);
		if(ret < 0) return '?';

		// Receive
		char rx;
		if(read(portfd, &rx, 1) != 1)
			return '?';

		return rx;

	}

	// Press a button and wait for release
	// Return false on failure
	bool press_button(BUTTON_ID button, bool print = false, std::ostream &out = std::cout) {
		if(portfd < 0) return false;

		if(print)
			out << "Pressed " << button_names[button] << std::endl;

		std::string command = std::to_string(button) + "\n";
		write(portfd, command.c_str(), command.length());
		tcdrain(portfd);

		if(read_char() != 'K') return false;

		return true;
	}

	void test() {
		assert(press_button(B_ON, true));
		assert(press_button(B_1, true));
		assert(press_button(B_2, true));
		assert(press_button(B_3, true));
	}

private:
	int portfd;
};
