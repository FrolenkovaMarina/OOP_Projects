#include "Freq.h"


std::map<std::string, int> Freq::calculate(std::list<std::string>& text) {
	std::string word;
	std::map<std::string, int> freq;
	for (std::string& line : text) {
		for (unsigned char ch : line) {
			if (std::isalnum(ch)) {
				word.push_back(std::tolower(ch));
			}
			else if (!word.empty()) {
				freq[word] += 1;
				word.clear();
			}
		}
		if (!word.empty()) {
			freq[word] += 1;
			word.clear();
		}
	}
	return freq;
}

