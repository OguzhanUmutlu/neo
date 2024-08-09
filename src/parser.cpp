#include <iostream>
#include <regex>
#include "parser.h"

using namespace std;

Token *Parser::peek(size_t offset) const {
    if (index + offset >= lexer.tokens.size()) {
        return lexer.eof;
    }
    return lexer.tokens[index + offset];
}

Token *Parser::next(size_t offset) {
    index += offset;
    return current();
}

Token *Parser::current() const {
    return peek(0);
}

Token *Parser::accumulate(size_t offset) {
    auto nx = next(offset);
    accumulator.push_back(nx);
    return nx;
}

void Parser::parseVariableDeclarationStatement() {
    auto constant = current()->value == "const";
    Token *t;

    accumulator.clear();
    while ((t = accumulate()) != lexer.eof && t->value != "=" && t->value != ":") {}
    accumulator.pop_back();
    auto name = accumulator;

    if (t->value == ":") {
        // ignoring type
        while ((t = next()) != lexer.eof && t->value != "=") {}
    }

    if (t->value != "=") {
        t->throwError("Expected an equals sign for the variable declaration.");
    }

    accumulator.clear();
    while ((t = accumulate()) != lexer.eof && t->type != T_EOL && t->type != T_EOE) {}
    accumulator.pop_back();
    auto value = accumulator;

    statements.push_back(make_unique<VariableDeclarationStatement>(name, value, constant));
}

void Parser::parseFunctionDeclarationStatement() {
    // fn <identifier> <group ()> <group {}>
    // fn <identifier> <group ()> : <tokens...> <group {}>

    auto name = next();
    if (name->type != T_IDENTIFIER) name->throwError("Expected an identifier for the function name.");
    auto args = next();
    if (args->value[0] != '(') args->throwError("Expected an open parenthesis for the function arguments.");
    if (peek(1)->value == ":") {
        while (next()->value[0] != '{') {}
        --index;
    }
    auto body = next();

    auto ps = Parser(Lexer(lexer.code, body->children));
    ps.parse();

    statements.push_back(make_unique<FunctionDeclarationStatement>(
            name, splitTokens(args->children, ","), std::move(ps.statements)));
}

void Parser::parseDoStatement() {
    auto body = next();

    auto ps = Parser(Lexer(lexer.code, body->children));
    ps.parse();

    if (peek(1)->value == "while") {
        auto condition = next();
        if (condition->value[0] != '(') condition->throwError("Expected an open parenthesis for the condition.");
        // statements.push_back(make_unique<DoWhileStatement>(std::move(ps.statements), std::move(condition->children)));
        return;
    }

    statements.push_back(make_unique<DoStatement>(std::move(ps.statements)));
}

void Parser::parseLoopStatement() {
    auto body = next();

    auto ps = Parser(Lexer(lexer.code, body->children));
    ps.parse();

    statements.push_back(make_unique<LoopStatement>(std::move(ps.statements)));
}

void Parser::parseForLoopStatement() {
    // for ( <identifier>, <identifier>... in <tokens...> ) <group {}>
    // for ( <init statement> ; <condition tokens> ; <iteration statement> ) <group {}>

    auto ins = next();
    if (ins->value[0] != '(') ins->throwError("Expected an open for the for loop initialization.");
    bool is_classic = false;
    for (auto t: ins->children) {
        if (t->type == T_EOE) {
            is_classic = true;
            break;
        }
    }
    auto body = next();

    auto bodyPs = Parser(Lexer(lexer.code, body->children));
    bodyPs.parse();

    if (is_classic) {
        auto spl = splitTokens(ins->children, ";");
        if (spl.size() != 3) ins->throwError("Expected an init, condition and an iterator for the for loop.");
        auto initPs = Parser(Lexer(lexer.code, spl[0]));
        auto iterPs = Parser(Lexer(lexer.code, spl[2]));
        initPs.parse();
        iterPs.parse();
        if (initPs.statements.size() != 1) ins->throwError("Expected a single init statement for the for loop.");
        if (iterPs.statements.size() != 1) ins->throwError("Expected a single iterator statement for the for loop.");
        statements.push_back(make_unique<ForClassicStatement>(
                std::move(initPs.statements[0]),
                std::move(spl[1]),
                std::move(iterPs.statements[0]),
                std::move(bodyPs.statements)
        ));
    } else {
    }
}

