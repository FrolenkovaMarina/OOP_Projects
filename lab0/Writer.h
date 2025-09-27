#pragma once
#include <string>
#include <vector>

class Writer {
public:
	void writing(std::vector<std::pair<std::string, int>>& dict, 
		int& countWords, bool& flag, const std::string& output);
};
