#include <iostream>
#include "parser.h"

using namespace std;

Parser::Parser(Lexer *lexer) : lexer(lexer), index(-1) {
}

Token *Parser::peek(size_t offset) const {
    if (index + offset >= lexer->tokens.size()) {
        return lexer->eof;
    }
    return lexer->tokens[index + offset];
}

Token *Parser::next(size_t offset) {
    index += offset;
    return current();
}

Token *Parser::current() const {
    return peek(0);
}

__attribute__((unused)) void Parser::skip(size_t offset) {
    index += offset;
}

void Parser::parse() {
    while (true) {
        auto token = next();
        if (token == lexer->eof) {
            break;
        }

        //token->dump();
    }
}

string Parser::toString() {
    return "Parser";
}

__attribute__((unused)) void Parser::dump() {
    cout << toString() << endl;
}

void Parser::free() {
    delete this;
}