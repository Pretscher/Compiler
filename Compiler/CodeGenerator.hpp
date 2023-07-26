#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <numeric>
using namespace std;
#include "Utility.hpp"

class CodeGenerator {
public:

	CodeGenerator(const vector<string>& parserOutput) : parserOutput(parserOutput) {
		if (parserOutput.size() > 0) {
			createLookupTables();
		}
	}

	vector<string> compile();

	void createLookupTables();

private:
	map<string, bool> isMethod;
	const vector<string>& parserOutput;
	int currentLineIndex = 0;
	string line;
	vector<string> compiled;

	int whileCounter = 0;
	int ifCounter = 0;

	//defines all the OS methods, such as Memory, Math etc.
	void createOsLookupTables();

	//methods for ease of use and debugging-----------------------------------------------------------------------------------------

	void print(string s) {
		compiled.push_back(s);
	}

	string getLineType() {
		return getClassification(parserOutput[currentLineIndex]);
	}

	string getLineTypeAndAdvance() {
		string out = getClassification(parserOutput[currentLineIndex]);
		advance();
		return out;
	}

	bool hasMoreLines() {
		return currentLineIndex < parserOutput.size() - 1;
	}

	string advance() {
		currentLineIndex++;
		line = getContent(parserOutput[currentLineIndex]);

		//cout << parserOutput[currentLineIndex] << "\n";//debugging
		return line;
	}

	string advance(int steps) {
		currentLineIndex += steps;
		line = getContent(parserOutput[currentLineIndex]);
		return line;
	}

	void reset() {
		currentLineIndex = -1;
		advance();
	}

	//advances if the current line matches the expected line, else terminates with an error. Either the actual line or only the content
	//can be provided.
	void advanceExpecting(string expectedLine) {
		if (expectedLine != line && expectedLine != parserOutput[currentLineIndex]) {
			std::cout << "Expected " << expectedLine << " but was " << line << "\n";
		}
		currentLineIndex++;
		line = getContent(parserOutput[currentLineIndex]);

		//cout << parserOutput[currentLineIndex] << "\n";//debugging
	}

	//advances if the current line matches any of the expected lines, else terminates with an error
	void advanceExpectingAny(vector<string> expectedLines) {

		if (find(expectedLines.begin(), expectedLines.end(), line) == expectedLines.end()) {//not found

			cout << "Expected one of the following " << std::accumulate(expectedLines.begin(), expectedLines.end(), std::string(" "))
				<< " but was " << line;
		}
		currentLineIndex++;
		line = getContent(parserOutput[currentLineIndex]);
	}

	//\methods for ease of use and debugging----------------------------------------------------------------------------------------

	//lookup tables---------------------------------------------------------------------
	struct Variable {
	public:
		Variable(string type, string kind, int index) {
			this->type = type;
			this->kind = kind;
			this->index = index;
		}

		Variable() {}
		string type, kind;
		int index;
	};

	string currentFunction, currentClass;//used to create the following tables and always instantly know, which maps to access

	map<string, map<string, Variable>> classLookupTables;//maps class name to class lookup table
	int staticIndex, fieldIndex;
	map<string, map<string, Variable>> functionLookupTables;//maps static function name to static function lookup table

	int argIndex, localIndex;

	//should be called when the current line is <classVarDec>
	void handleClassVariables();

	//should be called when the current line is <parameterList>
	void handleParameterList();

	//should be called when the current line is <varDec>
	void handleVarDeclaration();

	//\lookup tables--------------------------------------------------------------------
	
	//Code Generation-------------------------------------------------------------------

	//returns the vm translation for a variable with name x. for example, if its the first argument of a method, it returns "argument 0"
	string convertVariable(string name);

	string getVariableType(string name);
	//expressions

	void compileExpression();

	void compileOperator(string line);

	void compileTerm();
	
	//compiles an expressionList and returns the number of parameters. 
	int compileExpressionList();

	void compileSubroutineCall();

	void compileFunctionCall(string functionName, bool method);
	
	//statements

	void compileStatements();

	void compileLet();

	void compileReturn();

	void compileIf();

	void compileWhile();

	//objects

	void compileSubroutine();

	void compileConstructor();

	void compileFunction();

	//helpers

	void changeClass();

	int getParamCount(string methodName);

	int getLocalCount(string methodName);

	//\Code Generation-------------------------------------------------------------------
	 
	//Reading parser output-------------------------------------------------------------

	string getClassification(string line) {
		line = Utility::makeUniform(line);//remove blanks etc (not neccessary with my parser, but should work with any)
		string out;
		bool startWriting = false;
		for (char c : line) {
			if (c == '>') {
				return out;
			}
			if (startWriting == true) {
				out.push_back(c);
			}
			if (c == '<') {
				startWriting = true;
			}
		}
		return "Error: parser output corrupted.";
	}

	string getContent(string line) {
		line = Utility::makeUniform(line);//remove blanks etc (not neccessary with my parser, but should work with any)
		string out;
		bool startWriting = false;
		for (char c : line) {
			if (startWriting == true && c == '<') {
				return out;
			}
			if (startWriting == true) {
				out.push_back(c);
			}
			if (c == '>') {
				startWriting = true;
			}
		}
		return line;//then the line only has one classification and no content, and an be returned like that
	}
	//\Reading parser output------------------------------------------------------------

};

