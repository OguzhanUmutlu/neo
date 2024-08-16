#include <fstream>
#include "compiler.hpp"

#define FUNCTION_PARAMETERS "NeoObject *this, NeoObject **args, size_t arg_count, NeoHashMap *kwargs"

unordered_map<string, int> operatorPrecedence = {
        {"**", 4},
        {"*",  3},
        {"/",  3},
        {"%",  3},
        {"+",  2},
        {"-",  2},
        {"==", 1},
        {"!=", 1},
        {">",  1},
        {"<",  1},
        {">=", 1},
        {"<=", 1},
        {"||", 0},
        {"&&", 0}
};
unordered_map<string, string> operatorNames = {
        {"+",  "add"},
        {"-",  "subtract"},
        {"*",  "multiply"},
        {"/",  "divide"},
        {"%",  "modulo"},
        {"**", "pow"},
        {"==", "equals"},
        {"!=", "not_equals"},
        {">",  "greater_than"},
        {"<",  "less_than"},
        {">=", "greater_or_equals"},
        {"<=", "less_or_equals"},
        {"||", "or"},
        {"&&", "and"}
};

void Scope::append(string code, bool indent) {
    fnCode += (indent ? indentStr : "") + code;
}

void Scope::clearVariables() {
    for (auto it = variables.begin(); it != variables.end(); ++it) {
        if (it->second.isFunction || returning.pointer == it->second.pointer) continue;
        append("NEO_dereference(" + it->second.pointer + ");\n");
    }
}

void Scope::clearTemp() {
    for (auto t: temp) {
        if (t == returning.pointer) continue;
        append("NEO_dereference(" + t + ");\n");
    }
    temp.clear();
}

VariableDefinition *Scope::getVariableDefinition(string name) {
    auto scope = this;
    while (scope != nullptr) {
        if (scope->variables.find(name) != scope->variables.end()) {
            return &scope->variables[name];
        }
        scope = scope->parent;
    }
    return nullptr;
}

void Compiler::introduceFunction(Scope *scope, string name, vector<unique_ptr<Statement>> *statements, bool isLambda) {
    string fnId = isLambda ? "_neo_lambda_" + to_string(++_id) : "_neo_fn_" + to_string(scope->id) + "_" + name;
    string fnKey = "NeoObject *" + fnId + "(" FUNCTION_PARAMETERS ")";

    if (!isLambda) {
        string varId = "_neo_var_" + to_string(scope->id) + "_" + name;
        globalCode += "NeoObject *" + varId + ";\n";
        functions["void NEO_initFunctions()"] += "\t" + varId + " = NEO_function(" + fnId + ");\n";
        functions["void NEO_freeFunctions()"] += "\tNEO_dereference(" + varId + ");\n";
        scope->variables[name] = VariableDefinition(varId, true, true);
    }

    globalCode += fnKey + ";\n";

    functions[fnKey] = "";
    auto fnScope = new Scope(++_id, functions[fnKey], scope, false);
    compileScope(fnScope, statements);
    fnScope->append("return NULL;\n");
    delete fnScope;
}

void Compiler::compile() {
    functions["void NEO_initFunctions()"] = "";
    functions["void NEO_freeFunctions()"] = "";
    functions["int main(int argc, char *argv[])"] = "\tNEO_init(argc, argv);\n\tNEO_initFunctions();\n";
    globalCode += "void NEO_initFunctions();\n";
    globalCode += "void NEO_freeFunctions();\n";
    auto mainScope = new Scope(++_id, functions["int main(int argc, char *argv[])"], nullptr, false);
    compileScope(mainScope, &parser.statements);
    functions["int main(int argc, char *argv[])"] += "\tNEO_freeFunctions();\n\tNEO_exit(0);\n";
    delete mainScope;
    string code = "#include \"../api/include/neo.h\"\n\n" + globalCode + "\n";
    for (auto f: functions) {
        code += f.first + " {\n" + f.second + "}\n\n";
    }
    code.pop_back();
    if (missingFunctionDefinitions.size() > 0) {
        auto f = missingFunctionDefinitions[0];
        f.errorToken->throwError(
                "NameError: Function '" + f.functionName + "' is not defined");
    }

    ofstream file;
    file.open("output/main.c");
    file << code;
    file.close();

#ifdef WIN32
#define OS_NAME "windows"
#else
#ifdef __APPLE__
#define OS_NAME "macos"
#else
#define OS_NAME "linux"
#endif
#endif
    system("gcc output/main.c -Iapi/include api/neo.c api/types/*.c -lgmp -lmpfr -lm -o output/main");
    //system("gcc output/main.c -Iapi/include -Lapi/build -lneo-" OS_NAME " -lgmp -lmpfr -lm -o output/main");
#ifdef WIN32
    system(".\\output\\main.exe");
#else
    system("./output/main");
#endif
}

