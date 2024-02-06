#include <random>
#include <unordered_set>
#include <utility>

double random(double min, double max) {
    if (max < min) {
        std::swap(max, min);
    }
    static std::random_device rd;
    static std::mt19937 generator(rd());
    std::uniform_real_distribution<> distro(min, max);
    return distro(generator);
}

int randomInt(int min, int max) {
    if (max < min) {
        std::swap(max, min);
    }
    static std::random_device rd;
    static std::mt19937 generator(rd());
    std::uniform_int_distribution<> distro(min, max);
    return distro(generator);
}
