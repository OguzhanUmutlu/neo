#ifndef NEOLANG_PARSER_H
#define NEOLANG_PARSER_H

#ifdef __cplusplus

#include <string>
#include <vector>
#include <cstdint>
#include "lexer.h"

using namespace std;

class Statement {
};

class Parser {
public:
    explicit Parser(Lexer *lexer);

    vector<Statement> statements;

    Lexer *lexer;
    size_t index;

    Token *peek(size_t offset = 0) const;

    Token *current() const;

    Token *next(size_t offset = 1);

    __attribute__((unused)) void skip(size_t offset = 1);

    void parse();

    static string toString() ;

    __attribute__((unused)) static void dump() ;

    void free();
};

#endif

#endif //NEOLANG_PARSER_H
