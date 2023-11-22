/*
 Generates random button presses
*/

#pragma once

#include "buttons.hpp"

#include <utility>

class Generator {
public:
	bool done() const {
		return count >= 10;
	}

	// Return a button to press and if we should capture the calculator's state
	std::pair<BUTTON_ID, bool> generate() {
		return {(BUTTON_ID)(count++), true};
	}

private:
	int count = 0;
};
