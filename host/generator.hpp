/*
 Generates random button presses
*/

#pragma once

#include "buttons.hpp"

#include <utility>
#include <iostream>
#include <vector>
#include <cassert>

class Generator {
public:
	virtual ~Generator() {}

	virtual bool done() const {
		return count >= 10;
	}

	// Return a button to press and if we should capture the calculator's state
	virtual std::pair<BUTTON_ID, bool> generate() {
		return {(BUTTON_ID)(count++), true};
	}

	// Wrapper for generate that prints all button presses leading to each state capture
	std::pair<BUTTON_ID, bool> generate_print(std::ostream &out = std::cout) {
		auto gen = generate();
		assert(gen.first >= 0 && gen.first < B_MAX);

		out << button_names[gen.first];

		// Output newline when capturing
		if(gen.second)
			out << std::endl;
		else
			out << " ";

		return gen;
	}

private:
	int count = 0;
};
