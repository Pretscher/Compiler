#pragma once
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Utility {
public:
    //removes unnecessary whitespace and all comments from line
    static string makeUniform(string line) {
        bool lastWasWhitespace = true;//set to true because if we begin with whitespace, we want to delete that
        for (int i = 0; i < line.size(); i++) {
            string sub = line.substr(i, 1);
            bool isWhiteSpace = true;
            for (char c : sub) {
                if (!std::isspace(c)) {
                    isWhiteSpace = false;;
                }
            }

            if (isWhiteSpace) {
                if (lastWasWhitespace == true) {
                    line.erase(i, 1);
                    i--;
                    continue;
                }
                lastWasWhitespace = true;
            }
            else {
                lastWasWhitespace = false;
            }
        }
        //erase comments (everything after a //)
        size_t eraseFromHere = line.find("//");
        if (eraseFromHere != std::string::npos) {
            line.erase(eraseFromHere);
        }

        size_t pos = line.find("\t");
        while (pos != std::string::npos) {
            line.replace(pos, 1, "");  // Replace tab with an empty string
            pos = line.find("\t", pos);  // Find the next tab
        }
        return line;
    }


    static bool isOperator(const string& token) {
        return (token == "+" || token == "-" || token == "*" || token == "/" || token == "&amp;" ||
            token == "|" || token == "&lt;" || token == "&gt;" || token == "=");
    }

    static bool isKeywordConstant(const string& token) {
        return token == "true" || token == "false" || token == "null" || token == "this";
    }

    static bool isUnaryOperator(const string& token) {
        return token == "~" || token == "-";
    }


    static vector<string> appendVectors(vector<string>& first, const vector<string>& second) {
        first.insert(first.begin(), second.begin(), second.end());
    }
};