#include "CodeGenerator.hpp"


vector<string> CodeGenerator::compile() {
	currentFunction = "Main.main";
	currentClass = "Main";
	while (true) {
		if (parserOutput[currentLineIndex].find("START_FILE_") != string::npos) {
			print(line);//leave this line untouched so that we can split the files again later
			if (hasMoreLines()) {
				advance();
			}
			else {
				break;
			}
		}
		else if (getLineType() == "class") {
			changeClass();
		}
		else if (getLineType() == "subroutineDec") {
			compileSubroutine();
		}
		//advance over all the rest, e.g. variable declarations and other unnessecary tokens
		else if (hasMoreLines()) {
			advance();
		}
		else {
			break;
		}
	}
	return compiled;
}

void CodeGenerator::changeClass() {
	advanceExpecting("<class>");
	advanceExpecting("class");
	currentClass = line;
}

void CodeGenerator::createLookupTables() {
	createOsLookupTables();
	while (true) {//we dont know how many lines the code below jumps, so we have to do the break condition at he end, and else advance
		if (line == "class") {
			advance();
			currentClass = line;//i + 1 is classname
			advance();
			classLookupTables[currentClass] = map<string, Variable>();
			staticIndex = 0;
			fieldIndex = 0;
		}
		else if (getLineType() == "subroutineDec") {
			string type = advance();
			advance(2);//advance 3 steps to the name of the function
			currentFunction = currentClass + "." + line;

			if (type == "method") {//if we have a method, the first argument is always this, so we need to start counting at 1
				argIndex = 1;
			}
			else {
				argIndex = 0;
			}

			functionLookupTables[currentFunction] = map<string, Variable>();
			localIndex = 0;
		}
		else if (getLineType() == "classVarDec") {
			handleClassVariables();//i is instantly incremented after this => we start after the end of the lines this worked through
		}
		else if (getLineType() == "parameterList") {
			handleParameterList();
		}
		else if (getLineType() == "varDec") {
			handleVarDeclaration();
		}
		else if (hasMoreLines()) {
			advance();
		}
		else {
			break;
		}
	}
	//we run through the entire code once to create the tables, to do the actual compiling we need to run through it again => reset
	reset();
}

void CodeGenerator::createOsLookupTables() {
	classLookupTables["Memory"] = map<string, Variable>();
	classLookupTables["Math"] = map<string, Variable>();
	classLookupTables["Output"] = map<string, Variable>();
	classLookupTables["Screen"] = map<string, Variable>();
	classLookupTables["Sys"] = map<string, Variable>();
	classLookupTables["Keyboard"] = map<string, Variable>();
	classLookupTables["Array"] = map<string, Variable>(); 
}

//should be called when the current line is <classVarDec>
void CodeGenerator::handleClassVariables() {
	advanceExpecting("<classVarDec>");
	string kind = line;
	string type = advance();
	advance();//advance to identifier
	while (hasMoreLines()) {
		if (getLineType() == "identifier") {
			int index;
			if (kind == "static") {
				index = staticIndex;
				staticIndex++;
			}
			else {
				index = fieldIndex;
				kind = "this";//so that we can instantly print the kind
				fieldIndex++;
			}
			string name = line; advance();
			classLookupTables[currentClass][name] = Variable(type, kind, index);
		}
		//now we have a , or ;, or we have the end of the declaration
		if (line == "</classVarDec>") {
			advanceExpecting("</classVarDec>");
			return;
		}
		advanceExpectingAny({ ",", ";" });
	}
}

//should be called when the current line is <parameterList>
void CodeGenerator::handleParameterList() {
	advanceExpecting("<parameterList>");
	while (hasMoreLines()) {
		if (getLineType() == "keyword" || getLineType() == "identifier") {
			string type = line;
			string name = advance();
			functionLookupTables[currentFunction][name] = Variable(type, "argument", argIndex++);
			advance();
		}
		if (line == "</parameterList>") {
			break;
		}
		advanceExpecting(",");
	}
	advanceExpecting("</parameterList>");
}

