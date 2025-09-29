#include "Reader.h"
#include <iostream>
#include <locale>
#include <codecvt>

bool Reader::open(const std::string& input, std::wifstream& in) {

    in.open(input, std::ios::binary);
    if (!in) {
        std::cout << "ERROR: can't open input file\n";
        return false;
    }

    in.imbue(std::locale(".UTF-8"));

    auto ch = in.peek();
    if (ch == WEOF) {
        std::cout << "ERROR: input file empty\n";
        return false;
    }
    return true;
}
