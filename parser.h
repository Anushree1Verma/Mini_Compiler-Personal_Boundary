#ifndef PARSER_H
#define PARSER_H



#include "lexer.h"
#include <vector>
#include <string>


struct TimeExpr {
    int hour;      
    int minute;     
    string raw;     

    TimeExpr() : hour(-1), minute(0), raw("") {}
    TimeExpr(int h, int m, string r) : hour(h), minute(m), raw(r) {}
};

struct TimeClause {
    string relation;    
    TimeExpr timeA;     
    TimeExpr timeB;     
    bool hasTwoTimes;   
    TimeClause() : relation(""), hasTwoTimes(false) {}
};


struct ConditionExpr {
    string subject;   
    string stateVerb; 
    string modifier;  
    string value;     
    string connector; 

    ConditionExpr() : subject(""), stateVerb(""), modifier(""), value(""), connector("") {}
};


struct PolicyRule {
    string action;      
    bool negated;       
    string rawAction;   
    vector<string> subjects;

    TimeClause timeClause;

   
    bool hasCondition;
    vector<ConditionExpr> conditions;

    PolicyRule() : action(""), negated(false), rawAction(""), hasCondition(false) {}

    void print() const;

    string toJSON() const;
};
class Parser {
    vector<Token> tokens;
    int current;

    Token peek();
    Token advance();
    bool check(TokenType t);
    bool match(TokenType t);
    void error(string msg);

    
    void parseAction(PolicyRule& rule);
    void parseSubject(PolicyRule& rule);
    TimeClause parseTimeClause();
    TimeExpr parseTimeExpr();
    vector<ConditionExpr> parseCondition();

public:
    Parser(vector<Token> t);
    PolicyRule parseRule();
};

#endif