CompileTimeValue Compiler::executeToken(Scope *scope, Token *t0) {
    string val;
    if (t0->type == T_INTERNAL_IDENTIFIER) {
        return {CTV_VARIABLE, t0->value};
    } else if (t0->type == T_GROUP && t0->value[0] == '(') {
        if (t0->children.size() == 0) {
            t0->throwError("SyntaxError: Expected expression inside parenthesis");
        }
        return executeExpression(scope, t0->children);
    } else if (t0->type == T_IDENTIFIER) {
        if (t0->value == "print") {
            return {CTV_VARIABLE, "NeoGlobPrint"};
        }
        if (t0->value == "input") {
            return {CTV_VARIABLE, "NeoGlobInput"};
        }
        if (t0->value == "true") {
            return {CTV_VARIABLE, "NeoTrue"};
        }
        if (t0->value == "false") {
            return {CTV_VARIABLE, "NeoFalse"};
        }
        VariableDefinition *def = scope->getVariableDefinition(t0->value);
        if (def == nullptr) {
            return {CTV_INVALID_VARIABLE};
        }
        return {CTV_VARIABLE, def->pointer};
    } else {
        string store = "_neo_temp_" + to_string(++_id);
        if (t0->type == T_NUMBER) {
            bool is_big = t0->value.find('n') != string::npos;
            if (t0->value.find('.') == string::npos && t0->value.find('e') == string::npos) {
                if (is_big) {
                    // big int
                    scope->append("NeoObject *" + store + " = NEO_bigint_str(\"" + t0->value + "\");" + "\n");
                } else {
                    // int32
                    scope->append("NeoObject *" + store + " = NEO_int(" + t0->value + ");" + "\n");
                }
            } else {
                // double
                if (is_big) {
                    scope->append("NeoObject *" + store + " = NEO_bigfloat_str(\"" + t0->value + "\");" + "\n");
                } else {
                    scope->append("NeoObject *" + store + " = NEO_double(" + t0->value + ");" + "\n");
                }
            }
            return {CTV_TEMP, store};
        } else if (t0->type == T_STRING) {
            scope->append("NeoObject *" + store + " = NEO_string3(" + t0->value + ");" + "\n");
            return {CTV_TEMP, store};
        } else if (t0->type == T_GROUP) {
            if (t0->value[0] == '[') {
                scope->append("NeoObject *" + store + " = NEO_array();" + "\n");
                auto values = splitTokens(t0->children, ",", true);
                for (auto &value: values) {
                    if (value.size() == 0) {
                        t0->throwError("SyntaxError: Invalid array value");
                    }
                    auto valueStore = executeExpression(scope, value);
                    scope->append("internal_NEO_array_push(" + store + ", " + valueStore.pointer + ");\n");
                    scope->append("NEO_dereference(" + valueStore.pointer + ");\n");
                }
                return {CTV_TEMP, store};
            } else if (t0->value[0] == '{') {
                scope->append("NeoObject *" + store + " = NEO_object();" + "\n");
                auto values = splitTokens(t0->children, ",", true);
                for (auto &value: values) {
                    auto kv = splitTokens(value, ":", true);
                    if (kv.size() != 2) {
                        kv[0][0]->throwError("SyntaxError: Invalid key-value pair.");
                    }
                    if (kv[0].size() != 1) {
                        kv[0][0]->throwError("SyntaxError: Invalid object key.");
                    }
                    auto key = kv[0][0];
                    auto valueStore = executeExpression(scope, kv[1]);
                    if (key->type == T_IDENTIFIER || key->type == T_STRING) {
                        auto keyValue = key->value;
                        if (key->type == T_IDENTIFIER) {
                            keyValue = "\"" + key->value + "\"";
                        }
                        scope->append(
                                "NEO_set_object_property(" + store + ", " + keyValue + ", " + valueStore.pointer +
                                ");\n");
                        scope->append("NEO_dereference(" + valueStore.pointer + ");\n");
                    } else if (key->type == T_GROUP && key->value[0] == '[') {
                        auto keyStore = executeExpression(scope, key->children);
                        string tempStr = "_neo_temp_" + to_string(++_id);
                        scope->append("char *" + tempStr + " = NEO_to_string(" + keyStore.pointer + ");\n");
                        scope->append("NEO_set_object_property(" + store + ", " + tempStr + ", " + valueStore.pointer +
                                      ");\n");
                        scope->append("free(" + tempStr + ");\n");
                        scope->append("NEO_dereference(" + keyStore.pointer + ");\n");
                        scope->append("NEO_dereference(" + valueStore.pointer + ");\n");
                    } else {
                        key->throwError("SyntaxError: Invalid object key.");
                    }
                }
                return {CTV_TEMP, store};
            } else {
                t0->throwError("Assumption failed");
                return {};
            }
        } else {
            t0->throwError("SyntaxError: Unexpected token '" + t0->value + "'");
            return {};
        }
    }
}

