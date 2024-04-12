#pragma once
// #ifndef INCLUDE_CV_DYNAMIC_DETECTION_HPP_
// #define INCLUDE_CV_DYNAMIC_DETECTION_HPP_

#include <mutex>
#include <queue>
#include <chrono>
#include "utilities/constants.hpp"
#include "utilities/locks.hpp"
#include <thread>
#include "camera/interface.hpp"

using namespace std::chrono_literals;

/*
Design:
Producer-consumer thread communication model so that dynamic detection can run
asyncronously on its own to detect the forward camera for obstacles ~1Hz.

Steps:
1. Front camera takes pictures every second, puts into workqueue*.
2. FastSAM runs on the queue.
3. dynamic_detection looks at the segmentation output and decides
   whether to enter emergency avoidance.
4. Tick control gets handed over to the dynamic avoidance state.

(*) Not totally sure if I will end up putting a queue here. Note that
    if Thread 2 is using the Detection queue, it can lock out Thread 1's
    execution.

Threads:
Thread 1: Runs steps 1-3.
Thread 2: This is the main program execution, which should be polling every tick.
*/

// Output after an obstacle has been detected
struct Detection {
    std::chrono::time_point<std::chrono::system_clock> time;
    // TODO(Samir): is there a type for location of bounding box
    int x_pos;
    int y_pos;
};

class ObstacleDetectionQueue {
    public:
        ObstacleDetectionQueue();
        void add(Detection detection);
        bool isEmpty();
        // prune every image older than `age` seconds ago
        void pruneQueue(int age);
        // copies the queue so you can look at the data
        void copyQueue(std::queue<Detection> queue);

    private:
     // This is used to store all previous obstacles.
     // We need to make sure that a detection is not spurious
     // before engaging in emergency evasion.
     std::queue<Detection> queue;
     // Note that Step 4 in "Steps" above only triggers on a boolean
     // condition. I am still making this Detection queue thread safe
     // because it can be useful to be able to access this in the main
     // program. For example, once emergency evasion is engaged it can
     // poll the queue to make sure there is nothing more to evade.
     std::mutex mutex;
};

class DynamicDetection {
    public:
        DynamicDetection(CameraInterface* camera);
    private:
        // generic camera to use to take pictures
        CameraInterface* camera;

        // Take a photo from front facing camera
        // TODO: this functionality and hardware has not been added
        void* takeImage();
        // Run FastSAM on image and add to detection queue
        void* segmentImage();
        // Look at queue and decide if avoidance is needed.
        bool shouldEnterAvoidance();
        // Main loop. Has similar behavior of Tick in that it will
        // do scheduling of camera tasks every interval (1 second)
        void main();
};

// #endif  // INCLUDE_CV_DYNAMIC_DETECTION_HPP_
