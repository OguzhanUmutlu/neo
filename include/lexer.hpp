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
    T_SYMBOL, // . , : ;
    T_PAREN, // ( ) [ ] { }, these tokens will be removed and replaced with Group tokens
    T_EOL, // \n
    T_EOE, // semicolon
    T_EOF,
    T_INTERNAL_IDENTIFIER,

    T_GROUP,
    T_RANGE
} TokenType;

#define IsAnyOperatorToken(t) (t->type == T_OPERATOR || t->type == T_INC_OPERATOR || t->type == T_SET_OPERATOR)

class Token {
public:
    Token(TokenType type, string filename, string code, size_t start, size_t end, string value)
            : type(type), filename(filename), code(std::move(code)), start(start), end(end), value(std::move(value)),
              parent(nullptr) {};

    Token(TokenType type, string filename, const string &code, size_t start, size_t end)
            : Token(type, filename, code, start, end, code.substr(start, end - start)) {};

    TokenType type;
    string filename;
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

    void showError(const string &message) const;
};

vector<vector<Token *>> splitTokens(vector<Token *> tokens, string delim, bool emptyError = false);

string tokensToString(const string &pre, vector<Token *> tokens);

string tokensListToString(const string &pre, vector<vector<Token *>> tokens);

class Lexer {
public:
    explicit Lexer(string code, string filename)
            : code(std::move(code)), filename(filename),
              eof(new Token(T_EOF, filename, code, code.size(), code.size(), "")), index(-1) {
    };

    Lexer(string code, string filename, vector<Token *> tokens)
            : Lexer(code, filename) {
        this->tokens = tokens;
    };

    string filename;
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

    void throwError(const string &message, size_t index) const;

    void freeTokens();

    void showError(const string &message, size_t index_) const;
};

#endif

#endif //NEOLANG_LEXER_H
