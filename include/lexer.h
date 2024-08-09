#ifndef NEOLANG_LEXER_H
#define NEOLANG_LEXER_H

#ifdef __cplusplus

#include <string>
#include <utility>
#include <vector>
#include <cstdint>

using namespace std;

typedef enum {
    T_NUMBER,
    T_STRING,
    T_IDENTIFIER,
    T_KEYWORD,
    T_OPERATOR, // + - * / % ** & | ^ << >> ~ && || > < <= >= == != !
    T_SET_OPERATOR, // += -= *= /= %= &= |= >>= <<= ~= &&= ||=
    T_INC_OPERATOR, // ++ --
    T_SYMBOL, // . , : ; =
    T_PAREN, // ( ) [ ] { }, these tokens will be removed and replaced with Group tokens
    T_EOL, // \n
    T_EOE, // semicolon
    T_EOF,

    T_GROUP,
    T_RANGE
} TokenType;

class Token {
public:
    Token(TokenType type, string code, size_t start, size_t end, string value)
            : type(type), code(std::move(code)), start(start), end(end), value(std::move(value)), parent(nullptr) {};

    Token(TokenType type, const string &code, size_t start, size_t end)
            : Token(type, code, start, end, code.substr(start, end - start)) {};

    TokenType type;
    string code;
    size_t start;
    size_t end;
    string value;
    Token *parent;
    vector<Token *> children;

    void updateValue();

    string toString();

    void throwError(const string &message) const;

    __attribute__((unused)) void dump();

    void free(bool freeChildren = true);
};

vector<vector<Token *>> splitTokens(vector<Token *> tokens, string delim);

string tokensToString(const string &pre, vector<Token *> tokens);

string tokensListToString(const string &pre, vector<vector<Token *>> tokens);

class Lexer {
public:
    explicit Lexer(string code)
            : code(std::move(code)),
              eof(new Token(TokenType::T_EOF, code, code.size(), code.size(), "")), index(-1) {
    };

    Lexer(string code, vector<Token *> tokens)
            : Lexer(std::move(code)) {
        this->tokens = std::move(tokens);
    };

    vector<Token *> tokens;

    string code;
    size_t index;
    Token *eof;

    char peek(size_t offset = 0);

    char current();

    char next(size_t offset = 1);

    void tokenize();

    void groupTokens();

    string toString() const;

    __attribute__((unused)) void dump() const;

    void throwError(const string &message, size_t start, size_t end) const;

    void freeTokens();
};

#endif

#endif //NEOLANG_LEXER_H
