#include "camera/lucid.hpp"

#include <chrono>
#include <thread>
#include <optional>

#include <loguru.hpp>

#include "utilities/locks.hpp"

LucidCamera::LucidCamera() {

}

LucidCamera::~LucidCamera() {

}

void LucidCamera::connect() {
    WriteLock lock(this->arenaSystemLock)
    pSystem = Arena::OpenSystem();
}

bool LucidCamera::isConnected();

void LucidCamera::startTakingPictures(std::chrono::seconds interval) override;
void LucidCamera::stopTakingPictures() override;

std::optional<ImageData> LucidCamera::getLatestImage() override;
std::queue<ImageData> LucidCamera::getAllImages() override;