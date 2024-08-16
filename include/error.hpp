#ifndef NEO_ERROR_HPP
#define NEO_ERROR_HPP

#include <string>
#include <cstdint>

using namespace std;

#define RED "\033[31m"
#define YELLOW "\033[33m"
#define ITALIC "\033[3m"
#define UNDERLINE "\033[4m"
#define RESET "\033[0m"

void showCodeSnippet(const string &color, const string &filename, const string &code, size_t index);

void throwError(const string &message, const string &filename, const string &code, size_t index);

void showError(const string &message, const string &filename, const string &code, size_t index);

#endif //NEO_ERROR_HPP