#include "Parser.hpp"

#include <fstream> // for file-access
#include <filesystem>
#include <algorithm>
namespace fs = std::filesystem;

void writeFile(vector<string> lines, string path) {
	path.append("_mine.xml");
    std::ofstream outputFile(path);

    if (outputFile.is_open()) {
		//outputFile << "<tokens>\n";
        for (string line : lines) {
            outputFile << line << "\n";
        }
		//outputFile << "</tokens>\n";
        outputFile.close();
    }
    else {
        std::cout << "Unable to create the file." << std::endl;
    }
}

vector<string> getXmlTokens(Tokenizer tokenizer) {
	vector<string> tokens;
	tokens.push_back("<tokens>");
	while (tokenizer.hasMoreLines()) {
		tokens.push_back(tokenizer.getXMLToken());
		tokenizer.advanceToken();
	}
	tokens.push_back(tokenizer.getXMLToken());//last token
	tokenizer.resetLine();

	tokens.push_back("</tokens>");
	return tokens;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cout << "Incorrect amount of arguments. Exiting...";
		return -1;
	}
	Tokenizer tokenizer(argv[1]);
	Parser parser(&tokenizer);
	while (tokenizer.hasMoreFiles()) {
		vector<string> fileTranslation;

		writeFile(getXmlTokens(tokenizer), tokenizer.getCurrentFileName() + "T");
		fileTranslation = parser.parseFile();
		
		writeFile(fileTranslation, tokenizer.getCurrentFileName());
		tokenizer.advanceFile();
	}
	std::cout << "XML files created successfully!";
	return 0;
}