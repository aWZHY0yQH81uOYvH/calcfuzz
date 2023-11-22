/*
 A button press generator that reads from a stream in the same format generate_print writes
*/

#pragma once

#include "Generator.hpp"
#include "Buttons.hpp"

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <cctype>

class GeneratorStream: public Generator {
public:
	GeneratorStream(std::istream &stream = std::cin): stream(stream) {
		// Generate button map
		for(int id = 0; id < B_MAX; id++)
			button_map.emplace(button_names[id], (BUTTON_ID)id);
	}

	virtual bool done() const {
		return !stream.good() || stream.peek() == EOF;
	}

	virtual std::pair<BUTTON_ID, bool> generate() {
		std::stringstream ss;
		char ch;

		// Read until there is whitespace
		while(stream.get(ch)) {
			if(std::isspace(ch)) {
				// Ignore whitespace until we get a character
				if(ss.str().size() == 0)
					continue;

				// Get button ID
				auto it = button_map.find(ss.str());
				
				// Handle unknown
				if(it == button_map.end()) {
					std::cerr << "Unknown button name " << ss.str() << std::endl;
					ss.str("");
					continue;
				}

				// Capture if space character is a newline
				bool capture = (ch == '\n');

				return {it->second, capture};
			}

			// Save non-whitespace input
			else ss << ch;
		}

		// Will only get here if the last item in the stream is unknown
		return {B_ON, false};
	}

protected:
	// Stream to read button presses from
	std::istream &stream;

	// Map from button names to button IDs
	std::unordered_map<std::string, BUTTON_ID> button_map;
};
