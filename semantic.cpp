#include "semantic.h"
#include <iostream>


SemanticAnalyser::SemanticAnalyser(PolicyRule& r) : rule(r) {}

void SemanticAnalyser::checkTimeRange() {
    auto& A = rule.timeClause.timeA;
    if (A.hour < 0 || A.hour > 23) {
        errors.push_back(SemanticError(
            "Time hour " + to_string(A.hour) + " is out of range (0-23)",
            "Use a valid hour, e.g. '10 PM' or '22:00'"
        ));
    }
    if (A.minute < 0 || A.minute > 59) {
        errors.push_back(SemanticError(
            "Time minute " + to_string(A.minute) + " is out of range (0-59)",
            "Use minutes between 00 and 59, e.g. '10:30 PM'"
        ));
    }

    if (rule.timeClause.hasTwoTimes) {
        auto& B = rule.timeClause.timeB;
        if (B.hour < 0 || B.hour > 23) {
            errors.push_back(SemanticError(
                "Second time hour " + to_string(B.hour) + " is out of range",
                "Use a valid hour, e.g. '5 PM'"
            ));
        }
    }
}

void SemanticAnalyser::checkBetweenOrder() {
    if (rule.timeClause.relation != "BETWEEN") return;
    if (!rule.timeClause.hasTwoTimes) {
        errors.push_back(SemanticError(
            "BETWEEN requires two times, e.g. 'between 2 PM and 5 PM'"
        ));
        return;
    }

    auto& A = rule.timeClause.timeA;
    auto& B = rule.timeClause.timeB;

    int minA = A.hour * 60 + A.minute;
    int minB = B.hour * 60 + B.minute;

    if (minA == minB) {
        errors.push_back(SemanticError(
            "BETWEEN start and end times are the same (" + A.raw + ")",
            "Use two different times, e.g. 'between 2 PM and 5 PM'"
        ));
    }
}

void SemanticAnalyser::checkConditionValidity() {
    if (!rule.hasCondition) return;

    for (const auto& c : rule.conditions) {
        if (c.subject.empty() && c.value.empty() && c.modifier.empty()) {
            errors.push_back(SemanticError(
                "Condition is incomplete — expected a device state or mode",
                "Try: 'if device is on DND' or 'if focus mode is active'"
            ));
        }

        if (!c.modifier.empty() && c.value.empty() && c.subject.empty()) {
            errors.push_back(SemanticError(
                "Condition modifier '" + c.modifier + "' without a subject or value is ambiguous",
                "Try: 'if device is on DND'"
            ));
        }
    }
}

void SemanticAnalyser::checkSubjectConsistency() {
    bool hasAll = false;
    for (const auto& s : rule.subjects) {
        if (s == "ALL" || s == "EVERYTHING") hasAll = true;
    }

    if (hasAll && rule.subjects.size() > 1) {
        // Not an error, but worth a warning — we'll print it separately
        cerr << "  [Semantic Warning] Subject 'ALL' makes other subjects redundant.\n";
    }
}

bool SemanticAnalyser::analyse() {
    checkTimeRange();
    checkBetweenOrder();
    checkConditionValidity();
    checkSubjectConsistency();

    return errors.empty();
}

const vector<SemanticError>& SemanticAnalyser::getErrors() const {
    return errors;
}

void SemanticAnalyser::printErrors() const {
    for (const auto& e : errors) {
        cout << "  Semantic Error: " << e.message << endl;
        if (!e.suggestion.empty())
            cout << "  Suggestion:     " << e.suggestion << endl;
    }
}
