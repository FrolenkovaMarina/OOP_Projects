#include <iostream>
#include <list>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>

bool compare(const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
	if (a.second != b.second) {
		return a.second > b.second;
	}
	return a.first < b.first;
}

class ArgCheck {
private:
	std::string input;
	std::string output;
	bool correct = false;
public:
	ArgCheck(int argc, char** argv) {
		if (argc == 3) {
			input = argv[1];
			output = argv[2];
			correct = true;
		}
	}
	const std::string& getInput() {
		return input;
	}
	const std::string& getOutput() {
		return output;
	}
	const bool getCorrect() {
		return correct;
	}
};

class Reader {
public:
	std::list<std::string> reading(const std::string& input, bool& flag) {
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

};

class Freq {
public:
	std::map<std::string, int> calculate(std::list<std::string>& text) {
		std::string word;
		std::map<std::string, int> freq;
		for (std::string& line : text) {
			for (unsigned char ch : line) {
				if (std::isalnum(ch)){
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
};

class Writer {
public:
	void writing(std::vector<std::pair<std::string, int>>& dict, int& countWords, bool& flag, const std::string& output) {
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
};

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
	for(auto& i: freqResult){
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