#include <random>
#include <unordered_set>
#include <utility>
// TODO add srand(0) when benchmarking
double random(double min, double max) {
    if (max < min) {
        std::swap(max, min);
    }

    double randomValue = min + static_cast<double>(rand()) / RAND_MAX * (max - min);
    return randomValue;
}

int randomInt(int min, int max) {
    if (max < min) {
        std::swap(max, min);
    }

    int randomValue = min + rand() % (max - min + 1);
    return randomValue;
}

// double random(double min, double max) {
//     if (max < min) {
//         std::swap(max, min);
//     }

//     static std::random_device rd;
//     static std::mt19937 generator(rd());
//     std::uniform_real_distribution<> distro(min, max);
//     return distro(generator);
// }

// int randomInt(int min, int max) {
//     if (max < min) {
//         std::swap(max, min);
//     }

//     static std::random_device rd;
//     static std::mt19937 generator(rd());
//     std::uniform_int_distribution<> distro(min, max);
//     return distro(generator);
// }
