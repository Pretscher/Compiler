#include "Tokenizer.hpp"
#include <fstream> // for file-access
#include <filesystem>
#include <algorithm>
namespace fs = std::filesystem;
#include "Utility.hpp"

//gives out the line that can be directly written to the file (internally the unchanged lines are stored so that we can determine types)
string Tokenizer::getLine() {
    string line = files[currentFileIndex][currentTokenIndex];
    if (line[0] == '"') {
        return line.substr(1, line.size() - 2);
    }
    if (line[0] == '<') {
        return "&lt;";
    }
    if (line[0] == '>') {
        return "&gt;";
    }
    if (line[0] == '&') {
        return "&amp;";
    }
    return line;
}


string Tokenizer::getTokenType(const string& token) const {
    if (isKeyword(token)) return "keyword";
    if (isOperator(token)) return "symbol";
    if (isInteger(token)) return "integerConstant";
    if (isString(token)) return "stringConstant";
    if (isInteger(token.substr(0, 1))) {
        std::cout << "Syntax error: Cant start an identifier with a number";
    }
    return "identifier";
}


string Tokenizer::getXMLToken() {
    string out;
    out.append("<" + getTokenType(files[currentFileIndex][currentTokenIndex]) + ">");
    out.append(getLine());
    out.append("</" + getTokenType(files[currentFileIndex][currentTokenIndex]) + ">");
    return out;
}

vector<string> Tokenizer::getRemainingTokens() {
    vector<string> destination;
    std::copy(files[currentFileIndex].begin() + currentTokenIndex, files[currentFileIndex].end(), std::back_inserter(destination));
    return destination;
}


vector<string> Tokenizer::readFile(const string& file) {
    vector<string> tokens;
    fileNames.push_back(file);
    fileNames[0].erase(fileNames[0].end() - 5, fileNames[0].end());

    std::ifstream infile(file); //open the file
    if (infile.is_open() && infile.good()) {
        string line = "";
        while (std::getline(infile, line)) {
            line = Utility::makeUniform(line);//remove unneccesary whitespace and comments

            if (line != "") {//then the line would have been whitespace or a comment
                appendTokensIfNotComment(tokens, lineToTokens(line));
            }
        }
    }
    return tokens;
}

vector<vector<string>> Tokenizer::readInput(const string& path) {
    if (path.length() >= 5) {
        std::string lastFiveChars = path.substr(path.length() - 5);
        if (lastFiveChars == ".jack") {
            return { readFile(path) };
        }
    }
    return readFolder(path);
}


vector<vector<string>> Tokenizer::readFolder(const string& path) {
    vector<vector<string>> tokensForFile;
    int fileCount = 0;
    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.path().extension() == ".jack") {
            string fileName = entry.path().string();
            fileName.erase(fileName.end() - 5, fileName.end());
            fileNames.push_back(fileName);

            tokensForFile.push_back(vector<string>());
            tokensForFile[fileCount] = vector<string>();
            std::ifstream file(entry.path());
            std::string line;
            while (std::getline(file, line)) {
                line = Utility::makeUniform(line);//remove unneccesary whitespace and comments
                if (line != "") {//then the line would have been whitespace or a comment
                    appendTokensIfNotComment(tokensForFile[fileCount], lineToTokens(line));
                }
            }

            file.close();
            fileCount++;
        }
    }
    return tokensForFile;
}

void Tokenizer::appendTokensIfNotComment(vector<string>& toAppend, vector<string> tokens) {
    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i] == "/" && i + 1 < tokens.size() && tokens[i + 1] == "*") {
            currentlyComment = true;
        }

        if (tokens[i] == "*" && i + 1 < tokens.size() && tokens[i + 1] == "/") {
            currentlyComment = false;
            i = i + 2;
            continue;
        }

        if (currentlyComment == false) {
            toAppend.push_back(tokens[i]);
        }
    }
}

vector<string> Tokenizer::lineToTokens(const string& input) const {
    vector<string> tokens;
    string token;
    bool currentlyInString = false;
    string str;
    for (char c : input) {
        if (c == '\"') {
            str.push_back(c);
            if (currentlyInString == true) {
                currentlyInString = false;
                tokens.push_back(str);//push string without splitting
                str = "";
            }
            else {
                if (token != "") {
                    vector<string> splitToken = splitTokenAtSymbols(token);//push back all the tokens that were created until the string
                    tokens.insert(tokens.end(), splitToken.begin(), splitToken.end());
                    token = "";
                    currentlyInString = true;
                }
            }
        }
        else if (currentlyInString == false) {//dont split up strings at blanks
            if (c == ' ') {
                if (token != "") {
                    vector<string> splitToken = splitTokenAtSymbols(token);
                    tokens.insert(tokens.end(), splitToken.begin(), splitToken.end());
                    token = "";
                }
                continue;
            }
            token.push_back(c);
        }
        else {
            str.push_back(c);
        }
    }
    vector<string> splitToken = splitTokenAtSymbols(token);
    tokens.insert(tokens.end(), splitToken.begin(), splitToken.end());
    return tokens;
}

vector<string> Tokenizer::splitTokenAtSymbols(const string& token) const {
    vector<string> out;
    string currentString;
    for (const char& c : token) {
        if (isOperator(c)) {
            //cut, such that the previous token is a single token and the symbol is a token
            if (currentString != "") {
                out.push_back(currentString);
            }
            currentString = "";

            string charString;
            charString.push_back(c);
            out.push_back(charString);

        }
        else {
            currentString += c;
        }
    }
    if (currentString != "") {
        out.push_back(currentString);
    }
    return out;
}