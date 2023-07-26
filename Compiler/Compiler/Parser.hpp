#pragma once
#include "Tokenizer.hpp"

#include <iostream>
#include <vector>
using namespace std;


class Parser {
public:
	Parser(Tokenizer* tokenizer) {
		this->tokenizer = tokenizer;
	}

	vector<string> parseFile() {
		out = vector<string>();
		currentToken = tokenizer->getLine();
		parseClass();//program has to begin with a class.
		return out;
	}

private:
	Tokenizer* tokenizer;
	string currentToken;
	vector<string> out;

	void print(string str) {
		out.push_back(str);
	}

	void print(vector<string> strings) {
		for (string str : strings) {
			out.push_back(str);
		}
	}

	void advance();

	void process(const string& str);

	//Statements

	void parseStatement();

	void parseWhile();

	void parseLet();

	void parseIf();

	void parseDo();

	void parseReturn();

	//Expressions

	void parseExpressionList();

	void parseExpression();

	void parseTerm();

	void parseSubroutineCall();

	bool isSubroutineCall();

	//program structur

	void parseClass();

	void parseClassVarDeclaration();

	void parseSubroutineDeclaration();
	
	void parseParameterList();

	void parseSubroutineBody();

	void parseVarDec();

	void parseIdentifier();


	bool isOperator(const string& token) const {
		return (token == "+" || token == "-" || token == "*" || token == "/" || token == "&amp;" ||
			token == "|" || token == "&lt;" || token == "&gt;" || token == "=");
	}

	bool isKeywordConstant(const string& token) const {
		return token == "true" || token == "false" || token == "null" || token == "this";
	}

	bool isUnaryOperator(const string& token) const {
		return token == "~" || token == "-";
	}

	bool isType(const string& token) const {
		return token == "char" || token == "int" || token == "boolean" || tokenizer->getTokenType(token) == "identifier";
	}
};