//should be called when the current line is <varDec>
void CodeGenerator::handleVarDeclaration() {
	advanceExpecting("<varDec>");
	advanceExpecting("var");
	while (hasMoreLines()) {
		string type = line;//go to the next from var, which is the type
		string name = advance();
		functionLookupTables[currentFunction][name] = Variable(type, "local", localIndex++);
		advance();//advance to , or ;
		while (line == ",") {//while there are commata, add the variables
			name = advance();
			functionLookupTables[currentFunction][name] = Variable(type, "local", localIndex++);//type stays the same
			advance();
		}
		advanceExpecting(";");
		if (line == "</varDec>") {//advance over semicolon
			break;
		}
	}
	advanceExpecting("</varDec>");
}
//\lookup tables--------------------------------------------------------------------

//Code Generation-------------------------------------------------------------------

//returns the vm translation for a variable with name x. for example, if its the first argument of a method, it returns "argument 0"
string CodeGenerator::convertVariable(string name) {
	if (functionLookupTables[currentFunction].count(name) > 0) {//first look up in method scope
		return functionLookupTables[currentFunction][name].kind + " " + to_string(functionLookupTables[currentFunction][name].index);
	}
	else if (classLookupTables[currentClass].count(name) > 0) {//then in class scope
		return classLookupTables[currentClass][name].kind + " " + to_string(classLookupTables[currentClass][name].index);
	}
	else {
 		return "";//in that case the identifier is a function or doesnt exist
	}
}

string CodeGenerator::getVariableType(string name) {
	if (functionLookupTables[currentFunction].count(name) > 0) {//first look up in method scope
		return functionLookupTables[currentFunction][name].type;
	}
	else if (classLookupTables[currentClass].count(name) > 0) {//then in class scope
		return classLookupTables[currentClass][name].type;
	}
	else {
		return "";//in that case the identifier is a function or doesnt exist
	}
}

//expressions
void CodeGenerator::compileExpression() {
	advanceExpecting("<expression>");
	compileTerm();
	while (true) {//still a term? Compile another term or a mathematical operation that combines terms
		if (getLineType() == "term") {
			bool negate = false;
			if (line == "-") {
				negate = true;
			}
			compileTerm();
		}
		else if (Utility::isOperator(line)) {
			string op = line;
			advance();
			compileTerm();
			compileOperator(op);
		}
		else {
			break;
		}
	}
	advanceExpecting("</expression>");
}

void CodeGenerator::compileOperator(string line) {
	if (line == "+") print("add");
	else if (line == "-") print("sub");
	else if (line == "*") print("call Math.multiply 2");
	else if (line == "/") print("call Math.divide 2");
	else if (line == "&lt;") print("lt");
	else if (line == "=") print("eq");
	else if (line == "&amp;") print("and");
	else if (line == "&gt;") print("gt");
	else if (line == "|") print("or");
}

void CodeGenerator::compileTerm() {
	advanceExpecting("<term>");
	if (getLineType() == "integerConstant") {
		print("push constant " + line);
		advance();
	}
	else if (getLineType() == "stringConstant") {
		print("push constant " + to_string(line.size()));
		print("call String.new 1");
		for (char c : line) {
			print("push constant " + to_string((int)(c)));
			print("call String.appendChar 2");
		}
		advance();
	}
	else if (getLineType() == "identifier") {//function call or variable
		string var = convertVariable(line);
		if (var != "" && getContent(parserOutput[currentLineIndex + 1]) != ".") {
			print("push " + var);
			advance();
			if (line == "[") {
				advanceExpecting("[");
				compileExpression();
				print("add");//add array start address (from push var) to value of expression
				print("pop pointer 1");//save reference to element in THAT
				print("push that 0");//push value of element, which's reference is in THAT to stack
				advanceExpecting("]");
			}
		}
		else {
			compileSubroutineCall();
		}
	}
	//constants
	else if (getLineType() == "keyword") {
		if (line == "true") {
			print("push constant 0");
			print("not");
		}
		else if (line == "false") {
			print("push constant 0");			
		}
		else if (line == "null") {
			print("push constant 0");
		}
		else if (line == "this") {
			print("push pointer 0");
		}
		advance();
	}
	else if (Utility::isUnaryOperator(line)) {
		string op = line;
		advance();
		compileTerm();
		if (op == "-") {
			print("neg");
		}
		else {
			print("not");
		}
	}
	else if (line == "(") {
		advance();//advance over (
		compileExpression();
		advance();//advance over )
	}
	else if (getLineType() == "expressionList") {
		compileExpressionList();
	}
	advanceExpecting("</term>");
}

