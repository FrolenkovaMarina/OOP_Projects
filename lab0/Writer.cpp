#include "Writer.h"
#include <iostream>
#include <fstream>


void Writer::writing(std::vector<std::pair<std::string, int>>& dict, int& countWords, bool& flag, const std::string& output) {
	std::ofstream out(output);
	if (!out) {//смогли ли открыть файл
		flag = true;
		std::cout << "ERROR: can't open output file\n";
		return;
	}

	out << "Word;Freq;Percent\n";

	for (auto& i : dict) {
		auto& word = i.first;
		auto& countFreq = i.second;
		double p = (countWords == 0) ? 0.0 : 100.0 * static_cast<double>(countFreq) / static_cast<double>(countWords);

		out << word << ";" << countFreq << ";'" << p << "'\n";//одинарные ' т.к. Excel % на дату
	}
}
