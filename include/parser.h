#ifndef NEOLANG_PARSER_H
#define NEOLANG_PARSER_H

#ifdef __cplusplus

#include <string>
#include <utility>
#include <vector>
#include <cstdint>
#include <iostream>
#include <memory>
#include "lexer.h"

using namespace std;

typedef enum {
    S_VARIABLE_DECLARATION,
    S_FUNCTION_DECLARATION,
    S_DO,
    S_LOOP,
    S_WHILE,
    S_DO_WHILE,
    S_FOR_ITERATOR,
    S_FOR_CLASSIC,
    S_BREAK,
    S_CONTINUE,
    S_CLASS_DEFINITION,
    S_IF_FLOW,
    S_IMPORT,
    S_EXPRESSION
} StatementType;

class Statement {
public:
    Statement(StatementType type) : type(type) {};

    StatementType type;

    virtual string toString();

    __attribute__((unused)) void dump() { cout << toString() << endl; };
};

class VariableDeclarationStatement : public Statement {
public:
    VariableDeclarationStatement(vector<Token *> name, vector<Token *> value, bool constant)
            : name(std::move(name)), value(std::move(value)), constant(constant), Statement(S_VARIABLE_DECLARATION) {};
    vector<Token *> name;
    vector<Token *> value;
    bool constant;

    string toString() override;
};

class FunctionDeclarationStatement : public Statement {
public:
    FunctionDeclarationStatement(Token *name, vector<vector<Token *>> arguments, vector<unique_ptr<Statement>> body)
            : name(name), arguments(arguments), body(std::move(body)), Statement(S_FUNCTION_DECLARATION) {};

    Token *name;
    vector<vector<Token *>> arguments;
    vector<unique_ptr<Statement>> body;

    string toString() override;
};

class DoStatement : public Statement {
public:
    explicit DoStatement(vector<unique_ptr<Statement>> body)
            : body(std::move(body)), Statement(S_DO) {};

    vector<unique_ptr<Statement>> body;

    string toString() override;
};

class LoopStatement : public Statement {
public:
    explicit LoopStatement(vector<unique_ptr<Statement>> body)
            : body(std::move(body)), Statement(S_LOOP) {};

    vector<unique_ptr<Statement>> body;

    string toString() override;
};

class WhileStatement : public Statement {
public:
    explicit WhileStatement(vector<Token *> condition, vector<unique_ptr<Statement>> body)
            : condition(condition), body(std::move(body)), Statement(S_WHILE) {};

    vector<Token *> condition;
    vector<unique_ptr<Statement>> body;

    string toString() override;
};

class DoWhileStatement : public Statement {
public:
    explicit DoWhileStatement(vector<Token *> condition, vector<unique_ptr<Statement>> body)
            : condition(condition), body(std::move(body)), Statement(S_DO_WHILE) {};

    vector<Token *> condition;
    vector<unique_ptr<Statement>> body;

    string toString() override;
};

class ForIteratorStatement : public Statement {
public:
    explicit ForIteratorStatement(Token *index, Token *value, vector<Token *> iterator,
                                  vector<unique_ptr<Statement>> body)
            : index(index), value(value), iterator(iterator), body(std::move(body)), Statement(S_FOR_ITERATOR) {};

    Token *index;
    Token *value;
    vector<Token *> iterator;
    vector<unique_ptr<Statement>> body;

    string toString() override;
};

class ForClassicStatement : public Statement {
public:
    explicit ForClassicStatement(unique_ptr<Statement> init, vector<Token *> condition, unique_ptr<Statement> iterator,
                                 vector<unique_ptr<Statement>> body)
            : init(std::move(init)), condition(condition), iterator(std::move(iterator)), body(std::move(body)),
              Statement(S_FOR_CLASSIC) {};

    unique_ptr<Statement> init;
    vector<Token *> condition;
    unique_ptr<Statement> iterator;
    vector<unique_ptr<Statement>> body;

    string toString() override;
};

class BreakStatement : public Statement {
public:
    BreakStatement() : Statement(S_BREAK) {};

    string toString() override;
};

class ContinueStatement : public Statement {
public:
    ContinueStatement() : Statement(S_CONTINUE) {};

    string toString() override;
};

class ClassDefinitionStatement : public Statement {
public:
    explicit ClassDefinitionStatement(vector<unique_ptr<Statement>> attributes,
                                      vector<unique_ptr<Statement>> methods)
            : attributes(std::move(attributes)), methods(std::move(methods)),
              Statement(S_CLASS_DEFINITION) {};

    vector<unique_ptr<Statement>> attributes;
    vector<unique_ptr<Statement>> methods;

    string toString() override;
};

class IfFlowStatement : public Statement {
public:
    explicit IfFlowStatement(vector<Token *> condition, vector<unique_ptr<Statement>> body,
                             vector<unique_ptr<Statement>> elseBody)
            : condition(std::move(condition)), body(std::move(body)), elseBody(std::move(elseBody)),
              Statement(S_IF_FLOW) {};

    vector<Token *> condition;
    vector<unique_ptr<Statement>> body;
    vector<unique_ptr<Statement>> elseBody;

    string toString() override;
};

class ImportStatement : public Statement {
public:
    explicit ImportStatement(Token *name, vector<Token *> imports)
            : name(name), imports(std::move(imports)), Statement(S_IMPORT) {};

    Token *name;
    vector<Token *> imports;

    string toString() override;
};

class ExpressionStatement : public Statement {
public:
    explicit ExpressionStatement(vector<Token *> expression)
            : expression(std::move(expression)), Statement(S_EXPRESSION) {};

    vector<Token *> expression;

    string toString() override;
};

class Parser {
public:
    explicit Parser(Lexer lexer) : lexer(std::move(lexer)), index(-1) {};

    vector<unique_ptr<Statement>> statements;
    vector<Token *> accumulator;

    Lexer lexer;
    size_t index;

    Token *peek(size_t offset = 0) const;

    Token *current() const;

    Token *next(size_t offset = 1);

    Token *accumulate(size_t offset = 1);

    void parseVariableDeclarationStatement();

    void parseFunctionDeclarationStatement();

    void parseDoStatement();

    void parseLoopStatement();

    void parseForLoopStatement();

    void parseWhileLoopStatement();

    void parseClassDefinitionStatement();

    void parseIfFlowStatement();

    void parseElseFlowStatement();

    void parseImportStatement();

    void parse();

    string toString();

    __attribute__((unused)) void dump();
};

#endif

#endif //NEOLANG_PARSER_H
