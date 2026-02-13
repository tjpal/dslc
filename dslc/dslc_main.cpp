#include <algorithm>
#include <cctype>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

import Scanner.Regex;
import Scanner.RegexParser;
import Scanner.NFA;
import Scanner.ThompsonConstructionVisitor;
import Scanner.PowerSetConstruction;
import Scanner.DFA;
import Scanner.DFAMatcher;
import Scanner.Generator;
import Scanner.DFASerializer;

std::string trimWhitespace(std::string_view text) {
    const auto begin = std::find_if_not(text.begin(), text.end(), [](unsigned char ch) {
        return std::isspace(ch);
    });

    const auto end = std::find_if_not(text.rbegin(), text.rend(), [](unsigned char ch) {
        return std::isspace(ch);
    }).base();

    if (begin >= end) {
        return {};
    }

    return std::string(begin, end);
}

std::vector<std::string> loadRegexExpressions(const std::filesystem::path& regexFilePath) {
    std::ifstream regexFile(regexFilePath);
    if (!regexFile) {
        throw std::runtime_error("Failed to open regex file: " + regexFilePath.string());
    }

    std::vector<std::string> expressions;
    std::string line;
    while (std::getline(regexFile, line)) {
        const std::string expression = trimWhitespace(line);
        if (expression.empty()) {
            continue;
        }

        expressions.emplace_back(expression);
    }

    return expressions;
}

bool hasProfileFlag(const std::vector<std::string>& arguments) {
    return std::find(arguments.begin(), arguments.end(), "--profile") != arguments.end();
}

int runScanner(const int argc, const char** argv) {
    if (argc < 3) {
        std::cout << "Usage: dslc scanner <sub-command> <parameters ...>" << std::endl;
        return -1;
    }

    const std::string subCommand = argv[2];

    if (subCommand == "generate") {
        if (argc < 5) {
            std::cout << "Usage: dslc scanner generate [--profile] <regex-file> <output-file>" << std::endl;
            return -1;
        }

        std::vector<std::string> positionalArgs;
        positionalArgs.reserve(static_cast<std::size_t>(argc - 3));
        std::vector<std::string> providedOptions;
        providedOptions.reserve(static_cast<std::size_t>(argc - 3));

        for (int index = 3; index < argc; ++index) {
            const std::string argument = argv[index];
            providedOptions.emplace_back(argument);
            if (argument == "--profile") {
                continue;
            }

            positionalArgs.emplace_back(argument);
        }

        const bool profile = hasProfileFlag(providedOptions);

        if (positionalArgs.size() != 2) {
            std::cout << "Usage: dslc scanner generate [--profile] <regex-file> <output-file>" << std::endl;
            return -1;
        }

        const std::filesystem::path regexFilePath = positionalArgs[0];
        const std::filesystem::path outputFilePath = positionalArgs[1];

        try {
            const std::vector<std::string> expressions = loadRegexExpressions(regexFilePath);
            if (expressions.empty()) {
                std::cerr << "No regular expressions found in " << regexFilePath << std::endl;
                return -1;
            }

            scanner::Generator generator;
            generator.setProfilingEnabled(profile);
            const scanner::DFA dfa = generator.generateScanner(expressions);
            generator.printStatistics(std::cout);

            scanner::DFASerializer::serialize(dfa, outputFilePath);
        } catch (const std::exception& error) {
            std::cerr << "Failed to generate scanner: " << error.what() << std::endl;
            return -1;
        }

        return 0;
    }

    std::cout << "Unknown scanner sub-command: " << subCommand << std::endl;
    return -1;
}

int main(const int argc, const char** argv) {
    if (argc < 2) {
        std::cout << "Usage: dslc <command> <parameters ...>" << std::endl;
        return -1;
    }

    const std::string command = argv[1];

    if (command == "scanner") {
        return runScanner(argc, argv);
    }

    std::cout << "Unknown command: " << command << std::endl;
    return -1;
}