void Parser::parseWhileLoopStatement() {
    auto condition = next();
    if (condition->value[0] != '(') condition->throwError("Expected an open parenthesis for the condition.");
    auto body = next();
    auto ps = Parser(Lexer(lexer.code, body->children));
    ps.parse();

    statements.push_back(make_unique<WhileStatement>(std::move(condition->children), std::move(ps.statements)));
}

void Parser::parseIfFlowStatement() {
    auto condition = next();
    if (condition->value[0] != '(') condition->throwError("Expected an open parenthesis for the condition.");
    auto body = next();
    auto children = body->children;
    if (body->value[0] != '{') {
        --index;
        Token *t;
        accumulator.clear();
        while ((t = accumulate()) != lexer.eof && t->type != T_EOL && t->type != T_EOE) {}
        accumulator.pop_back();
        children = accumulator;
    }
    auto ps = Parser(Lexer(lexer.code, children));
    ps.parse();

    statements.push_back(make_unique<IfFlowStatement>(condition->children, std::move(ps.statements),
                                                      vector<unique_ptr<Statement>>()));
}

void Parser::parseElseFlowStatement() {
    if (statements.size() == 0 || statements.back()->type != S_IF_FLOW) {
        current()->throwError("Expected an if statement before the 'else' keyword.");
    }
    IfFlowStatement *ifStatement = (IfFlowStatement *) statements.back().get();
    auto body = next();
    auto children = body->children;
    if (body->value[0] != '{') {
        --index;
        Token *t;
        accumulator.clear();
        while ((t = accumulate()) != lexer.eof && t->type != T_EOL && t->type != T_EOE) {}
        accumulator.pop_back();
        children = accumulator;
    }
    auto ps = Parser(Lexer(lexer.code, children));
    ps.parse();

    ifStatement->elseBody = std::move(ps.statements);
}

void Parser::parseClassDefinitionStatement() {}

void Parser::parseImportStatement() {}

void Parser::parse() {
    while (true) {
        auto token = next();
        if (token == lexer.eof) {
            break;
        }
        if (token->type == T_EOL || token->type == T_EOE) {
            continue;
        }
        if (token->value == "let" || token->value == "const") {
            parseVariableDeclarationStatement();
        } else if (token->value == "fn") {
            parseFunctionDeclarationStatement();
        } else if (token->value == "do") {
            parseDoStatement();
        } else if (token->value == "loop") {
            parseLoopStatement();
        } else if (token->value == "for") {
            parseForLoopStatement();
        } else if (token->value == "break") {
            statements.push_back(make_unique<BreakStatement>());
        } else if (token->value == "continue") {
            statements.push_back(make_unique<ContinueStatement>());
        } else if (token->value == "if") {
            parseIfFlowStatement();
        } else if (token->value == "else") {
            parseElseFlowStatement();
        } else if (token->value == "while") {
            parseWhileLoopStatement();
        } else if (token->value == "class") {
            parseClassDefinitionStatement();
        } else if (token->value == "import") {
            parseImportStatement();
        } else if (token->value == "from") {
            parseImportStatement();
        } else {
            --index;
            accumulator.clear();
            while ((token = accumulate()) != lexer.eof && token->type != T_EOL && token->type != T_EOE) {}
            accumulator.pop_back();
            statements.push_back(make_unique<ExpressionStatement>(accumulator));
        }
    }
}

string Parser::toString() {
    string result = "[";
    size_t i = 0;
    for (const auto &s: statements) {
        result += s->toString() + (i < statements.size() - 1 ? "\n" : "");
        ++i;
    }
    return result + "]";
}

__attribute__((unused)) void Parser::dump() {
    cout << toString() << endl;
}

