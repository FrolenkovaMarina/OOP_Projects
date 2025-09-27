#pragma once
#include <list>
#include <string>


class Reader {
public:
	std::list<std::string> reading(const std::string& input, bool& flag);
};