CompileTimeValue Compiler::executeSingleExpression(Scope *scope, vector<Token *> tokens) {
    // unary operations: !, ~, -, +, ++, --
    // check if the first token is one of them and use the rest as the expression
    if (tokens.size() == 0) {
        cout << "Untraceable empty expression" << endl;
        exit(1);
    }
    auto t0 = tokens[0];
    if (t0->value == "!" || t0->value == "~" || t0->value == "-" || t0->value == "+" || t0->value == "++" ||
        t0->value == "--") {
        string op = t0->value;
        tokens.erase(tokens.begin());
        auto val = executeSingleExpression(scope, tokens);
        if (op == "!" || op == "~") {
            string temp = "_neo_temp_" + to_string(++_id);
            scope->append("NeoObject *" + temp + " = NEO_" + operatorNames[op] + "(" + val.pointer + ");\n");
            if (val.type == CTV_TEMP) {
                scope->append("NEO_dereference(" + val.pointer + ");\n");
            }
            return {CTV_TEMP, temp};
        } else if (op == "+") {
            return val;
        } else if (op == "-") {
            string temp = "_neo_temp_" + to_string(++_id);
            scope->append("NeoObject *" + temp + " = NEO_negate(" + val.pointer + ");\n");
            if (val.type == CTV_TEMP) {
                scope->append("NEO_dereference(" + val.pointer + ");\n");
            }
            return {CTV_TEMP, temp};
        } else {
            string temp = "_neo_temp_" + to_string(++_id);
            scope->append(
                    "NeoObject *" + temp + " = NEO_" + (op == "++" ? "add" : "subtract") + "(" + val.pointer + ");\n");
            if (val.type == CTV_TEMP) {
                scope->append("NEO_dereference(" + val.pointer + ");\n");
            }
            scope->append(val.pointer + " = " + temp + ";\n");
            return {CTV_TEMP, temp};
        }
    }
    auto val = executeToken(scope, t0);
    bool missingFunction = false;
    if (val.type == CTV_INVALID_VARIABLE) {
        if (tokens.size() == 1 || tokens[1]->type != T_GROUP || tokens[1]->value[0] != '(') {
            t0->throwError("SyntaxError: '" + t0->value + "' is not defined");
            exit(1);
        } else missingFunction = true;
    }

    auto tokens_size = tokens.size();
    for (size_t i = 1; i < tokens_size; i++) {
        auto t = tokens[i];
        if (t->value == ".") {
            if (tokens[i - 1]->value == "." || i == tokens_size - 1) {
                t->throwError("SyntaxError: Unexpected '.'");
            }
            continue;
        }
        CompileTimeValue newStore = {CTV_TEMP, "_neo_temp_" + to_string(++_id)};
        scope->append("NeoObject *" + newStore.pointer + ";\n");
        if (t->type == T_IDENTIFIER) {
            // indexing
            scope->append(
                    newStore.pointer + " = NEO_get_object_property(" + val.pointer + ", \"" + t->value + "\");\n");
            val = newStore;
        } else if (t->type == T_GROUP && t->value[0] == '(') {
            vector<CompileTimeValue> args;
            unordered_map<string, CompileTimeValue> kwargs;
            for (auto arg: splitTokens(t->children, ",", true)) {
                if (arg.size() > 2 && arg[0]->type == T_IDENTIFIER && arg[1]->value == ":") {
                    string key = arg[0]->value;
                    arg.erase(arg.begin(), arg.begin() + 2);
                    kwargs[key] = executeExpression(scope, arg);
                } else {
                    args.push_back(executeExpression(scope, arg));
                }
            }
            string argsValue = "NULL, 0";
            string kwargsValue = "NeoEmptyHashmap";
            if (args.size() > 0) {
                string argsStore = "_neo_temp_" + to_string(++_id);
                argsValue = argsStore + ", " + to_string(args.size());
                scope->append("NeoObject *" + argsStore + "[] = { ");
                for (size_t j = 0; j < args.size(); j++) {
                    if (j > 0) scope->fnCode += ", ";
                    scope->fnCode += args[j].pointer;
                }
                scope->fnCode += " };\n";
            }
            if (kwargs.size() > 0) {
                kwargsValue = "_neo_temp_" + to_string(++_id);
                scope->append("NeoHashMap *" + kwargsValue + " = NEO_create_hashmap(" +
                              to_string((int) kwargs.size() * 1.33) + ");\n");
                for (auto kwarg: kwargs) {
                    scope->append("NEO_hashmap_set(" + kwargsValue + ", \"" + kwarg.first + "\", " +
                                  kwarg.second.pointer + ");\n");
                }
            }
            string callArguments = argsValue + ", " + kwargsValue;
            if (missingFunction) {
                missingFunction = false;
                scope->append(newStore.pointer + " = NEO_call(");
                vector<size_t> positions;
                positions.push_back(scope->fnCode.size());
                scope->fnCode += ", ";
                positions.push_back(scope->fnCode.size());
                scope->fnCode += ", " + callArguments + ");\n";
                missingFunctionDefinitions.push_back({scope, t0, t0->value, positions});
                val = newStore;
            } else {
                scope->append(
                        newStore.pointer + " = NEO_call(" + val.pointer + ", " + val.pointer + ", " + callArguments +
                        ");\n");
                val = newStore;
            }
        } else if (t->type == T_GROUP && t->value[0] == '[') {
            // bracket indexing
            if (t->children.size() == 0) {
                t->throwError("SyntaxError: Expected expression");
            }
            auto key = executeExpression(scope, t->children);
            string tempStr = "_neo_temp_" + to_string(++_id);
            scope->append("char *" + tempStr + " = NEO_to_string(" + key.pointer + ");\n");
            scope->append("NEO_dereference(" + key.pointer + ");\n");
            scope->append(newStore.pointer + " = NEO_get_object_property(" + val.pointer + ", " + tempStr + ");\n");
            scope->append("free(" + tempStr + ");\n");
            val = newStore;
        } else {
            t->throwError("SyntaxError: Unexpected token '" + t->value + "'");
        }
    }
    return val;
}

