cmake_minimum_required(VERSION 3.30)

project(scanner)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(scanner STATIC)

target_sources(
    scanner PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    dfa/DFA.cppm
    dfa/DFAMatcher.cppm
    regex/Regex.cppm
    regex/RegexNode.cppm
    regex/Concatenation.cppm
    regex/Kleene.cppm
    regex/Leaf.cppm
    regex/Optional.cppm
    regex/Union.cppm
    regex/RegexNodeVisitor.cppm
    generator/nfa/NFANode.cppm
    generator/nfa/NFAEdge.cppm
    generator/nfa/NFA.cppm
    generator/thompson-construction/ThompsonConstructionVisitor.cppm
    generator/nfa-dfa-conversion/StateSet.cppm
    generator/nfa-dfa-conversion/eClosure.cppm
    generator/nfa-dfa-conversion/PowerSetConstruction.cppm
)