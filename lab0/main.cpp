#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <clocale>   // setlocale

#include "Reader.h"
#include "Freq.h"
#include "Writer.h"

bool compare(const std::pair<std::wstring, int>&a, const std::pair<std::wstring, int>&b) {
    if (a.second != b.second) return a.second > b.second;
    return a.first < b.first;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "ERROR: wrong count of arguments\n";
        return 1;
    }

    std::string input = argv[1];
    std::string output = argv[2];

    // Глобальная C-локаль, для iswalnum/towlower кириллицы. Второй аргумент пустой, берет настройки системы, а не конкретную локаль
    std::setlocale(LC_ALL, "");

    Reader reader;
    std::wifstream inp;
    if (!reader.open(input, inp)) {
        return 1;
    }

    FrequencyCalculator calcFreq;
    int countWords = 0;
    std::map<std::wstring, int> freqResult = calcFreq.calculate_stream(inp, countWords);

    std::vector<std::pair<std::wstring, int>> dict;
    dict.reserve(freqResult.size());
    for (auto& it : freqResult) {
        dict.emplace_back(it.first, it.second);
    }

    std::sort(dict.begin(), dict.end(), compare);

    Writer writer;
    if (writer.write(dict, countWords, output)) {
        return 1;
    }

    return 0;
}
