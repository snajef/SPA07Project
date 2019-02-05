#include <iostream>
#include <fstream>

#include "Parser.h"
#include "Lexer.h"

using std::ifstream;
using std::invalid_argument;
using std::logic_error;
using std::cout;
using std::getline;

Parser::Parser() {
	token = NULL;
}
Parser::~Parser() {}

queue<Token*> Parser::parse(string filePath) {
	ifstream inputFile;
	inputFile.open(filePath);
	if (!inputFile) {
		throw invalid_argument("FileNotFoundException: " + filePath);
	}
	Lexer lexer;
	string line;
	queue<Token*> tokenQueue;
	while (getline(inputFile, line)) {
		vector<Token*> tokens = lexer.tokenize(line);
		for (Token* token : tokens) {
			tokenQueue.push(token);
		}
	}
	inputFile.close();
	return tokenQueue;
}

TNode* Parser::buildAST(queue<Token*> &tokenQueue) {
	this->tokenQueue = tokenQueue;
	expectToken("procedure");
	TNode* procedureTNode;
	try {
		procedureTNode = createTNodeProcedure();
	} catch (const string e) {
		cout << e;
	}
	return procedureTNode;
}

void Parser::getNextToken() {
	Token* nextToken = tokenQueue.front();
	tokenQueue.pop();
	token = nextToken;
}

void Parser::expectToken(string expectedToken) {
	getNextToken();
	if (token->getName() != expectedToken) {
		throw logic_error("Expected '" + expectedToken + "' but got '" + token->getName() + "'");
	}
}

TNode* Parser::createTNodeProcedure() {
	getNextToken();
	TNode* procedureTNode = new TNode(TNodeType::Procedure, token->getName(), token->getStatementNumber());
	expectToken("{");
	TNode* stmtLstTNode = createTNodeStatementList();
	expectToken("}");
	procedureTNode->setChildren({ stmtLstTNode });
	return procedureTNode;
}

TNode* Parser::createTNodeAssign() {
	TNode* variableTNode = new TNode(TNodeType::Variable, token->getName(), token->getStatementNumber());
	expectToken("=");
	TNode* expressionTNode = createTNodeExpression();
	expectToken(";");
	return createTNode(TNodeType::Assign, { variableTNode, expressionTNode });
}

TNode* Parser::createTNodeRead() {
	getNextToken(); // token holding var_name
	TNode* variableTNode = new TNode(TNodeType::Variable, token->getName(), token->getStatementNumber());
	expectToken(";");
	return createTNode(TNodeType::Read, { variableTNode });
}

TNode* Parser::createTNodePrint() {
	getNextToken(); // token holding var_name
	TNode* variableTNode = new TNode(TNodeType::Variable, token->getName(), token->getStatementNumber());
	expectToken(";");
	return createTNode(TNodeType::Print, { variableTNode });
}

TNode* Parser::createTNodeWhile() {
	expectToken("(");
	TNode* condExpressionTNode = createTNodeConditionExpression();
	expectToken(")");
	expectToken("{");
	TNode* stmtLstTNode = createTNodeStatementList();
	expectToken("}");
	return createTNode(TNodeType::While, { condExpressionTNode, stmtLstTNode });
}

TNode* Parser::createTNodeIf() {
	expectToken("(");
	TNode* condExpressionTNode = createTNodeConditionExpression();
	expectToken(")");
	expectToken("then");
	expectToken("{");
	TNode* thenStmtLstTNode = createTNodeStatementList();
	expectToken("}");
	expectToken("else");
	expectToken("{");
	TNode* elseStmtLstTNode = createTNodeStatementList();
	expectToken("}");
	return createTNode(TNodeType::If, { condExpressionTNode, thenStmtLstTNode, elseStmtLstTNode });
}

TNode* Parser::createTNodeStatementList() {
	TNode* stmtLstTNode = new TNode(TNodeType::StatementList);
	vector<TNode*> stmtTNodes;
	while (tokenQueue.front()->getName() != "}") {
		stmtTNodes.push_back(createTNodeStatement());
		if (tokenQueue.empty()) {
			throw logic_error("Missing '}'");
		}
	}
	if (stmtTNodes.empty()) {
		throw logic_error("Expected at least 1 statement but got 0");
	}
	stmtLstTNode->setChildren(stmtTNodes);
	return stmtLstTNode;
}

TNode* Parser::createTNodeStatement() {
	getNextToken(); // holding token after { or ;
	switch (token->getType()) {
	case TokenType::Identifier:
		return createTNodeAssign();
	case TokenType::Read:
		return createTNodeRead();
	case TokenType::Print:
		return createTNodePrint();
	case TokenType::While:
		return createTNodeWhile();
	case TokenType::If:
		return createTNodeIf();
	default:
		throw invalid_argument("Expected TokenType to be 'Identifier', 'Read', 'Print', 'While' or 'If' but got '" + token->getName() + "'");
	}
}