int CodeGenerator::compileExpressionList() {
	int paramCount = 0;
	advanceExpecting("<expressionList>");
	while (getLineType() == "expression") {
		paramCount++;
		compileExpression();
		if (line == ",") {
			advance();//advance over comma
		}
	}
	advanceExpecting("</expressionList>");
	return paramCount;
}

void CodeGenerator::compileSubroutineCall() {
	bool doStatement = false;
	if (getLineType() == "doStatement") {
		doStatement = true;
		advanceExpecting("<doStatement>");
		advanceExpecting("do");
	}
	if (getContent(parserOutput[currentLineIndex + 1]) == ".") {
		string classOrObjectOrFunctionName = line;
		advance();
		//look for method name
		if (classLookupTables.count(classOrObjectOrFunctionName) > 0) {//its a class name, thus we try calling statically
			advance();//line is now the functions name
			compileFunctionCall(classOrObjectOrFunctionName, false);
		}
		else {//its an object, so we try calling the function on the object
			string object = convertVariable(classOrObjectOrFunctionName);
			if (object == "") {
				cout << "Object " << object << " does not exist.";
				exit(0);
			}
			else {
				print("push " + object);//push the object
				string objectClass = getVariableType(classOrObjectOrFunctionName);
				advance();//line is now the functions name
				compileFunctionCall(objectClass, true);
			}
		}
	}
	else {//the method is in the same class
		print("push pointer 0");//push this as the first argument instead of an object
		compileFunctionCall(currentClass, true);//then the function is in the current class
	}

	if (doStatement == true) {
		print("pop temp 0");//discard output
		advanceExpecting(";");//do-statements always end in ;
		advanceExpecting("</doStatement>");
	}
}

//call only wenn line is the functions name
void CodeGenerator::compileFunctionCall(string className, bool method) {
	string functionName = className + "." + line;
	advance();//advance to '('
	advanceExpecting("(");
	int paramCount = compileExpressionList();
	if (method) {
		paramCount++;
	}
	advanceExpecting(")");
	print("call " + functionName + " " + to_string(paramCount));
}

//statements

void CodeGenerator::compileStatements() {
	advanceExpecting("<statements>");
	while (true) {
		if (getLineType() == "letStatement") compileLet();
		else if (getLineType() == "returnStatement") compileReturn();
		else if (getLineType() == "ifStatement") compileIf();
		else if (getLineType() == "whileStatement") compileWhile();
		else if (getLineType() == "doStatement") {
			compileSubroutineCall();
		}
		else {
			break;//no statements anymore
		}
	}
	advanceExpecting("</statements>");
}

void CodeGenerator::compileLet() {
	advanceExpecting("<letStatement>");
	advanceExpecting("let");

	string var = convertVariable(line);
	bool arr = false;
	advance();

	if (line == "[") {
		arr = true;
		advanceExpecting("[");
		compileExpression();
		advanceExpecting("]");
		print("push " + var);
		print("add");//i + arr
		//print("pop temp 1");//put the address of the array element to temp. Not yet to pointer 1, because compileExpression might override that!
	}

	advanceExpecting("=");
	compileExpression();
	if (arr == true) {
		print("pop temp 0");//put expression into temp. the index in the array is now the TOS.
		print("pop pointer 1");
		print("push temp 0");//get value of expression back
		print("pop that 0");//push to array
	}
	else {
		print("pop " + var);
	}
	advanceExpecting(";");
	advanceExpecting("</letStatement>");
}

void CodeGenerator::compileReturn() {
	advanceExpecting("<returnStatement>");
	advanceExpecting("return");
	if (line == ";") {//if there is no return value, push a dummy 0 onto the stack
		print("push constant 0");
	}
	else {
		compileExpression();
	}
	print("return");
	advanceExpecting(";");
	advanceExpecting("</returnStatement>");
}


