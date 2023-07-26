#include "Parser.hpp"
#include "CodeGenerator.hpp"
#include <fstream> // for file-access
#include <filesystem>
#include <algorithm>
namespace fs = std::filesystem;

void writeFile(vector<string> lines) {

	int i = 0;
	while (i < lines.size()) {
		string path = lines[i++];
		path.erase(path.begin(), path.begin() + 11);//erase the START_FILE_ part
		path += ".vm";
		std::ofstream outputFile(path);
		if (outputFile.is_open()) {
			while (i < lines.size()) {
				string line = lines[i];
				if (line.find("START_FILE_") != string::npos) {
					outputFile.close();
					break;
				}
				outputFile << line << "\n";
				i++;
			}
			outputFile.close();
		}
		else {
			std::cout << "Unable to create the file." << std::endl;
		}
	}
}


int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cout << "Incorrect amount of arguments. Exiting...";
		return -1;
	}
	Tokenizer* tokenizer = new Tokenizer(argv[1]);
	Parser parser(tokenizer);


	vector<string> parsedLines;
	while (tokenizer->hasMoreFiles()) {
		vector<string> fileTranslation;
		parsedLines.push_back("START_FILE_" + tokenizer->getCurrentFileName());
		fileTranslation = parser.parseFile();
		parsedLines.insert(parsedLines.end(), fileTranslation.begin(), fileTranslation.end());
		tokenizer->advanceFile();
	}
	CodeGenerator codeGenerator(parsedLines);
	vector<string> out = codeGenerator.compile();
	string str = argv[1];
	writeFile(out);
	return 0;
}