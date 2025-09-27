#pragma once
#include <string>

class ArgCheck {

private:
	std::string input;
	std::string output;
	bool correct = false;

public:
    ArgCheck(int argc, char** argv);
    const std::string& getInput() const;
    const std::string& getOutput() const;
    bool getCorrect() const;
};
