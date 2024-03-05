#include <random>
#include <unordered_set>
#include <utility>

// TODO add srand(0) when benchmarking
double random(double min, double max) {
    return min + static_cast<double>(rand()) / RAND_MAX * (max - min);
}

int randomInt(int min, int max) { return min + rand() % (max - min + 1); }

// double random(double min, double max) {
//     static std::random_device rd;
//     static std::mt19937 generator(rd());
//     std::uniform_real_distribution<> distro(min, max);
//     return distro(generator);
// }

// int randomInt(int min, int max) {
//     static std::random_device rd;
//     static std::mt19937 generator(rd());
//     std::uniform_int_distribution<> distro(min, max);
//     return distro(generator);
// }
