#ifndef NEOLANG_LEXER_H
#define NEOLANG_LEXER_H

#ifdef __cplusplus

#include <string>
#include <vector>
#include <cstdint>

using namespace std;

typedef enum {
    T_NUMBER,
    T_STRING,
    T_IDENTIFIER,
    T_KEYWORD,
    T_OPERATOR, // + - * / % ** & | ^ << >> ~ && || ?? > < <= >= == != ! @ #
    T_SET_OPERATOR, // += -= *= /= %= &= |= >>= <<= ~= &&= ||= ??= @= #=
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
    Token(TokenType type, string code, size_t start, size_t end, string value);

    Token(TokenType type, const string &code, size_t start, size_t end);

    TokenType type;
    string code;
    size_t start;
    size_t end;
    string value;
    Token *parent;
    vector<Token *> children;

    void updateValue();

    string toString() const;

    void throwError(const string &message) const;

    __attribute__((unused)) void dump() const;

    void free(bool freeChildren = true);
};

class Lexer {
public:
    explicit Lexer(string filePath, string code);

    vector<Token *> tokens;

    string filePath;
    string code;
    size_t index;
    Token *eof;

    char peek(size_t offset = 0);

    char current();

    char next(size_t offset = 1);

    void skip(size_t offset = 1);

    void tokenize();

    void groupTokens();

    string toString() const;

    void dump() const;

    void throwError(const string &message, size_t start, size_t end) const;

    void free();
};

#endif

#endif //NEOLANG_LEXER_H
