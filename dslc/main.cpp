#include <memory>
#include <string>
#include <iostream>

import Scanner.Regex;
import Scanner.RegexParser;
import Scanner.NFA;
import Scanner.ThompsonConstructionVisitor;
import Scanner.PowerSetConstruction;
import Scanner.DFA;
import Scanner.DFAMatcher;

int runScanner(const int argc, const char** argv) {
    if (argc <= 3) {
        std::cout << "Usage: dslc scanner <sub-command> <parameters ...>" << std::endl;
    }

    const std::string subCommand = argv[2];

    if (subCommand == "generate") {
        if (argc < 5) {
            std::cout << "Usage: dslc scanner generate <regex-file> <output-file>" << std::endl;
            return -1;
        }

        std::string regexFile = argv[3];
        std::string outputFile = argv[4];

        // regexFile contains one regular expression per line.
        // Read it and build a DFA from it.

        // Write the DFA into outputFile
        return 0;
    }

    std::cout << "Unknown scanner sub-command: " << subCommand << std::endl;
    return -1;
}

int main(const int argc, const char** argv) {
    if (argc <= 2) {
        std::cout << "Usage: dslc <command> <parameters ...>" << std::endl;
    }

    const std::string command = argv[1];

    if (command == "scanner") {
        return runScanner(argc, argv);
    }

    std::cout << "Unknown command: " << command << std::endl;
    return -1;
}