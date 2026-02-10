cmake_minimum_required(VERSION 3.30)

project(scanner)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(libscanner STATIC)

target_sources(
    libscanner PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    utilities/SpecialSymbols.cppm
    dfa/DFA.cppm
    dfa/DFAMatcher.cppm
    dfa/DFAAcceptingState.cppm
    dfa/DFASerializer.cppm
    regex/Regex.cppm
    regex/RegexNode.cppm
    regex/Concatenation.cppm
    regex/Kleene.cppm
    regex/Plus.cppm
    regex/Leaf.cppm
    regex/Optional.cppm
    regex/Union.cppm
    regex/RegexNodeVisitor.cppm
    parser/RegexParser.cppm
    generator/Generator.cppm
    generator/nfa/MergedNFA.cppm
    generator/nfa/NFANode.cppm
    generator/nfa/NFANodeFactory.cppm
    generator/nfa/NFAEdge.cppm
    generator/nfa/NFA.cppm
    generator/thompson-construction/ThompsonConstructionVisitor.cppm
    generator/nfa-dfa-conversion/StateSet.cppm
    generator/nfa-dfa-conversion/eClosure.cppm
    generator/nfa-dfa-conversion/PowerSetConstruction.cppm
)