string Statement::toString() {
    if (type == S_VARIABLE_DECLARATION) {
        return ((VariableDeclarationStatement *) this)->toString();
    } else if (type == S_FUNCTION_DECLARATION) {
        return ((FunctionDeclarationStatement *) this)->toString();
    } else if (type == S_DO) {
        return ((DoStatement *) this)->toString();
    } else if (type == S_LOOP) {
        return ((LoopStatement *) this)->toString();
    } else if (type == S_WHILE) {
        return ((WhileStatement *) this)->toString();
    } else if (type == S_DO_WHILE) {
        return ((DoWhileStatement *) this)->toString();
    } else if (type == S_FOR_ITERATOR) {
        return ((ForIteratorStatement *) this)->toString();
    } else if (type == S_FOR_CLASSIC) {
        return ((ForClassicStatement *) this)->toString();
    } else if (type == S_CLASS_DEFINITION) {
        return ((ClassDefinitionStatement *) this)->toString();
    } else if (type == S_IF_FLOW) {
        return ((IfFlowStatement *) this)->toString();
    } else if (type == S_IMPORT) {
        return ((ImportStatement *) this)->toString();
    } else if (type == S_EXPRESSION) {
        return ((ExpressionStatement *) this)->toString();
    }
    return "Unknown Statement";
}

string statementsToString(string pre, vector<unique_ptr<Statement>> &statements) {
    string result = "";
    string nl = "\n" + pre;
    for (const auto &s: statements) {
        result += pre + regex_replace(s->toString(), regex("\n"), nl) + "\n";
    }
    return result;
}

string VariableDeclarationStatement::toString() {
    return "{'type': 'variable declaration', 'identifier': [\n" + tokensToString("", name) + "], 'value': " +
           tokensToString("", value) +
           ", 'constant': " + (constant ? "true" : "false") + "}";
}

string FunctionDeclarationStatement::toString() {
    return "{'type': 'function declaration', 'name': '" + name->value + "', 'arguments': " +
           tokensListToString("", arguments) + ", 'body': [\n" +
           statementsToString("    ", body) + "]}";
}

string DoStatement::toString() {
    return "{'type': 'do', 'body': [\n" + statementsToString("    ", body) + "]}";
}

string LoopStatement::toString() {
    return "{'type': 'loop', 'body': [\n" + statementsToString("    ", body) + "]}";
}

string WhileStatement::toString() {
    return "{'type': 'while', 'condition': [\n" + tokensToString("    ", condition) + "], 'body': [\n" +
           statementsToString("    ", body) + "]}";
}

string DoWhileStatement::toString() {
    return "{'type': 'do while', 'condition': [\n" + tokensToString("    ", condition) + "], 'body': [\n" +
           statementsToString("    ", body) + "]}";
}

string ForIteratorStatement::toString() {
    return "{'type': 'for iterator', 'index': " + index->toString() + ", 'value': " + value->toString() +
           ", 'iterator': [\n" + tokensToString("    ", iterator) + "]}";
}

string ForClassicStatement::toString() {
    return "{'type': 'for classic', 'init': " + init->toString() + ", 'condition': [\n" +
           tokensToString("    ", condition) + "], 'iterator': " + iterator->toString() + ", 'body': [\n" +
           statementsToString("    ", body) + "]}";
}

string BreakStatement::toString() {
    return "{'type': 'break'}";
}

string ContinueStatement::toString() {
    return "{'type': 'continue'}";
}

string ClassDefinitionStatement::toString() {
    return "{'type': 'class definition', 'attributes': [\n" + statementsToString("", attributes) + "], 'methods': [\n" +
           statementsToString("    ", methods) + "]}";
}

string IfFlowStatement::toString() {
    return "{'type': 'if flow', 'condition': [\n" + tokensToString("    ", condition) + "], "
           + "'body': [\n" + statementsToString("    ", body) + "], "
           + "'elseBody': [\n" + statementsToString("    ", elseBody) + "]}";
}

string ImportStatement::toString() {
    return "{'type': 'import', 'name': [\n" + name->toString() + "], 'imports': [\n" + tokensToString("", imports) +
           "]}";
}

string ExpressionStatement::toString() {
    return "{'type': 'expression', 'expression': [\n" + tokensToString("", expression) + "]}";
}
