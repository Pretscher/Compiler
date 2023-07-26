#pragma once
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Tokenizer {
public:
	Tokenizer(string directory) {
        files = readInput(directory);
	}

    void advanceToken() {
        if (currentTokenIndex + 1 < files[currentFileIndex].size()) {
            currentTokenIndex++;
        }
    }

    void resetLine() {
        currentTokenIndex = 0;
    }

    void advanceFile() {
        currentFileIndex ++;
        currentTokenIndex = 0;
    }

    //gives out the line that can be directly written to the file (internally the unchanged lines are stored so that we can determine types)
    string getLine();

    bool hasMoreLines() {
        return currentTokenIndex + 1 < files[currentFileIndex].size();
    }

    bool hasMoreFiles() {
        return currentFileIndex < files.size();
    }

    string getCurrentFileName() {
        return fileNames[currentFileIndex];
    }

    string getTokenType(const string& token) const;
    string getXMLToken();
    vector<string> getRemainingTokens();

private:
    vector<vector<string>> files;
    vector<string> fileNames;
    size_t currentTokenIndex;
    size_t currentFileIndex;

    vector<string> readFile(const string& file);
    vector<vector<string>> readInput(const string& path);
    vector<vector<string>> readFolder(const string& path);
    //removes unnecessary whitespace and all comments from line
    bool currentlyComment = false;
    void appendTokensIfNotComment(vector<string>& toAppend, vector<string> tokens);
    vector<string> lineToTokens(const string& input) const;
    vector<string> splitTokenAtSymbols(const string& token) const;

    bool isOperator(const string& s) const {
        if (s.size() > 1) {
            return false;
        }
        if (s[0] == ';' || s[0] == '(' || s[0] == ')' || s[0] == '{' || s[0] == '}' || s[0] == '[' || s[0] == ']' ||
            s[0] == '.' || s[0] == ',' || s[0] == '+' || s[0] == '-' || s[0] == '*' || s[0] == '/' || s[0] == '&' ||
            s[0] == '<' || s[0] == '>' || s[0] == '=' || s[0] == '~' || s[0] == '|') return true;
        return false;
    }

    bool isOperator(const char& c) const {
        if (c == ';' || c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' ||
            c == '.' || c == ',' || c == '+' || c == '-' || c == '*' || c == '/' || c == '&' ||
            c == '<' || c == '>' || c == '=' || c == '~' || c == '|') return true;
        return false;
    }

    bool isInteger(const string& str) const {
        try {
            int a = stoi(str);
            return true;
        }
        catch (const exception&) {
            return false;
        }
    }

    bool isString(const string& str) const {
        if (str[0] == '\"') {
            return true;
        }
        return false;
    }

    bool isKeyword(const string& str) const {
        return str == "class" || str == "constructor" || str == "function" || str == "method" || str == "field" || str == "static" || str == "var" ||
            str == "int" || str == "char" || str == "boolean" || str == "void" || str == "true" || str == "false" || str == "null" || str == "this" ||
            str == "let" || str == "do" || str == "if" || str == "else" || str == "while" || str == "return";
    }


};