#include "ArgCheck.h"


ArgCheck::ArgCheck(int argc, char** argv) {
	if (argc == 3) {
		input = argv[1];
		output = argv[2];
		correct = true;
	}
}

const std::string& ArgCheck::getInput() const { return input; }
const std::string& ArgCheck::getOutput() const { return output; }
bool ArgCheck::getCorrect() const { return correct; }
