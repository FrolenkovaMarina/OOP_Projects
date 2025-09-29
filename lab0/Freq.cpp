#include "Freq.h"
#include <cwctype> // iswalnum towlower

std::map<std::wstring, int> FrequencyCalculator::calculate_stream(std::wistream& in, int& countWords) {
    std::map<std::wstring, int> freq;
    std::wstring word;
    countWords = 0;


    //weof - wide end of file
    for (wint_t ch = in.get(); ch != WEOF; ch = in.get()) {
        if (std::iswalnum(ch)) {
            word.push_back(static_cast<wchar_t>(std::towlower(ch)));
        }
        else if (!word.empty()) {
                ++freq[word];
                ++countWords;
                word.clear();
        }
    }

    if (!word.empty()) {
        ++freq[word];
        ++countWords;
        word.clear();
    }
    return freq;
}
