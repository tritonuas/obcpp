#include "cv/dynamic_detection.hpp"

ObstacleDetectionQueue::ObstacleDetectionQueue() {
    std::queue<Detection> queue;
    std::mutex mutex;
}

void ObstacleDetectionQueue::add() {
    Lock lock(this->mutex);
    // queue.push();
}

bool ObstacleDetectionQueue::isEmpty() {
    Lock lock(this->mutex);
    return queue.empty();
}

DynamicDetection::DynamicDetection() {}

void* DynamicDetection::takeImage() {
    // TODO:
    return NULL;
}

void* DynamicDetection::segmentImage() {
    // TODO:
    return NULL;
}

bool DynamicDetection::shouldEnterAvoidance() {
    // TODO: write a condition:
    // Object coming closer, multiple consistent detections
    // can do some housekeeping here to make sure queue
    // does not have stale (>15s) detections
    return false;
}

void DynamicDetection::main() {
    while (true) {
        takeImage();
        segmentImage();
        if (shouldEnterAvoidance()) {
            // TODO: enter avoidance
            return;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
