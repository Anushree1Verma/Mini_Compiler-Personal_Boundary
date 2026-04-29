#ifndef LEXER_H
#define LEXER_H


#include <iostream>
#include <vector>
#include <string>
using namespace std;

enum TokenType {
    ACTION,
    NEGATION,
    SUBJECT,
    RELATION,
    NUMBER,
    MERIDIEM,
    TIME_KEYWORD,
    IF_KW,
    CONNECTOR,
    COND_SUBJECT,
    COND_VERB,
    COND_STATE,
    COND_VALUE,
    UNKNOWN
};

struct Token {
    string value;
    TokenType type;
    Token(string v, TokenType t) : value(v), type(t) {}
};

class Lexer {
    string input;
    string toUpper(const string& s);

public:
    Lexer(string in);
    vector<Token> tokenize();
    static string typeToString(TokenType t);
};

#endif
