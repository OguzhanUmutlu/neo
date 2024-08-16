#include "error.hpp"
#include <string>
#include <iostream>
#include <vector>
#include <sstream>

void showCodeSnippet(const string &color, const string &filename, const string &code, size_t index) {
    vector<string> lines;
    stringstream ss(code);
    string line;

    while (getline(ss, line)) {
        lines.push_back(line);
    }

    size_t charCount = 0;
    for (size_t i = 0; i < lines.size(); ++i) {
        size_t oldCharCount = charCount;
        charCount += lines[i].size() + 1;
        if (index >= oldCharCount && index < charCount) {
            line = lines[i];
            size_t ind = index - oldCharCount;
            cout << color << "Error on file " << filename << ", line " << (i + 1) << ", column "
                 << (ind + 1) << ":" << RESET << endl;
            cout << "    " << ITALIC << line.substr(0, ind) << (ind >= line.size() ? ' ' : line[ind])
                 << (ind >= line.size() ? " " : line.substr(ind + 1)) << RESET << endl;
            for (size_t j = 0; j < ind + 4; ++j) {
                cout << " ";
            }
            return;
        }
    }
    return;
}

void showError(const string &message, const string &filename, const string &code, size_t index) {
    showCodeSnippet(RED, filename, code, index);
    cout << YELLOW << "^ " << message << endl;
}

void throwError(const string &message, const string &filename, const string &code, size_t index) {
    showError(message, filename, code, index);
    exit(1);
}