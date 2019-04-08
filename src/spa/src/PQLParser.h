#pragma once
#include "ExprParser.h"
#include "GeneralLexer.h"
#include "Query.h"
#include <map>
#include <memory>

namespace Parser {
enum class RefType { EntRef, StmtRef, LineRef };

namespace PQLTokens {
// Design Entities
const static Lexer::Token Stmt{Lexer::TokenType::Identifier, "stmt"};
const static Lexer::Token Read{Lexer::TokenType::Identifier, "read"};
const static Lexer::Token Print{Lexer::TokenType::Identifier, "print"};
const static Lexer::Token Call{Lexer::TokenType::Identifier, "call"};
const static Lexer::Token While{Lexer::TokenType::Identifier, "while"};
const static Lexer::Token If{Lexer::TokenType::Identifier, "if"};
const static Lexer::Token Assign{Lexer::TokenType::Identifier, "assign"};
const static Lexer::Token Variable{Lexer::TokenType::Identifier, "variable"};
const static Lexer::Token Constant{Lexer::TokenType::Identifier, "constant"};
const static Lexer::Token Prog{Lexer::TokenType::Identifier, "prog"};
const static Lexer::Token Line{Lexer::TokenType::Identifier, "line"};
const static Lexer::Token Procedure{Lexer::TokenType::Identifier, "procedure"};

// Selectors
const static Lexer::Token Select{Lexer::TokenType::Identifier, "Select"};
const static Lexer::Token Such{Lexer::TokenType::Identifier, "such"};
const static Lexer::Token That{Lexer::TokenType::Identifier, "that"};
const static Lexer::Token Pattern{Lexer::TokenType::Identifier, "pattern"};
const static Lexer::Token With{Lexer::TokenType::Identifier, "with"};

// Relations
const static Lexer::Token Modifies{Lexer::TokenType::Identifier, "Modifies"};
const static Lexer::Token Uses{Lexer::TokenType::Identifier, "Uses"};
const static Lexer::Token Calls{Lexer::TokenType::Identifier, "Calls"};
const static Lexer::Token Parent{Lexer::TokenType::Identifier, "Parent"};
const static Lexer::Token Follows{Lexer::TokenType::Identifier, "Follows"};
const static Lexer::Token Next{Lexer::TokenType::Identifier, "Next"};
const static Lexer::Token Affects{Lexer::TokenType::Identifier, "Affects"};
const static Lexer::Token NextBip{Lexer::TokenType::Identifier, "NextBip"};
const static Lexer::Token AffectsBip{Lexer::TokenType::Identifier,
                                     "AffectsBip"};

// Other keywords
const static Lexer::Token Boolean{Lexer::TokenType::Identifier, "BOOLEAN"};
const static Lexer::Token And{Lexer::TokenType::Identifier, "and"};

// Symbols
const static Lexer::Token Underscore{Lexer::TokenType::Delimiter, "_"};
const static Lexer::Token LeftParentheses{Lexer::TokenType::Delimiter, "("};
const static Lexer::Token RightParentheses{Lexer::TokenType::Delimiter, ")"};
const static Lexer::Token LeftChevron{Lexer::TokenType::Delimiter, "<"};
const static Lexer::Token RightChevron{Lexer::TokenType::Delimiter, ">"};
const static Lexer::Token Semicolon{Lexer::TokenType::Delimiter, ";"};
const static Lexer::Token Comma{Lexer::TokenType::Delimiter, ","};
const static Lexer::Token Period{Lexer::TokenType::Delimiter, "."};
const static Lexer::Token Equals{Lexer::TokenType::Operator, "="};
const static Lexer::Token Star{Lexer::TokenType::Operator, "*"};
const static Lexer::Token Quote{Lexer::TokenType::Delimiter, "\""};

// General
const static Lexer::Token Identifier{Lexer::TokenType::Identifier, ""};
const static Lexer::Token Number{Lexer::TokenType::Number, ""};

static std::map<Lexer::Token, QueryEntityType> tokenEntityMap = {
    std::make_pair(PQLTokens::Stmt, QueryEntityType::Stmt),
    std::make_pair(PQLTokens::Read, QueryEntityType::Read),
    std::make_pair(PQLTokens::Print, QueryEntityType::Print),
    std::make_pair(PQLTokens::Call, QueryEntityType::Call),
    std::make_pair(PQLTokens::While, QueryEntityType::While),
    std::make_pair(PQLTokens::If, QueryEntityType::If),
    std::make_pair(PQLTokens::Assign, QueryEntityType::Assign),
    std::make_pair(PQLTokens::Variable, QueryEntityType::Variable),
    std::make_pair(PQLTokens::Constant, QueryEntityType::Constant),
    std::make_pair(PQLTokens::Prog, QueryEntityType::Progline),
    std::make_pair(PQLTokens::Procedure, QueryEntityType::Procedure)};

static std::set<Lexer::Token> transitiveRel = {
    PQLTokens::Calls,     PQLTokens::Parent,  PQLTokens::Follows,
    PQLTokens::Next,      PQLTokens::Affects, PQLTokens::NextBip,
    PQLTokens::AffectsBip};
}; // namespace PQLTokens

class PQLParser {
private:
  std::list<Lexer::Token> tokens;
  Query query;
  std::map<std::string, QueryEntityType> declarations;

  // Parsing
  Lexer::Token expect(Lexer::Token token);
  void parseQuery();
  void parseDeclaration();
  void parseClauses();

  // Such that
  void parseResultCl();
  void parseTuple();

  // With
  void parseWithCl();
  void parseAttrCond();
  void parseAttrCompare();
  void parseAttrRef();

  // Such That
  void parseSuchThatCl();
  void parseRelCond();
  void parseRelRef();

  // Pattern
  void parsePatternCl();
  void parsePatternCond();
  void parsePattern();
  void parseAssign();
  void parseWhile();
  void parseIf();
  void parseExprSpec();
  std::string parseQuotedExpr();

  // General
  void parseRef();
  void parseGenRef();
  void parseEntRef();
  void parseStmtRef();
  void parseLineRef();
  void parseElem();
  void parseSynonym();

public:
  PQLParser(std::list<Lexer::Token>);
  Query getQuery();
};

Query parsePQL(std::list<Lexer::Token>);
} // namespace Parser
