#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

import Scanner.DFASerializer;
import Scanner.DFAMatcher;

scanner::DFAMatcher buildMatcher(const std::filesystem::path& dfaPath) {
    return scanner::DFAMatcher(scanner::DFASerializer::deserialize(dfaPath));
}

std::string escapeCaptureValue(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());

    for (const char symbol : value) {
        switch (symbol) {
        case '\\':
            escaped += "\\\\";
            break;
        case ',':
            escaped += "\\,";
            break;
        case ';':
            escaped += "\\;";
            break;
        case ':':
            escaped += "\\:";
            break;
        case '=':
            escaped += "\\=";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        default:
            escaped.push_back(symbol);
            break;
        }
    }

    return escaped;
}

void writeMatchingIDs(std::ostream& resultsFile, const std::set<std::uint32_t>& ids) {
    bool first = true;

    for (const auto id : ids) {
        if (!first) {
            resultsFile << ',';
        }
        first = false;
        resultsFile << id;
    }
}

void writeCaptures(std::ostream& resultsFile, const std::vector<scanner::RegexMatchResult>& matches) {
    bool first = true;

    for (const auto& match : matches) {
        const auto& captures = match.getCaptures();
        std::vector<std::string> names;
        names.reserve(captures.size());
        for (const auto& [name, _] : captures) {
            names.emplace_back(name);
        }
        std::sort(names.begin(), names.end());

        for (const auto& name : names) {
            for (const auto& value : captures.at(name)) {
                if (!first) {
                    resultsFile << ',';
                }
                first = false;
                resultsFile << match.getRegexID() << ':' << name << '=' << escapeCaptureValue(value);
            }
        }
    }
}

bool writeResultLine(std::ostream& resultsFile, std::size_t lineNumber, const scanner::MatchingResult& matches) {
    resultsFile << lineNumber << ';';
    writeMatchingIDs(resultsFile, matches.getMatchingIDs());
    resultsFile << ';';
    writeCaptures(resultsFile, matches.getMatches());

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
        const auto matches = matcher.getMatches(line, true);

        if (!writeResultLine(resultsFile, lineNumber, matches)) {
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