void CodeGenerator::compileIf() {
	advanceExpecting("<ifStatement>");
	advanceExpecting("if");
	advanceExpecting("(");
	compileExpression();
	advanceExpecting(")");
	advanceExpecting("{");

	string tempCounter = to_string(ifCounter++);
	print("if-goto IF_TRUE" + tempCounter);

	print("goto IF_FALSE" + tempCounter);
	print("label IF_TRUE" + tempCounter);

	if (getLineType() == "statements") {
		compileStatements();//statements executed if we dont jump at first. 
	}

	advanceExpecting("}");
	if (line == "else") {
		advanceExpecting("else");
		advanceExpecting("{");

		print("goto IF_END" + tempCounter);
		print("label IF_FALSE" + tempCounter);
		compileStatements();
		print("label IF_END" + tempCounter);

		advanceExpecting("}");
	}
	else {
		print("label IF_FALSE" + tempCounter);
	}
	advanceExpecting("</ifStatement>");
}

void CodeGenerator::compileWhile() {
	advanceExpecting("<whileStatement>");
	string tempCounter = to_string(whileCounter++);//save because this can be changed in the statements
	print("label WHILE_EXP" + tempCounter);//start
	advanceExpecting("while");
	advanceExpecting("(");
	compileExpression();
	print("not");
	print("if-goto WHILE_END" + tempCounter);//goto end
	
	advanceExpecting(")");
	advanceExpecting("{");
	compileStatements();
	print("goto WHILE_EXP" + tempCounter);//goto start
	advanceExpecting("}");

	print("label WHILE_END" + tempCounter);//end
	advanceExpecting("</whileStatement>");
}

//Objects

void CodeGenerator::compileSubroutine() {
	advanceExpecting("<subroutineDec>");
	if (line == "constructor") compileConstructor();
	if (line == "method" || line == "function") compileFunction();
	advanceExpecting("</subroutineDec>");
}

void CodeGenerator::compileConstructor() {
	advanceExpecting("constructor");
	string className = line;
	advance();
	advanceExpecting("new");

	string functionName = className + ".new";
	currentFunction = functionName;//change the scopes every time a method is compiled, so that convertVariable works properly
	print("function " + functionName + " " + to_string(getLocalCount(functionName)));
	//get amount of fields in the class
	int fieldCount = 0;
	for (auto pair : classLookupTables[className]) {
		if (pair.second.kind == "this") {
			fieldCount++;
		}
	}
	//allocate as many words as there are fields in the class
	print("push constant " + to_string(fieldCount));
	print("call Memory.alloc 1");
	print("pop pointer 0");

	while (getLineType() != "statements") {
		advance();
	}
	compileStatements();//compile method body
	advanceExpecting("}");
	advanceExpecting("</subroutineBody>");
}

void CodeGenerator::compileFunction() {
	bool method = false;
	if (line == "method") {
		method = true;
	}
	advanceExpectingAny({ "method", "function" });
	advance();//go to function name
	currentFunction = currentClass + "." + line;//change the scopes every time a method is compiled, so that convertVariable works properly
	print("function " + currentFunction + " " + to_string(getLocalCount(currentFunction)));
	if (method) {//set this correctly if method
		print("push argument 0");
		print("pop pointer 0");
	}
	while (getLineType() != "statements") {
		advance();
	}
	compileStatements();
	advanceExpecting("}");
	advanceExpecting("</subroutineBody>");//neccessary so that the methos ends with </subroutineDec>, which compileSubroutine() expects
}

int CodeGenerator::getParamCount(string methodName) {
	int paramCount = 0;
	for (auto pair : functionLookupTables[methodName]) {
		if (pair.second.kind == "argument") {
			paramCount++;
		}
	}
	return paramCount;
}

int CodeGenerator::getLocalCount(string methodName) {
	int paramCount = 0;
	for (auto pair : functionLookupTables[methodName]) {
		if (pair.second.kind == "local") {
			paramCount++;
		}
	}
	return paramCount;
}