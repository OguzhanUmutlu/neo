#include "lexer.hpp"
#include "error.hpp"
#include <string>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <regex>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtrigraphs"
using namespace std;

// operators: + - * / % ** & | ^ << >> ~ && || > < <= >= == != !

unordered_set<char> whitespace = {' ', '\t', '\r', '\v'};
unordered_set<char> symbols = {'.', ',', ':', ';', '\\'};
unordered_set<char> paren = {'(', ')', '{', '}', '[', ']'};
unordered_set<char> singleOperators = {'+', '-', '*', '/', '%', '&', '|', '^', '~', '>', '<', '!', '='};
unordered_set<string> doubleOperators = {"**", "<<", ">>", "&&", "||", "<=", ">=", "==", "!="};
unordered_set<string> doubleSetOperators = {"+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "~=", ":="};
unordered_set<string> tripleSetOperators = {"<<=", ">>=", "&&=", "||=", "**="};
unordered_set<string> keywords = {"let", "const", "if", "for", "loop", "while", "return", "break", "continue", "fn",
                                  "class", "import", "in", "switch", "match", "case", "default", "throw"};
unordered_map<TokenType, string> tokenTypeToString = {
        {T_NUMBER,              "number"},
        {T_STRING,              "string"},
        {T_IDENTIFIER,          "identifier"},
        {T_KEYWORD,             "keyword"},
        {T_OPERATOR,            "operator"},
        {T_INC_OPERATOR,        "inc-dec operator"},
        {T_SET_OPERATOR,        "set operator"},
        {T_SYMBOL,              "symbol"},
        {T_PAREN,               "parenthesis"},
        {T_EOL,                 "eol"},
        {T_EOE,                 "eoe"},
        {T_EOF,                 "eof"},
        {T_GROUP,               "group"},
        {T_RANGE,               "range"},
        {T_INTERNAL_IDENTIFIER, "internal identifier"}
};
unordered_map<string, string> parenMap = {
        {"(", ")"},
        {"{", "}"},
        {"[", "]"}
};

vector<vector<Token *>> splitTokens(vector<Token *> tokens, string delim, bool emptyError) {
    vector<vector<Token *>> result;
    vector<Token *> current;
    for (auto token: tokens) {
        if (token->value == delim) {
            if (emptyError && current.empty()) {
                token->throwError("SyntaxError: Unexpected token '" + token->value + "'");
            }
            result.push_back(current);
            current.clear();
        } else {
            current.push_back(token);
        }
    }
    if (!current.empty()) {
        result.push_back(current);
    }
    return result;
}

string tokensToString(const string &pre, vector<Token *> tokens) {
    string childrenStr;
    string nl = "\n" + pre;
    for (auto &token: tokens) {
        childrenStr += pre + regex_replace(token->toString(), regex("\n"), nl) +
                       (token != tokens.back() ? ",\n" : "");
    }
    return childrenStr;
}

string tokensListToString(const string &pre, vector<vector<Token *>> tokens) {
    string childrenStr;
    string nl = "\n" + pre;
    for (auto &token: tokens) {
        childrenStr += pre + tokensToString(pre, token) + (token != tokens.back() ? ",\n" : "");
    }
    return childrenStr;
}

string Token::toString() {
    if (type == T_GROUP) {
        return "{'type': '" + tokenTypeToString.find(T_GROUP)->second + "', 'children': [\n" +
               tokensToString("    ", children) + "\n  ]\n}";
    }
    auto val = value;
    if (val == "\n") {
        val = "\\n";
    }
    return "{'type': '" + tokenTypeToString.find(type)->second + "', 'value': '" + val + "'}";
}

void Token::throwError(const std::string &message) const {
    ::throwError(message, filename, code, start);
}

void Token::showError(const std::string &message) const {
    ::showError(message, filename, code, start);
}

__attribute__((unused)) void Token::dump() {
    cout << toString() << endl;
}

void Token::updateValue() {
    this->value = this->code.substr(this->start, this->end - this->start);
}

void Token::free(bool freeChildren) {
    if (freeChildren) {
        for (auto token: children) {
            token->free(freeChildren);
        }
    }
    delete this;
}

char Lexer::peek(size_t offset) {
    if (index + offset >= code.size()) {
        return '\0';
    }
    return code[index + offset];
}

char Lexer::next(size_t offset) {
    index += offset;
    return current();
}

char Lexer::current() {
    return peek(0);
}

string Lexer::toString() const {
    return "{\n  'tokens': [\n" + tokensToString("    ", tokens) + "\n  ]\n}";
}

__attribute__((unused)) void Lexer::dump() const {
    cout << toString() << endl;
}

void Lexer::throwError(const string &message, size_t index_) const {
    ::throwError(message, filename, code, index_);
}

void Lexer::showError(const string &message, size_t index_) const {
    ::showError(message, filename, code, index_);
}

void Lexer::tokenize() {
    code.size();
    while (true) {
        auto chr = next();
        auto chrStr = string(1, chr);
        auto si = index;

        if (chr == '\0') {
            break;
        }
        if (whitespace.find(chr) != whitespace.end()) {
            continue;
        }
        if (chr == '\n') {
            tokens.push_back(new Token(T_EOL, filename, code, si, si + 1, chrStr));
            continue;
        }
        if (chr == ';') {
            tokens.push_back(new Token(T_EOE, filename, code, si, si + 1, chrStr));
            continue;
        }
        auto chr1 = peek(1); // next token
        auto chr1str = chrStr + chr1;
        if (chr == '/' && chr1 == '/') {
            while (true) {
                auto chr2 = next();
                if (chr2 == '\n' || chr2 == '\0') {
                    break;
                }
            }
            --index;
            continue;
        }
        if (chr == '/' && chr1 == '*') {
            while (true) {
                auto chr2 = next();
                if (chr2 == '\0') {
                    break; // unterminated comment, but it doesn't matter, I guess, maybe throw an error
                }
                if (chr2 == '*' && peek(1) == '/') {
                    ++index;
                    break;
                }
            }
            continue;
        }
        if ((chr == '+' || chr == '-') && chr1 == chr) {
            tokens.push_back(new Token(T_INC_OPERATOR, filename, code, si, si + 2, chr1str));
            ++index;
            continue;
        }

        if (paren.find(chr) != paren.end()) {
            tokens.push_back(new Token(T_PAREN, filename, code, si, si + 1, chrStr));
            continue;
        }

        auto chr2 = peek(2); // double next token
        auto chr2str = chrStr + chr1 + chr2;
        if (tripleSetOperators.find(chr2str) != doubleSetOperators.end()) {
            tokens.push_back(new Token(T_SET_OPERATOR, filename, code, si, si + 1, chr2str));
            index += 2;
            continue;
        }

        if (doubleOperators.find(chr1str) != doubleOperators.end()) {
            tokens.push_back(new Token(T_OPERATOR, filename, code, si, si + 1, chr1str));
            ++index;
            continue;
        }

        if (doubleSetOperators.find(chr1str) != doubleSetOperators.end()) {
            tokens.push_back(new Token(T_SET_OPERATOR, filename, code, si, si + 1, chr1str));
            ++index;
            continue;
        }

        if (singleOperators.find(chr) != singleOperators.end()) {
            tokens.push_back(new Token(T_OPERATOR, filename, code, si, si + 1, chrStr));
            continue;
        }

        if (isdigit(chr) || (chr == '.' && isdigit(chr1))) {
            bool is_float = chr == '.';
            while ((chr = next()) != '\0' && (isdigit(chr) || (!is_float && chr == '.' && peek(1) != '.'))) {
                if (chr == '.') {
                    is_float = true;
                }
            }
            if (chr == 'e') {
                ++index;
                chr = peek(1);
                if (chr == '+' || chr == '-') {
                    ++index;
                }
                if (!isdigit(peek(1))) {
                    throwError("SyntaxError: Expected an integer", index);
                }
                while ((chr = next()) != '\0' && isdigit(chr)) {
                }
            }
            if (chr == 'n') {
                ++index;
            }
            tokens.push_back(new Token(T_NUMBER, filename, code, si, index));
            --index;
            continue;
        }
        if (symbols.find(chr) != symbols.end()) {
            tokens.push_back(new Token(T_SYMBOL, filename, code, si, si + 1, chrStr));
            continue;
        }
        if (chr == '"' or chr == '\'') {
            char startChar = chr;
            string res = string(1, chr);
            bool backslash = false;
            while ((chr = next()) != '\0' && (chr != startChar || backslash)) {
                if (chr == '\\') backslash = !backslash;
                else backslash = false;
                if (chr == '\r') continue;
                if (chr == '\n') {
                    res += '\\';
                    res += 'n';
                } else {
                    res += chr;
                }
            }
            if (chr == '\0') {
                throwError("SyntaxError: Unterminated string", si);
            }
            res += chr;
            tokens.push_back(new Token(T_STRING, filename, code, si, index, res));
            continue;
        }

        if (isalpha(chr) || chr == '_') {
            string res = string(1, chr);
            while ((chr = next()) != '\0' && (isalnum(chr) || chr == '_')) {
                res += chr;
            }
            --index;
            if (keywords.find(res) != keywords.end()) {
                tokens.push_back(new Token(T_KEYWORD, filename, code, si, index, res));
                continue;
            }
            tokens.push_back(new Token(T_IDENTIFIER, filename, code, si, index, res));
            continue;
        }

        throwError("SyntaxError: Unexpected character", index);
    }
}

void Lexer::groupTokens() {
    auto program = new Token(T_GROUP, filename, code, 0, code.size());
    auto parent = program;
    for (auto token: tokens) {
        if (token->type == T_PAREN && (token->value == "(" || token->value == "[" || token->value == "{")) {
            auto group = new Token(T_GROUP, filename, code, token->start, token->end, token->value);
            token->free();
            group->parent = parent;
            parent->children.push_back(group);
            parent = group;
        } else if (token->type == T_PAREN &&
                   (token->value == ")" || token->value == "]" || token->value == "}")) {
            if (parent == program || token->value != parenMap.find(parent->value)->second) {
                token->throwError("SyntaxError: Unexpected token '" + token->value + "'");
            }
            parent->end = token->end;
            parent->updateValue();
            token->free();
            parent = parent->parent;
        } else {
            parent->children.push_back(token);
        }
    }
    if (parent != program) {
        parent->throwError("SyntaxError: Unterminated parenthesis");
    }
    tokens.clear();
    tokens = program->children;
    program->free(false);
}

void Lexer::freeTokens() {
    for (auto &token: tokens) {
        token->free();
    }
    tokens.clear();
    eof->free();
}

#pragma clang diagnostic pop