#pragma once
#include <map>
#include <string>
#include <list>

class Freq {
public:
	std::map<std::string, int> calculate(std::list<std::string>& text);
};