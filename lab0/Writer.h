#pragma once
#include <string>
#include <vector>
#include <utility>

class Writer {
public:
    bool write(std::vector<std::pair<std::wstring, int>>& dict,
        int& countWords, const std::string& output);
};

