#ifndef NEO_COMPILER_HPP
#define NEO_COMPILER_HPP

#include "lexer.hpp"
#include "parser.hpp"
#include <sstream>
#include <unordered_map>

using namespace std;

class Compiler;

class VariableDefinition {
public:
    VariableDefinition() {};

    VariableDefinition(string pointer, bool constant, bool isFunction)
            : pointer(pointer), constant(constant), isFunction(isFunction) {};

    string pointer;
    bool constant;
    bool isFunction;
};

typedef enum {
    CTV_TEMP,
    CTV_VARIABLE,
    CTV_INVALID_VARIABLE,
    CTV_NULL
} CTV_Type;

typedef struct {
    CTV_Type type;
    string pointer;
} CompileTimeValue;

class Scope {
public:
    Scope(int id, string &fnCode, Scope *parent, bool isLoop) : id(id), fnCode(fnCode), parent(parent), isLoop(isLoop) {};

    int id;
    string &fnCode;
    string indentStr = "\t";
    Scope *parent = nullptr;
    unordered_map<string, VariableDefinition> variables;
    vector<string> temp; // stores the temp variables' names, should be cleared and dereferenced after use
    CompileTimeValue returning;
    bool isLoop;

    void append(string code, bool indent = true);

    void clearTemp();

    VariableDefinition *getVariableDefinition(string name);

    void clearVariables();
};

typedef struct {
    Scope *scope;
    Token *errorToken;
    string functionName;
    vector<size_t> scopePoint;
} MissingFunctionDefinition;

class Compiler {
public:
    Compiler(Parser &parser) : parser(parser) {};

    unordered_map<string, string> functions;
    string globalCode;
    vector<string> functionList;
    size_t _id = 0;
    Parser &parser;
    vector<MissingFunctionDefinition> missingFunctionDefinitions;

    void compile();

    void compileScope(Scope *scope, vector<unique_ptr<Statement>> *statements);

    CompileTimeValue executeSingleExpression(Scope *scope, vector<Token *> tokens);

    CompileTimeValue executeExpression(Scope *scope, vector<Token *> tokens);

    CompileTimeValue computeBinaryOperation(Scope *scope, vector<Token *> a, Token *op, vector<Token *> b);

    CompileTimeValue executeSeparatedExpression(Scope *scope, vector<vector<Token *>> sep);

    CompileTimeValue executeToken(Scope *scope, Token *t0);

    void introduceFunction(Scope *scope, string name, vector<unique_ptr<Statement>> *statements, bool isLambda);
};

#endif //NEO_COMPILER_HPP
