#include <iostream>

#include <unordered_set>

int main() {
    std::cout << "hello" << std::endl;

    std::unordered_set<int> test;
    test.insert(5);

    if (test.contains(5)) {
        std::cout << "C++20" << std::endl;
    }

    return 0;
}