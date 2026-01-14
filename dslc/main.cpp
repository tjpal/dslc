import Scanner.Regex;
import Scanner.NFA;

#include <iostream>

int main() {
    auto start = scanner::NFANode();
    auto middle = scanner::NFANode();
    auto end = scanner::NFANode();

    start.addEdge(scanner::NFAEdge(end.getNodeID(), { 'a' } ));
    start.addEdge(scanner::NFAEdge(middle.getNodeID(), { 'b' } ));
    middle.addEdge(scanner::NFAEdge(end.getNodeID(), { 'c' } ));

    auto nfa = scanner::NFA(start, { start, end, middle }, end);

    std::cout << "NFA Start Node ID: " << nfa.getStartNodeID() << "\n";
    std::cout << "NFA Accepting Node ID: " << nfa.getAcceptingNodeID() << "\n";
    std::cout << "NFA middle Node ID: " << middle.getNodeID() << "\n";

    // All nodes of nfa
    std::cout << "NFA Nodes:\n";
    for (const auto& node : nfa.getNodes()) {
        std::cout << node.getNodeID() << "\n";
    }

    int i = 0;
    i = i * 2;
}