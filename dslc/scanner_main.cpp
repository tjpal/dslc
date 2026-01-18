#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>

import Scanner.DFASerializer;
import Scanner.DFAMatcher;

scanner::DFAMatcher buildMatcher(const std::filesystem::path& dfaPath) {
    return scanner::DFAMatcher(scanner::DFASerializer::deserialize(dfaPath));
}

bool writeResultLine(std::ostream& resultsFile, std::size_t lineNumber, const std::set<std::uint32_t>& ids) {
    resultsFile << lineNumber << ';';
    bool first = true;

    for (const auto id : ids) {
        if (!first) {
            resultsFile << ',';
        }
        first = false;
        resultsFile << id;
    }

    resultsFile << '\n';
    return static_cast<bool>(resultsFile);
}

bool processInputFile(const scanner::DFAMatcher& matcher,
                      std::istream& inputFile,
                      std::ostream& resultsFile,
                      const std::filesystem::path& resultsPath) {
    std::string line;
    std::size_t lineNumber = 0;

    while (std::getline(inputFile, line)) {
        ++lineNumber;
        const auto matchedIds = matcher.getMatchingIDs(line);

        if (!writeResultLine(resultsFile, lineNumber, matchedIds)) {
            std::cerr << "Failed to write to results file: " << resultsPath << std::endl;
            return false;
        }
    }

    if (inputFile.bad()) {
        std::cerr << "Error reading input file: " << resultsPath << std::endl;
        return false;
    }

    return true;
}

bool runScanner(const std::filesystem::path& dfaPath,
                const std::filesystem::path& inputPath,
                const std::filesystem::path& resultsPath) {
    scanner::DFAMatcher matcher = buildMatcher(dfaPath);

    std::ifstream inputFile(inputPath);
    if (!inputFile) {
        std::cerr << "Failed to open input file: " << inputPath << std::endl;
        return false;
    }

    std::ofstream resultsFile(resultsPath, std::ios::trunc);
    if (!resultsFile) {
        std::cerr << "Failed to open results file: " << resultsPath << std::endl;
        return false;
    }

    return processInputFile(matcher, inputFile, resultsFile, resultsPath);
}

int main(const int argc, const char** argv) {
    if (argc != 4) {
        std::cout << "Usage: dslc-scanner <dfa-file> <input-file> <results-file>" << std::endl;
        return -1;
    }

    const std::filesystem::path dfaPath(argv[1]);
    const std::filesystem::path inputPath(argv[2]);
    const std::filesystem::path resultsPath(argv[3]);

    try {
        if (!runScanner(dfaPath, inputPath, resultsPath)) {
            return -1;
        }
    } catch (const std::exception& exception) {
        std::cerr << "Failed to run scanner: " << exception.what() << std::endl;
        return -1;
    }

    return 0;
}
