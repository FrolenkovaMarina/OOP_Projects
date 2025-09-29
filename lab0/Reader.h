#pragma once
#include <string>
#include <fstream>

class Reader {
public:
    bool open(const std::string& input, std::wifstream& in);
};

