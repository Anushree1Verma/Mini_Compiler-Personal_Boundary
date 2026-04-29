#ifndef SEMANTIC_H
#define SEMANTIC_H



#include "parser.h"
#include <vector>
#include <string>

struct SemanticError {
    string message;
    string suggestion;

    SemanticError(string m, string s = "") : message(m), suggestion(s) {}
};

class SemanticAnalyser {
    PolicyRule& rule;
    vector<SemanticError> errors;

    void checkTimeRange();
    void checkBetweenOrder();
    void checkConditionValidity();
    void checkSubjectConsistency();

public:
    SemanticAnalyser(PolicyRule& r);
    bool analyse();

    const vector<SemanticError>& getErrors() const;
    void printErrors() const;
    void printWarnings() const;
};

#endif
