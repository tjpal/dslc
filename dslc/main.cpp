import Scanner.Regex;
import Scanner.NFA;

#include <memory>

int main() {
    auto start = scanner::NFANode();
    auto end = scanner::NFANode();

    start.addEdge(scanner::NFAEdge(end.getNodeID(), { 'a' } ));

    auto nfa = scanner::NFA(start, {std::move(start), std::move(end)}, end);

    int i = 0;
    i = i * 2;
}