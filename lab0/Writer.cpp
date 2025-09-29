#include "Writer.h"
#include <iostream>
#include <fstream>
#include <locale>
#include <codecvt>

bool Writer::write(std::vector<std::pair<std::wstring, int>>& dict,
    int& countWords, const std::string& output) {
    std::wofstream out(output, std::ios::binary);
    if (!out) {
        std::cout << "ERROR: can't open output file\n";
        return false;
    }

    out.imbue(std::locale(".UTF-8"));


    // Пишем BOM, чтобы Excel воспринял UTF-8
    out << L"\uFEFF";
    out << L"Word;Freq;Percent\n";

    for (auto& p : dict) {
        double percent = (countWords == 0) ? 0.0
            : 100.0 * static_cast<double>(p.second) / static_cast<double>(countWords);
        out << p.first << L";" << p.second << L";'" << percent << L"'\n";
    }

    return true;
}
