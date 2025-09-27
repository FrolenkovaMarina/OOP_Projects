#include "Reader.h"
#include <iostream>
#include <fstream>


std::list<std::string> Reader::reading(const std::string& input, bool& flag) {
	std::ifstream inp(input);
	std::list<std::string> text;
	if (!inp) {//правильность ввода
		flag = true;
		std::cout << "ERROR: can't open input file\n";
		return text;
	}
	std::string line;
	int count = 0;
	while (std::getline(inp, line)) {
		count++;
		text.push_back(line);
	}
	if (count == 0) {//является ли пустым файл
		flag = true;
		std::cout << "ERROR: input file empty\n";
		return text;
	}
	return text;

}

