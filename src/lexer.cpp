#include "lexer.h"
#include "error.h"
#include <string>
#include <utility>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <regex>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtrigraphs"
using namespace std;

// operators: + - * / % ** & | ^ << >> ~ && || ?? > < <= >= == != ! @ #

#define isdigit(x) (x >= '0' && x <= '9')

unordered_set<char> whitespace = {' ', '\t', '\r', '\v'};
unordered_set<char> symbols = {'.', ',', ':', ';', '='};
unordered_set<char> paren = {'(', ')', '{', '}', '[', ']'};
unordered_set<char> singleOperators = {'+', '-', '*', '/', '%', '&', '|', '^', '~', '>', '<', '!', '@', '#'};
unordered_set<string> doubleOperators = {"**", "<<", ">>", "&&", "||", "??", "<=", ">=", "==", "!="};
unordered_set<string> doubleSetOperators = {"+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "~=", "++", "--"};
unordered_set<string> tripleSetOperators = {"<<=", ">>=", "&&=", "||=", "??=", "**="};
unordered_set<string> keywords = {"let", "const", "if", "for", "loop", "while", "return", "break", "continue", "fn",
                                  "class", "import", "in"};
unordered_map<TokenType, string> tokenTypeToString = {
        {T_NUMBER,       "number"},
        {T_STRING,       "string"},
        {T_IDENTIFIER,   "identifier"},
        {T_KEYWORD,      "keyword"},
        {T_OPERATOR,     "operator"},
        {T_SET_OPERATOR, "set operator"},
        {T_SYMBOL,       "symbol"},
        {T_PAREN,        "parenthesis"},
        {T_EOL,          "eol"},
        {T_EOE,          "eoe"},
        {T_EOF,          "eof"},
        {T_GROUP,        "group"},
        {T_RANGE,        "range"}
};
unordered_map<string, string> parenMap = {
        {"(", ")"},
        {"{", "}"},
        {"[", "]"}
};


Token::Token(TokenType type, string code, size_t start, size_t end, string value)
        : type(type), code(std::move(code)), start(start), end(end), value(std::move(value)), parent(nullptr) {
}

Token::Token(TokenType type, const string &code, size_t start, size_t end)
        : Token(type, code, start, end, code.substr(start, end - start)) {
}

string tokensToString(const string &pre, vector<Token *> tokens) { // NOLINT(*-no-recursion)
    string childrenStr;
    string nl = "\n" + pre;
    for (auto &token: tokens) {
        childrenStr += pre + regex_replace(token->toString(), regex("\n"), nl) +
                       (token != tokens.back() ? ",\n" : "");
    }
    return childrenStr;
}

string Token::toString() const { // NOLINT(*-no-recursion)
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
    ::throwError(message, code, start, end);
}

__attribute__((unused)) void Token::dump() const {
    cout << toString() << endl;
}

void Token::updateValue() {
    this->value = this->code.substr(this->start, this->end - this->start);
}

void Token::free(bool freeChildren) { // NOLINT(*-no-recursion)
    if (freeChildren) {
        for (auto token: children) {
            token->free(freeChildren);
        }
    }
    delete this;
}

Lexer::Lexer(string filePath, string code)
        : filePath(std::move(filePath)), code(std::move(code)),
          eof(new Token(TokenType::T_EOF, code, code.size(), code.size(), "")), index(-1) {
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

void Lexer::skip(size_t offset) {
    index += offset;
}

string Lexer::toString() const {
    return "{\n  'tokens': [\n" + tokensToString("    ", tokens) + "\n  ]\n}";
}

void Lexer::dump() const {
    cout << toString() << endl;
}

void Lexer::throwError(const string &message, size_t start, size_t end) const {
    ::throwError(message, code, start, end);
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
            tokens.push_back(new Token(TokenType::T_EOL, code, si, si + 1, chrStr));
            continue;
        }
        if (chr == ';') {
            tokens.push_back(new Token(TokenType::T_EOE, code, si, si + 1, chrStr));
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
            skip(-1);
            continue;
        }
        if (chr == '/' && chr1 == '*') {
            while (true) {
                auto chr2 = next();
                if (chr2 == '\0') {
                    break; // unterminated comment, but it doesn't matter, I guess, maybe throw an error
                }
                if (chr2 == '*' && peek(1) == '/') {
                    skip(1);
                    break;
                }
            }
            continue;
        }
        if (singleOperators.find(chr) != singleOperators.end()) {
            tokens.push_back(new Token(TokenType::T_OPERATOR, code, si, si + 1, chrStr));
            continue;
        }
        if (paren.find(chr) != paren.end()) {
            tokens.push_back(new Token(TokenType::T_PAREN, code, si - 1, si, chrStr));
            continue;
        }
        if (doubleOperators.find(chr1str) != doubleOperators.end()) {
            tokens.push_back(new Token(TokenType::T_OPERATOR, code, si, si + 1, chr1str));
            index += 1;
            continue;
        }
        if (doubleSetOperators.find(chr1str) != doubleSetOperators.end()) {
            tokens.push_back(new Token(TokenType::T_SET_OPERATOR, code, si, si + 1, chr1str));
            index += 1;
            continue;
        }
        auto chr2 = peek(2); // double next token
        auto chr2str = chrStr + chr1 + chr2;
        if (tripleSetOperators.find(chr2str) != doubleSetOperators.end()) {
            tokens.push_back(new Token(TokenType::T_SET_OPERATOR, code, si, si + 1, chr2str));
            index += 2;
            continue;
        }

        if (isdigit(chr) || (chr == '.' && isdigit(chr1))) {
            bool is_float = chr == '.';
            while ((chr = next()) != '\0' && (isdigit(chr) || (!is_float && chr == '.')) || chr == 'n') {
                if (chr == '.') {
                    is_float = true;
                }
                if (chr == 'n') {
                    skip(1);
                    break;
                }
            }
            if (code[si] == '.' && !tokens.empty() && tokens.back()->type == TokenType::T_NUMBER &&
                tokens.back()->value.back() == '.') {
                auto backStart = tokens.back()->start;
                tokens.back()->free();
                tokens.pop_back();
                tokens.push_back(new Token(TokenType::T_RANGE, code, backStart, index));
                skip(-1);
                continue;
            }
            tokens.push_back(new Token(TokenType::T_NUMBER, code, si, index));
            skip(-1);
            continue;
        }
        if (symbols.find(chr) != symbols.end()) {
            tokens.push_back(new Token(TokenType::T_SYMBOL, code, si, si + 1, chrStr));
            continue;
        }
        if (chr == '"' or chr == '\'') {
            char startChar = chr;
            string res = string(1, chr);
            bool backslash = false;
            while ((chr = next()) != '\0' && (chr != startChar || backslash)) {
                if (chr == '\\') backslash = !backslash;
                else backslash = false;
                res += chr;
            }
            if (chr == '\0') {
                throwError("Unterminated string", si, si + 1);
            }
            res += chr;
            tokens.push_back(new Token(TokenType::T_STRING, code, si, index, res));
            continue;
        }

        if (isalpha(chr) || chr == '_') {
            string res = string(1, chr);
            while ((chr = next()) != '\0' && (isalnum(chr) || chr == '_')) {
                res += chr;
            }
            skip(-1);
            if (keywords.find(res) != keywords.end()) {
                tokens.push_back(new Token(TokenType::T_KEYWORD, code, si, index, res));
                continue;
            }
            tokens.push_back(new Token(TokenType::T_IDENTIFIER, code, si, index, res));
            continue;
        }

        throwError("SyntaxError: Unexpected character", index, index + 1);
    }
}

void Lexer::groupTokens() {
    auto program = new Token(TokenType::T_GROUP, code, 0, code.size());
    auto parent = program;
    for (auto token: tokens) {
        if (token->type == TokenType::T_PAREN && (token->value == "(" || token->value == "[" || token->value == "{")) {
            auto group = new Token(TokenType::T_GROUP, code, token->start, token->end, token->value);
            token->free();
            group->parent = parent;
            parent->children.push_back(group);
            parent = group;
        } else if (token->type == TokenType::T_PAREN &&
                   (token->value == ")" || token->value == "]" || token->value == "}")) {
            if (token->value != parenMap.find(parent->value)->second) {
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

void Lexer::free() {
    for (auto &token: tokens) {
        token->free();
    }
    tokens.clear();
    eof->free();
    delete this;
}
#pragma clang diagnostic pop