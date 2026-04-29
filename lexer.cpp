#include "lexer.h"
#include <sstream>
#include <algorithm>
#include <cctype>

string Lexer::toUpper(const string& s) {
    string result = s;
    transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

Lexer::Lexer(string in) : input(in) {}

vector<Token> Lexer::tokenize() {
    vector<Token> tokens;
    stringstream ss(input);
    string word;

    while (ss >> word) {
        while (!word.empty() && ispunct(word.back()) && word.back() != ':')
            word.pop_back();

        string up = toUpper(word);

        if (up == "BLOCK" || up == "ALLOW" || up == "MUTE" ||
            up == "SHUT"  || up == "SILENCE" || up == "ENABLE" ||
            up == "DISABLE") {
            tokens.push_back(Token(up, ACTION));
        }

        else if ((up == "MESSAGE" || up == "CALL" || up == "NOTIFY") &&
                 !tokens.empty() && tokens.back().type == NEGATION) {
            
            while (!tokens.empty() && tokens.back().type == NEGATION)
                tokens.pop_back();
            
            tokens.push_back(Token("BLOCK", ACTION));
            tokens.push_back(Token(up + "S", SUBJECT));
        }

        else if (up == "NOT" || up == "NO" || up == "DON'T" || up == "DO") {
            tokens.push_back(Token(up, NEGATION));
        }

        else if (up == "MESSAGE"  || up == "MESSAGES" ||
                 up == "CALL"     || up == "CALLS"    ||
                 up == "NOTIFICATION" || up == "NOTIFICATIONS" ||
                 up == "ALERT"    || up == "ALERTS"   ||
                 up == "ALL"      || up == "EVERYTHING") {
            tokens.push_back(Token(up, SUBJECT));
        }

        else if (up == "AFTER" || up == "BEFORE" ||
                 up == "BETWEEN" || up == "DURING") {
            tokens.push_back(Token(up, RELATION));
        }

        else if (up == "AM" || up == "PM") {
            tokens.push_back(Token(up, MERIDIEM));
        }

        else if (up == "MIDNIGHT" || up == "NOON" ||
                 up == "SUNRISE"  || up == "SUNSET") {
            tokens.push_back(Token(up, TIME_KEYWORD));
        }

        else if (up == "IF") {
            tokens.push_back(Token(up, IF_KW));
        }

        
        else if (up == "AND" || up == "OR") {
            tokens.push_back(Token(up, CONNECTOR));
        }

        else if (up == "DEVICE" || up == "PHONE" || up == "SCREEN" ||
                 up == "FOCUS"  || up == "MODE"  || up == "APP") {
            tokens.push_back(Token(up, COND_SUBJECT));
        }

        else if (up == "IS" || up == "ARE" || up == "HAS") {
            tokens.push_back(Token(up, COND_VERB));
        }

        else if (up == "ON" || up == "OFF" || up == "ACTIVE" ||
                 up == "ENABLED" || up == "DISABLED" || up == "LOCKED") {
            tokens.push_back(Token(up, COND_STATE));
        }

        else if (up == "DND" || up == "SILENT" || up == "VIBRATE" ||
                 up == "FOCUS_MODE" || up == "SLEEP"  || up == "DRIVING" ||
                 up == "WORK") {
            tokens.push_back(Token(up, COND_VALUE));
        }

        
        else if (!word.empty() && isdigit(word[0])) {
            tokens.push_back(Token(word, NUMBER));
        }

        else {
            tokens.push_back(Token(up, UNKNOWN));
        }
    }

    return tokens;
}

string Lexer::typeToString(TokenType t) {
    switch (t) {
        case ACTION:       return "ACTION";
        case NEGATION:     return "NEGATION";
        case SUBJECT:      return "SUBJECT";
        case RELATION:     return "RELATION";
        case NUMBER:       return "NUMBER";
        case MERIDIEM:     return "MERIDIEM";
        case TIME_KEYWORD: return "TIME_KEYWORD";
        case IF_KW:        return "IF";
        case CONNECTOR:    return "CONNECTOR";
        case COND_SUBJECT: return "COND_SUBJECT";
        case COND_VERB:    return "COND_VERB";
        case COND_STATE:   return "COND_STATE";
        case COND_VALUE:   return "COND_VALUE";
        default:           return "UNKNOWN";
    }
}
