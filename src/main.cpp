#include <iostream>

#include "core/states.hpp"
#include "pathing/plotting.hpp"

int main() {
    std::cout << "hello" << std::endl;

    PreparationState state;

    state.tick();

    return 0;
}