TNode* Parser::createTNodeConditionExpression() {
	TNode* condExpressionTNode;
	TNode* leftCondExpressionTNode;
	TNode* rightCondExpressionTNode;
	switch (tokenQueue.front()->getType()) {
	case TokenType::ExclamationMark:
		expectToken("!");
		expectToken("(");
		condExpressionTNode = createTNodeConditionExpression();
		expectToken(")");
		return createTNode(TNodeType::Not, { condExpressionTNode });
	case TokenType::OpenParenthesis:
		expectToken("(");
		leftCondExpressionTNode = createTNodeConditionExpression();
		expectToken(")");
		switch (tokenQueue.front()->getType()) {
		case TokenType::And:
			expectToken("(");
			rightCondExpressionTNode = createTNodeConditionExpression();
			expectToken(")");
			return createTNode(TNodeType::And, { leftCondExpressionTNode, rightCondExpressionTNode });
		case TokenType::Or:
			expectToken("(");
			rightCondExpressionTNode = createTNodeConditionExpression();
			expectToken(")");
			return createTNode(TNodeType::And, { leftCondExpressionTNode, rightCondExpressionTNode });
		default:
			expectToken("&&, ||");
		}
	default:
		return createTNodeRelativeExpression();
	}
}

TNode* Parser::createTNodeRelativeExpression() {
	TNode* leftRelFactor = createTNodeRelativeFactor();
	TNode* rightRelFactor;
	switch (tokenQueue.front()->getType()) {
	case TokenType::Greater:
		rightRelFactor = createTNodeRelativeFactor();
		return createTNode(TNodeType::Greater, { leftRelFactor, rightRelFactor });
	case TokenType::GreaterThanOrEqual:
		rightRelFactor = createTNodeRelativeFactor();
		return createTNode(TNodeType::GreaterThanOrEqual, { leftRelFactor, rightRelFactor });
	case TokenType::Lesser:
		rightRelFactor = createTNodeRelativeFactor();
		return createTNode(TNodeType::Lesser, { leftRelFactor, rightRelFactor });
	case TokenType::LesserThanOrEqual:
		rightRelFactor = createTNodeRelativeFactor();
		return createTNode(TNodeType::LesserThanOrEqual, { leftRelFactor, rightRelFactor });
	case TokenType::Equal:
		rightRelFactor = createTNodeRelativeFactor();
		return createTNode(TNodeType::Equal, { leftRelFactor, rightRelFactor });
	case TokenType::NotEqual:
		rightRelFactor = createTNodeRelativeFactor();
		return createTNode(TNodeType::NotEqual, { leftRelFactor, rightRelFactor });
	default:
		expectToken(">, >=, <, <=, ==, !=");
	}
}

TNode* Parser::createTNodeRelativeFactor() {
	getNextToken();
	switch (token->getType()) {
	case TokenType::Identifier:
		return new TNode(TNodeType::Variable, token->getName(), token->getStatementNumber());
	case TokenType::Constant:
		return new TNode(TNodeType::Constant, token->getName(), token->getStatementNumber());
	default:
		return createTNodeExpression();
	}
}

TNode* Parser::createTNodeExpression() {
	TNode* termTNode = createTNodeTerm();
	TNode* expressionTNode;
	switch (tokenQueue.front()->getType()) {
	case TokenType::Plus:
		getNextToken();
		expressionTNode = createTNodeExpression();
		return createTNode(TNodeType::Plus, { expressionTNode, termTNode });
	case TokenType::Minus:
		getNextToken();
		expressionTNode = createTNodeExpression();
		return createTNode(TNodeType::Minus, { expressionTNode, termTNode });
	default:
		return termTNode;
	}
}

TNode* Parser::createTNodeTerm() {
	TNode* factorTNode = createTNodeFactor();
	TNode* termTNode;
	switch (tokenQueue.front()->getType()) {
	case TokenType::Multiply:
		getNextToken();
		termTNode = createTNodeTerm();
		return createTNode(TNodeType::Multiply, { termTNode, factorTNode });
	case TokenType::Divide:
		getNextToken();
		termTNode = createTNodeTerm();
		return createTNode(TNodeType::Divide, { termTNode, factorTNode });
	case TokenType::Mod:
		getNextToken();
		termTNode = createTNodeTerm();
		return createTNode(TNodeType::Mod, { termTNode, factorTNode });
	default:
		return factorTNode;
	}
}

TNode* Parser::createTNodeFactor() {
	getNextToken();
	switch (token->getType()) {
	case TokenType::Identifier:
		return new TNode(TNodeType::Variable, token->getName(), token->getStatementNumber());
	case TokenType::Constant:
		return new TNode(TNodeType::Constant, token->getName(), token->getStatementNumber());
	case TokenType::OpenParenthesis:
		TNode* expressionTNode = createTNodeExpression();
		expectToken(")");
		return expressionTNode;
	}
}

TNode* Parser::createTNode(TNodeType type, vector<TNode*> children) {
	TNode* tNode = new TNode(type);
	tNode->setChildren(children);
	return tNode;
}
