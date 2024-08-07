#ifndef NEO_ERROR_H
#define NEO_ERROR_H

#include <string>
#include <cstdint>

using namespace std;

const string RED = "\033[31m";
const string WHITE_BG = "\033[47m";
const string UNDERLINE = "\033[4m";
const string RESET = "\033[0m";

void showCodeSnippet(const string& color, const string& code, size_t start, size_t end);
void throwError(const string &message, const string& code, size_t start, size_t end);

#endif //NEO_ERROR_H