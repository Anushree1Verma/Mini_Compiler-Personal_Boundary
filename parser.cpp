#include "parser.h"
#include <sstream>
#include <iomanip>


Parser::Parser(vector<Token> t) : tokens(t), current(0) {}

Token Parser::peek() {
    if (current < (int)tokens.size())
        return tokens[current];
    return Token("EOF", UNKNOWN);
}

Token Parser::advance() {
    if (current < (int)tokens.size())
        return tokens[current++];
    return Token("EOF", UNKNOWN);
}

bool Parser::check(TokenType t) {
    return peek().type == t;
}

bool Parser::match(TokenType t) {
    if (check(t)) { advance(); return true; }
    return false;
}

void Parser::error(string msg) {
    cout << "\nSyntax Error: " << msg << endl;
    if (current < (int)tokens.size())
        cout << "  at token: \"" << tokens[current].value << "\"" << endl;
    exit(1);
}

void Parser::parseAction(PolicyRule& rule) {
    rule.negated = false;

    if (check(NEGATION) && peek().value == "DO") {
        advance(); 
        if (check(NEGATION) && peek().value == "NOT") {
            advance(); 
            rule.negated = true;
        }
    }
    
    else if (check(NEGATION)) {
        advance();
        rule.negated = true;
    }

    if (!check(ACTION)) {
        error("Expected an action word (BLOCK, ALLOW, MUTE, SHUT, SILENCE)");
    }

    Token act = advance();
    rule.rawAction = act.value;

    if (act.value == "ALLOW" || act.value == "ENABLE") {
        rule.action = "ALLOW";
    } else {
       
        rule.action = "BLOCK";
    }

    
    if (rule.negated) {
        rule.action = (rule.action == "BLOCK") ? "ALLOW" : "BLOCK";
    }

    if (check(NEGATION)) {
        advance();
        rule.negated = !rule.negated;
        rule.action = (rule.action == "BLOCK") ? "ALLOW" : "BLOCK";
    }
}

void Parser::parseSubject(PolicyRule& rule) {
    if (!check(SUBJECT)) {
        error("Expected a subject (MESSAGES, CALLS, NOTIFICATIONS, ALL)");
    }

    while (check(SUBJECT)) {
        rule.subjects.push_back(advance().value);
        // Skip "AND" between subjects if present
        if (check(CONNECTOR) && peek().value == "AND") {
            advance();
        }
    }
}

TimeExpr Parser::parseTimeExpr() {
    if (check(TIME_KEYWORD)) {
        string kw = advance().value;
        if (kw == "MIDNIGHT") return TimeExpr(0, 0, "midnight");
        if (kw == "NOON")     return TimeExpr(12, 0, "noon");
        if (kw == "SUNRISE")  return TimeExpr(6, 0, "sunrise");
        if (kw == "SUNSET")   return TimeExpr(18, 0, "sunset");
    }

    if (!check(NUMBER)) {
        error("Expected a time (e.g. 10 PM, 9:30 AM, midnight)");
    }

    string numStr = advance().value;

    int hour = 0, minute = 0;
    size_t colon = numStr.find(':');
    if (colon != string::npos) {
        hour   = stoi(numStr.substr(0, colon));
        minute = stoi(numStr.substr(colon + 1));
    } else {
        hour = stoi(numStr);
    }

    string raw = numStr;

    if (check(MERIDIEM)) {
        string meridiem = advance().value;
        raw += " " + meridiem;

        if (meridiem == "PM" && hour != 12) hour += 12;
        if (meridiem == "AM" && hour == 12) hour = 0;
    }

    if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
        error("Time out of range: " + raw);
    }

    return TimeExpr(hour, minute, raw);
}

TimeClause Parser::parseTimeClause() {
    TimeClause tc;

    if (!check(RELATION)) {
        error("Expected a time relation (AFTER, BEFORE, BETWEEN, DURING)");
    }

    tc.relation = advance().value;
    tc.timeA = parseTimeExpr();

    if (tc.relation == "BETWEEN") {
        if (check(CONNECTOR) && peek().value == "AND") {
            advance(); 
        }
        tc.timeB = parseTimeExpr();
        tc.hasTwoTimes = true;
    }

    return tc;
}
vector<ConditionExpr> Parser::parseCondition() {
    vector<ConditionExpr> conditions;

    if (!check(IF_KW)) {
        error("Expected IF to start a condition");
    }
    advance(); 
    do {
        ConditionExpr ce;

        while (check(COND_SUBJECT)) {
            ce.subject += advance().value + " ";
        }
        if (!ce.subject.empty() && ce.subject.back() == ' ')
            ce.subject.pop_back();

        if (check(COND_VERB)) {
            ce.stateVerb = advance().value;
        }

        if (check(COND_STATE)) {
            ce.modifier = advance().value;
        }

        if (check(COND_VALUE)) {
            ce.value = advance().value;
        }

        if (ce.subject.empty() && ce.stateVerb.empty() &&
            ce.modifier.empty() && ce.value.empty()) {
            error("Empty condition expression after IF");
        }

        conditions.push_back(ce);
        if (check(CONNECTOR)) {
            ce.connector = peek().value;
            advance(); 
        } else {
            break;
        }
    } while (true);

    return conditions;
}


