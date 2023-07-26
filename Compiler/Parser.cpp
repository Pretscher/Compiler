#include "Parser.hpp"

void Parser::advance() {
	tokenizer->advanceToken();
	currentToken = tokenizer->getLine();
}

void Parser::process(const string& str) {
	if (currentToken == str) {
		print(tokenizer->getXMLToken());
		if (tokenizer->hasMoreLines()) {
			advance();
		}
	}
	else {
		std::cout << str + " expected, but was " + currentToken;
		std::exit(0);
	}
}

//Statements

void Parser::parseStatement() {
	print("<statements>");
	while (currentToken != "}") {
		if (currentToken == "while") {
			parseWhile();
		}
		else if (currentToken == "let") {
			parseLet();
		}
		else if (currentToken == "if") {
			parseIf();
		}
		else if (currentToken == "do") {
			parseDo();
		}
		else if (currentToken == "return") {
			parseReturn();
		}
		else {
			std::cout << "Statement expected instead of " << currentToken;
			std::exit(0);
		}
	}
	print("</statements>");
}

void Parser::parseWhile() {
	print("<whileStatement>");
	process("while");
	process("(");
	parseExpression();
	process(")");
	process("{");
	parseStatement();
	process("}");
	print("</whileStatement>");
}

void Parser::parseLet() {
	print("<letStatement>");
	process("let");
	parseIdentifier();
	if (currentToken == "[") {
		process("[");
		parseExpression();
		process("]");
	}
	process("=");
	parseExpression();
	process(";");
	print("</letStatement>");
}

void Parser::parseIf() {
	print("<ifStatement>");
	process("if");
	process("(");
	parseExpression();
	process(")");
	process("{");
	parseStatement();
	process("}");
	if (currentToken == "else") {
		process("else");
		process("{");
		parseStatement();
		process("}");
	}
	print("</ifStatement>");
}

void Parser::parseDo() {
	print("<doStatement>");
	process("do");
	parseSubroutineCall();
	process(";");
	print("</doStatement>");
}

void Parser::parseReturn() {
	print("<returnStatement>");
	process("return");
	if (currentToken != ";") {
		parseExpression();
	}
	process(";");
	print("</returnStatement>");
}

//Expressions

void Parser::parseExpressionList() {
	print("<expressionList>");
	while (currentToken != ")") {//every expressionlist ends with a )
		parseExpression();
		if (currentToken == ")") {
			break;
		}
		process(",");
	}
	print("</expressionList>");
}

void Parser::parseExpression() {
	print("<expression>");
	parseTerm();
	while (isOperator(currentToken)) {
		process(currentToken);
		parseTerm();
	}
	print("</expression>");
}

void Parser::parseTerm() {
	print("<term>");
	vector<string> remainingTokens = tokenizer->getRemainingTokens();
	string type = tokenizer->getTokenType(remainingTokens[0]);
	if (type == "integerConstant" || type == "stringConstant" || isKeywordConstant(currentToken)) {
		process(currentToken);
	}
	else if (isSubroutineCall()) {
		parseSubroutineCall();
	}
	else if (type == "identifier") {
		parseIdentifier();
		if (currentToken == "[") {
			process("[");
			parseExpression();
			process("]");
		}
	}
	else if (currentToken == "(") {
		process("(");
		parseExpression();
		process(")");
	}
	else if (isUnaryOperator(currentToken)) {
		process(currentToken);
		parseTerm();
	}
	print("</term>");
}

void Parser::parseSubroutineCall() {
	parseIdentifier();
	if (currentToken == ".") {//if it is the method of an object
		process(".");
		parseIdentifier();
	}
	process("(");
	parseExpressionList();
	process(")");
}

bool Parser::isSubroutineCall() {
	vector<string> remainingTokens = tokenizer->getRemainingTokens();
	if (tokenizer->getTokenType(remainingTokens[0]) == "identifier") {
		if (remainingTokens[1] == "." && tokenizer->getTokenType(remainingTokens[2]) == "identifier" && remainingTokens[3] == "(") {//object function needs a second identifier
			return true;
		}
		if (remainingTokens[1] == "(") return true;//normal function
	}
	return false;
}

//program structure

void Parser::parseClass() {
	print("<class>");
	process("class");
	parseIdentifier();
	process("{");
	parseClassVarDeclaration();
	parseSubroutineDeclaration();
	process("}");
	print("</class>");
}

void Parser::parseClassVarDeclaration() {
	while (currentToken == "static" || currentToken == "field") {
		print("<classVarDec>");
		//a single line
		process(currentToken);
		if (isType(currentToken)) {
			process(currentToken);
			parseIdentifier();
			while (currentToken == ",") {
				process(",");
				parseIdentifier();
			}
		}
		else {
			std::cout << "Unrecognized type " << currentToken;
			std::exit(0);
		}
		process(";");
		print("</classVarDec>");
	}
}

void Parser::parseSubroutineDeclaration() {
	while (currentToken == "constructor" || currentToken == "function" || currentToken == "method") {
		print("<subroutineDec>");
		process(currentToken);
		if (currentToken == "void" || isType(currentToken)) {
			process(currentToken);
			parseIdentifier();
			process("(");
			parseParameterList();
			process(")");
			parseSubroutineBody();
		}
		else {
			std::cout << "Invalid return type " << currentToken;
			std::exit(0);
		}
		print("</subroutineDec>");
	}
}

void Parser::parseParameterList() {
	print("<parameterList>");
	while (true) {
		if (currentToken == ")") {
			break;
		}
		if (isType(currentToken)) {
			process(currentToken);
		}
		else {
			std::cout << "Type expected instead of " << currentToken;
			std::exit(0);
		}
		parseIdentifier();
		if (currentToken == ")") {
			break;
		}
		process(",");//process a coma and continue the loop of the list is not closed
	}
	print("</parameterList>");
}

void Parser::parseSubroutineBody() {
	print("<subroutineBody>");
	process("{");
	parseVarDec();
	parseStatement();
	process("}");
	print("</subroutineBody>");
}

void Parser::parseVarDec() {
	while (currentToken == "var") {
		print("<varDec>");//we want to print this every time a variable is declared
		process("var");
		if (isType(currentToken)) {
			process(currentToken);
			parseIdentifier();
			while (currentToken == ",") {
				process(",");
				parseIdentifier();
			}
			process(";");
		}
		else {
			std::cout << "Type expected instead of " << currentToken;
			std::exit(0);
		}
		print("</varDec>");
	}

}

void Parser::parseIdentifier() {
	if (tokenizer->getTokenType(currentToken) != "identifier") {
		std::cout << "Identifier expected instead of " << currentToken;
		std::exit(0);
	}
	print(tokenizer->getXMLToken());
	advance();
}
