#pragma once
#include <map>
#include <string>
#include <istream>

class FrequencyCalculator {
public:
    std::map<std::wstring, int> calculate_stream(std::wistream& in, int& countWords);
};

