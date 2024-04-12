#include "cv/dynamic_detection.hpp"
#include "camera/lucid.hpp"

ObstacleDetectionQueue::ObstacleDetectionQueue() {
    std::queue<Detection> queue;
    std::mutex mutex;
}

void ObstacleDetectionQueue::add(Detection detection) {
    Lock lock(this->mutex);
    queue.push(detection);
}

bool ObstacleDetectionQueue::isEmpty() {
    Lock lock(this->mutex);
    return queue.empty();
}

void ObstacleDetectionQueue::pruneQueue(int age = 15.0) {
    Lock lock(this->mutex);
    // delete items
    auto now = std::chrono::system_clock::now();
    while (now - queue.front().time > std::chrono::seconds(age)) {
        queue.pop();
    }
}

void ObstacleDetectionQueue::copyQueue(std::queue<Detection> queue) {
    // copies the data of the queue
    queue = this->queue;
}

DynamicDetection::DynamicDetection(CameraInterface* camera) {
    this->camera = camera;
} 

void* DynamicDetection::takeImage() {
    // TODO: this is pseudocode, dynamic images are not taken with the Lucid camera
    // Take a picture every second
    this->camera->startTakingPictures(1);
    return NULL;
}

void* DynamicDetection::segmentImage() {
    // TODO: stolen sample code from Igor
    // cv::Mat img = cv::imread(imgPath, cv::IMREAD_COLOR);
    // Saliency::salience(img);
    return NULL;
}

bool DynamicDetection::shouldEnterAvoidance() {
    // TODO: write a condition:
    // Object coming closer, multiple consistent detections
    // can do some housekeeping here to make sure queue
    // does not have stale (>15s) detections
    
    // do housekeeping: remove stale detections
    ObstacleDetectionQueue::pruneQueue();

    // TODO: check if the boxes are getting larger AND bigger than a threshold
    std::queue<Detection> queue;
    ObstacleDetectionQueue::copyQueue(queue);
    if (queue.back().x_pos > 0) {
        // TODO: this condition is not actually testing for size.
        // This is waiting to see how the model returns bboxes (bounding boxes)
        return true;
    }
    

    // TODO: I am rethinking this now and it does not make sense to 
    // have dynamic obstacle detection be part of it's own state.
    // The dynamic detection should be a part of a separate thread and
    // send a message to the Airdrop state to switch to the dynamic avoidance
    // state.
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

        std::this_thread::sleep_for(1ms);
    }
}