CompileTimeValue Compiler::computeBinaryOperation(Scope *scope, vector<Token *> a, Token *op, vector<Token *> b) {
    auto av = executeSingleExpression(scope, a);
    auto bv = executeSingleExpression(scope, b);
    CompileTimeValue store = {CTV_TEMP, "_neo_temp_" + to_string(++_id)};
    scope->append("NeoObject *" + store.pointer + " = NEO_" + operatorNames[op->value] + "(" + av.pointer + ", " +
                  bv.pointer + ");\n");
    if (av.type == CTV_TEMP) {
        scope->append("NEO_dereference(" + av.pointer + ");\n");
    }
    if (bv.type == CTV_TEMP) {
        scope->append("NEO_dereference(" + bv.pointer + ");\n");
    }
    return store;
}

CompileTimeValue Compiler::executeExpression(Scope *scope, vector<Token *> tokens) {
    auto sep = separateExpression(tokens);
    return executeSeparatedExpression(scope, sep);
}

CompileTimeValue Compiler::executeSeparatedExpression(Scope *scope, vector<vector<Token *>> sep) {
    if (sep.size() > 1 && sep[1][0]->type == T_SET_OPERATOR) {
        if (sep.size() < 3) {
            sep[1][0]->throwError("SyntaxError: Expected an expression");
        }
        string op = sep[1][0]->value;
        if (op == ":=") {
            sep[1][0]->throwError("SyntaxError: Cannot use ':=' inside expressions");
        }
        CompileTimeValue original;
        if (op != "=") {
            original = executeSingleExpression(scope, sep[0]);
        }
        auto last = sep[0].back();
        sep[0].pop_back();
        CompileTimeValue var;
        bool isSingle = sep[0].size() == 0;
        if (isSingle) {
            var = executeToken(scope, last);
            if (var.type == CTV_INVALID_VARIABLE) {
                last->throwError("SyntaxError: '" + last->value + "' is not defined");
            }
        } else {
            var = executeSingleExpression(scope, sep[0]);
        }
        sep.erase(sep.begin(), sep.begin() + 2);
        auto value = executeSeparatedExpression(scope, sep);
        if (op != "=") {
            string temp = "_neo_temp_" + to_string(++_id);
            scope->append(
                    "NeoObject *" + temp + " = NEO_" + operatorNames[string(1, op[0])] + "(" + value.pointer + ", " +
                    original.pointer + ");\n");
            if (value.type == CTV_TEMP) {
                scope->append("NEO_dereference(" + value.pointer + ");\n");
            }
            value = {CTV_TEMP, temp};
        }
        if (isSingle) {
            if (var.pointer == value.pointer) {
                return var; // x = x
            }
            scope->append("NEO_reference(" + value.pointer + ");\n");
            scope->append("NEO_dereference(" + var.pointer + ");\n");
            scope->append(var.pointer + " = " + value.pointer + ";\n");
        } else {
            if (last->type != T_IDENTIFIER && last->value[0] != '[') {
                last->throwError("SyntaxError: Invalid indexing operation");
            }
            if (last->type == T_IDENTIFIER) {
                scope->append("NEO_set_object_property(" + var.pointer + ", \"" + last->value + "\", " + value.pointer +
                              ");\n");
            } else {
                auto keyValue = executeExpression(scope, last->children);
                scope->append("_tempStr = NEO_to_string(" + keyValue.pointer + ");\n");
                scope->append("NEO_set_object_property(" + var.pointer + ", _tempStr, " + value.pointer + ");\n");
                scope->append("free(_tempStr);\n");
            }
        }

        return var;
    }
    auto len = sep.size();
    if (len == 1) {
        return executeSingleExpression(scope, sep[0]);
    }
    vector<vector<Token *>> output;
    vector<vector<Token *>> stack;
    for (int i = 0; i < len; ++i) {
        auto chain = sep[i];
        if (i % 2 == 0) {
            if (IsAnyOperatorToken(chain[0])) {
                chain[0]->throwError("SyntaxError: Expected an expression got an operator");
            }
            output.push_back(chain);
            continue;
        }
        auto o1 = chain;
        while (1) {
            auto o2 = stack.empty() ? vector<Token *>{} : stack.back();
            if (stack.empty() || (
                    operatorPrecedence[o2[0]->value] <= operatorPrecedence[o1[0]->value] &&
                    (operatorPrecedence[o2[0]->value] != operatorPrecedence[o1[0]->value] || o1[0]->value == "**")
            )) {
                break;
            }
            output.push_back(stack.back());
            stack.pop_back();
        }
        stack.push_back(o1);
    }
    while (!stack.empty()) {
        output.push_back(stack.back());
        stack.pop_back();
    }
    CompileTimeValue store;
    for (auto &token: output) {
        if (!IsAnyOperatorToken(token[0])) {
            stack.push_back(token);
            continue;
        }

        auto right = stack.back();
        stack.pop_back();
        if (stack.empty()) {
            token[0]->throwError("SyntaxError: Expected expression after operator");
        }
        auto left = stack.back();
        stack.pop_back();
        store = computeBinaryOperation(scope, left, token[0], right);
        stack.push_back({new Token(T_INTERNAL_IDENTIFIER, "", "", 0, store.pointer.size(), store.pointer)});
    }

    for (auto &token: stack) {
        if (token[0]->type == T_INTERNAL_IDENTIFIER) {
            delete token[0];
        }
    }

    return store;
}

