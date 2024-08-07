#include "error.h"
#include <string>
#include <iostream>
#include <vector>
#include <sstream>

void showCodeSnippet(const string &color, const string &code, size_t start, size_t end) {
    vector<string> lines;
    stringstream ss(code);
    string line;

    while (getline(ss, line)) {
        lines.push_back(line);
    }

    size_t lineStart = 0, lineEnd = 0, charCount = 0;

    for (size_t i = 0; i < lines.size(); ++i) {
        charCount += lines[i].size() + 1; // +1 for the newline character

        if (charCount > start && lineStart == 0) {
            lineStart = i;
        }
        if (charCount >= end) {
            lineEnd = i;
            break;
        }
    }

    cout << color;

    for (size_t i = lineStart;
         i <= lineEnd; ++i) {
        cout << i + 1 << " | ";

        size_t lineStartPos = (i == lineStart) ? start - (charCount - lines[i].size() - 1) : 0;
        size_t lineEndPos = (i == lineEnd) ? end - (charCount - lines[i].size() - 1) : lines[i].size();

        cout << lines[i].substr(0, lineStartPos);
        cout << UNDERLINE << lines[i].substr(lineStartPos, lineEndPos - lineStartPos) << RESET << color;
        if (lines[i].size() > lineEndPos) {
            cout << lines[i].substr(lineEndPos);
        } else {
            cout << WHITE_BG << " ";
        }
        cout << RESET;
        cout << endl;
    }
}

void throwError(const string &message, const string &code, size_t start, size_t end) {
    showCodeSnippet(RED, code, start, end);
    cout << RED << message << endl;
    exit(1);
}