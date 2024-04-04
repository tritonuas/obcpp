#ifndef INCLUDE_CV_AGGREGATOR_HPP_
#define INCLUDE_CV_AGGREGATOR_HPP_

#include "cv/utilities.hpp"
#include "cv/pipeline.hpp"
#include "utilities/constants.hpp"

#include <mutex>
#include <vector>
#include <future>
#include <queue>
#include <array>

class CVAggregator {
 public:
    CVAggregator(Pipeline p);
    ~CVAggregator();

    void runPipeline(const ImageData& image);

 private:
    void worker(ImageData image);

    Pipeline pipeline;

    std::mutex mut;
    int num_worker_threads;

    // For when too many pipelines are active at the same time
    std::queue<ImageData> overflow_queue;


    std::vector<DetectedTarget> detected_targets;
    // matches[0] = 5 => detected_targets[5] is matched with bottle A
    std::array<size_t, NUM_AIRDROP_BOTTLES> matches;

    // Helper functions to interface with detected_targets and matches vector/array
    inline std::optional<const DetectedTarget&> bottleToMatchedTarget(BottleDropIndex index);
};

#endif  // INCLUDE_CV_AGGREGATOR_HPP_