void Compiler::compileScope(Scope *scope, vector<unique_ptr<Statement>> *statements) {
    for (auto &statement: *statements) {
        if (statement->type == S_EXPRESSION) {
            unique_ptr<ExpressionStatement> &st = (unique_ptr<ExpressionStatement> &) statement;
            auto v = executeExpression(scope, st->expression);
            if (v.type == CTV_TEMP) {
                scope->append("NEO_dereference(" + v.pointer + ");\n");
            }
        } else if (statement->type == S_VARIABLE_DECLARATION) {
            unique_ptr<VariableDeclarationStatement> &st = (unique_ptr<VariableDeclarationStatement> &) statement;
            if (scope->variables.find(st->name->value) != scope->variables.end()) {
                st->name->throwError("SyntaxError: Variable '" + st->name->value + "' already defined");
            }
            string varId = "_neo_var_" + to_string(scope->id) + "_" + st->name->value;
            globalCode += "NeoObject *" + varId + ";\n";
            auto value = executeExpression(scope, st->value);
            scope->append(varId + " = " + value.pointer + ";\n");
            scope->variables[st->name->value] = VariableDefinition(varId, st->constant, false);
        } else if (statement->type == S_DO) {
            unique_ptr<DoStatement> &st = (unique_ptr<DoStatement> &) statement;
            auto newScope = new Scope(++_id, scope->fnCode, scope, scope->isLoop);
            newScope->indentStr = scope->indentStr;
            compileScope(newScope, &st->body);
            delete newScope;
        } else if (statement->type == S_IF_FLOW) {
            unique_ptr<IfFlowStatement> &st = (unique_ptr<IfFlowStatement> &) statement;
            auto condition = executeExpression(scope, st->condition);
            scope->append("if (NEO_get_truthy(" + condition.pointer + ")) {\n");
            auto newScope = new Scope(++_id, scope->fnCode, scope, scope->isLoop);
            newScope->indentStr = scope->indentStr + "\t";
            compileScope(newScope, &st->body);
            delete newScope;
            scope->append("}");
            if (st->elseBody.size() > 0) {
                scope->append(" else {\n", false);
                newScope = new Scope(++_id, scope->fnCode, scope, scope->isLoop);
                newScope->indentStr = scope->indentStr + "\t";
                compileScope(newScope, &st->elseBody);
                scope->append("}\n");
            } else scope->append("\n", false);
            if (condition.type == CTV_TEMP) {
                scope->append("NEO_dereference(" + condition.pointer + ");\n");
            }
        } else if (statement->type == S_FUNCTION_DECLARATION) {
            unique_ptr<FunctionDeclarationStatement> &st = (unique_ptr<FunctionDeclarationStatement> &) statement;
            if (scope->variables.find(st->name->value) != scope->variables.end()) {
                st->name->throwError("SyntaxError: '" + st->name->value + "' is already defined");
            }
            vector<MissingFunctionDefinition> newMissing;
            vector<size_t> indexes;
            for (int i = missingFunctionDefinitions.size() - 1; i >= 0; --i) {
                auto missing = missingFunctionDefinitions[i];
                if (missing.functionName == st->name->value) {
                    for (int j = missing.scopePoint.size() - 1; j >= 0; --j) {
                        missing.scope->fnCode.insert(missing.scopePoint[j],
                                                     "_neo_var_" + to_string(scope->id) + "_" + st->name->value);
                    }
                } else {
                    newMissing.insert(newMissing.begin(), missing);
                }
            }
            missingFunctionDefinitions.clear();
            missingFunctionDefinitions = newMissing;
            introduceFunction(scope, st->name->value, &st->body, false);
        } else if (statement->type == S_RETURN) {
            unique_ptr<ReturnStatement> &st = (unique_ptr<ReturnStatement> &) statement;
            if (st->value.size() == 0) {
                scope->append("return NULL;\n");
            } else {
                auto result = executeExpression(scope, st->value);
                scope->returning = result;
                scope->clearVariables();
                scope->clearTemp();
                scope->append("return " + result.pointer + ";\n");
            }
            return;
        } else if (statement->type == S_BREAK) {
            scope->append("break;\n");
            return;
        } else if (statement->type == S_CONTINUE) {
            scope->append("break;\n");
            return;
        } else if (statement->type == S_LOOP) {
            scope->append("while(1) {\n");
            auto newScope = new Scope(++_id, scope->fnCode, scope, true);
            newScope->indentStr = scope->indentStr + "\t";
            compileScope(newScope, &((unique_ptr<LoopStatement> &) statement)->body);
            scope->append("}\n");
            return;
        } else {
            cout << "Unhandled statement: " << statement->type << endl;
            exit(1);
        }
    }
    scope->clearVariables();
    scope->clearTemp();
}
