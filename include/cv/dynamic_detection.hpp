#pragma once
// #ifndef INCLUDE_CV_DYNAMIC_DETECTION_HPP_
// #define INCLUDE_CV_DYNAMIC_DETECTION_HPP_

#include <mutex>
#include <queue>
#include <chrono>

/*
Design:
Producer-consumer thread communication model so that dynamic detection can run
asyncronously on its own to detect the forward camera for obstacles ~1Hz.

Steps:
1. Front camera takes pictures every second, puts into workqueue.
2. FastSAM runs on the queue.
3. dynamic_detection looks at the segmentation output and decides
   whether to enter emergency avoidance.
4. Tick control gets handed over to the dynamic avoidance state.

Threads:
Thread 1: Runs steps 1-3.
Thread 2: This is the main program execution, which should be polling every tick.
*/

// Output after an obstacle has been detected
struct Detection {
    time_t time;
    // TODO(Samir): is there a type for location of bounding box
    int x_pos;
    int y_pos;
};

class ObstacleDetectionQueue {
    public:
        void add();
        bool isEmpty();

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

// #endif  // INCLUDE_CV_DYNAMIC_DETECTION_HPP_