PolicyRule Parser::parseRule() {
    PolicyRule rule;

    parseAction(rule);
    parseSubject(rule);
    rule.timeClause = parseTimeClause();

    if (check(IF_KW)) {
        rule.hasCondition = true;
        rule.conditions = parseCondition();
    }

    if (current < (int)tokens.size() && peek().type != UNKNOWN) {
    
        cerr << "Warning: unexpected tokens at end of input starting at: \""
             << peek().value << "\"" << endl;
    }

    return rule;
}
void PolicyRule::print() const {
    cout << "  PolicyRule" << endl;
    cout << "  ├─ Action:   " << action
         << " (raw: " << rawAction << ", negated: " << (negated ? "yes" : "no") << ")" << endl;

    cout << "  ├─ Subject:  ";
    for (size_t i = 0; i < subjects.size(); ++i) {
        cout << subjects[i];
        if (i + 1 < subjects.size()) cout << ", ";
    }
    cout << endl;

    cout << "  ├─ Time:     " << timeClause.relation
         << " " << timeClause.timeA.raw;
    if (timeClause.hasTwoTimes)
        cout << " AND " << timeClause.timeB.raw;
    cout << endl;

    auto fmt = [](int h, int m) -> string {
        ostringstream os;
        os << setfill('0') << setw(2) << h << ":" << setw(2) << m;
        return os.str();
    };
    cout << "  │   └─ (24h): " << timeClause.relation
         << " " << fmt(timeClause.timeA.hour, timeClause.timeA.minute);
    if (timeClause.hasTwoTimes)
        cout << " – " << fmt(timeClause.timeB.hour, timeClause.timeB.minute);
    cout << endl;

    if (hasCondition) {
        cout << "  └─ Condition:" << endl;
        for (const auto& c : conditions) {
            cout << "      IF";
            if (!c.subject.empty())  cout << " " << c.subject;
            if (!c.stateVerb.empty()) cout << " " << c.stateVerb;
            if (!c.modifier.empty()) cout << " " << c.modifier;
            if (!c.value.empty())    cout << " " << c.value;
            cout << endl;
        }
    } else {
        cout << "  └─ Condition: (none)" << endl;
    }
}

string PolicyRule::toJSON() const {
    ostringstream js;
    js << "{\n";
    js << "  \"action\": \"" << action << "\",\n";

    js << "  \"subjects\": [";
    for (size_t i = 0; i < subjects.size(); ++i) {
        js << "\"" << subjects[i] << "\"";
        if (i + 1 < subjects.size()) js << ", ";
    }
    js << "],\n";

    js << "  \"timeClause\": {\n";
    js << "    \"relation\": \"" << timeClause.relation << "\",\n";
    js << "    \"timeA\": { \"hour\": " << timeClause.timeA.hour
       << ", \"minute\": " << timeClause.timeA.minute
       << ", \"raw\": \"" << timeClause.timeA.raw << "\" }";

    if (timeClause.hasTwoTimes) {
        js << ",\n    \"timeB\": { \"hour\": " << timeClause.timeB.hour
           << ", \"minute\": " << timeClause.timeB.minute
           << ", \"raw\": \"" << timeClause.timeB.raw << "\" }";
    }
    js << "\n  }";

    if (hasCondition) {
        js << ",\n  \"conditions\": [\n";
        for (size_t i = 0; i < conditions.size(); ++i) {
            const auto& c = conditions[i];
            vector<string> fields;
            if (!c.subject.empty())   fields.push_back("\"subject\": \"" + c.subject + "\"");
            if (!c.stateVerb.empty()) fields.push_back("\"verb\": \"" + c.stateVerb + "\"");
            if (!c.modifier.empty())  fields.push_back("\"state\": \"" + c.modifier + "\"");
            if (!c.value.empty())     fields.push_back("\"value\": \"" + c.value + "\"");
            js << "    { ";
            for (size_t k = 0; k < fields.size(); ++k) {
                js << fields[k];
                if (k + 1 < fields.size()) js << ", ";
            }
            js << " }";
            if (i + 1 < conditions.size()) js << ",";
            js << "\n";
        }
        js << "  ]";
    }

    js << "\n}";
    return js.str();
}
