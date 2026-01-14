# DSLC
## Description
**DSLC** (Domain Specific Language Compiler) is a tool designed to simplify the creation of Domain Specific Languages (**DSLs**). This educational project implements the compiler design principles outlined in *Compilers: Principles, Techniques, and Tools* by Alfred V. Aho, Monica S. Lam, Ravi Sethi, and Jeffrey D. Ullman.

## Features
- **Grammar Specification:** Define custom DSL grammars using an intuitive syntax.
- **Lexer and Parser Generation:** Automatically generate a scanner and parser (LR1) based on your grammar.
- **Semantic Analysis:** Incorporate semantic rules to enforce language constraints.
- **Code Generation:** Generate target code from DSL specifications.

## Prerequisites
- **C++ Compiler:** clang 19 (Though any other C++ compiler which supports the C++20 feature set should also work)
- **CMake:** 3.31 or higher
- **Git:** For cloning the repository
- **ninja:** make does not support modules (yet)
- **gtest:** (Optional) If -DBUILD_TESTS=ON you will require the dev/devel dependency of gtest

## Building the project (in source)
```
git clone git@github.com:tjpal/dslc.git
cd dslc

mkdir build
cd build

cmake -G "Ninja" ..
ninja -j $(nproc)
```

## Usage
To be defined