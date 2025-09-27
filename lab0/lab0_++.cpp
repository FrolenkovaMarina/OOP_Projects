#include <iostream>
#include <list>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>

#include "ArgCheck.h"
#include "Reader.h"
#include "Freq.h"
#include "Writer.h"

bool compare(const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
	if (a.second != b.second) {
		return a.second > b.second;
	}
	return a.first < b.first;
}


int main(int argc, char** argv) {
	ArgCheck checker(argc, argv);
	if (!checker.getCorrect()) {
		std::cout << "ERROR: wrong count of arguments\n";
		return 1;
	}
	Reader reader;
	Freq calcFreq;
	bool flag = false;
	std::list<std::string> text = reader.reading(checker.getInput(), flag);
	if (flag) {
		return 1;
	}
	std::map<std::string, int> freqResult = calcFreq.calculate(text);
	std::vector<std::pair<std::string, int>> dict;
	dict.reserve(freqResult.size());

	int countWords = 0;
	for (auto& i : freqResult) {
		auto& word = i.first;
		auto& countFreq = i.second;

		dict.emplace_back(word, countFreq);//создаёт pair внутри vector
		countWords += countFreq; //все слова(не разные только)
	}
	std::sort(dict.begin(), dict.end(), compare);

	Writer writer;
	writer.writing(dict, countWords, flag, checker.getOutput());
	if (flag) {
		return 1;
	}
	return 0;
}