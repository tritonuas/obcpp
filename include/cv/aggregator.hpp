#ifndef INCLUDE_CV_AGGREGATOR_HPP_
#define INCLUDE_CV_AGGREGATOR_HPP_

#include <cmath>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "cv/pipeline.hpp"
#include "cv/utilities.hpp"
#include "utilities/constants.hpp"
#include "utilities/lockptr.hpp"

struct CVResults {
    // All detected targets collected from any pipeline run
    std::vector<DetectedTarget> detected_targets;
};

class CVAggregator {
 public:
    explicit CVAggregator(Pipeline&& p);
    ~CVAggregator();

    // Spawn a thread to run the pipeline on the given imageData
    void runPipeline(const ImageData& image);

    // Lockable pointer to retrieve aggregator results
    LockPtr<CVResults> getResults();

 private:
    void worker(ImageData image, int thread_num);

    Pipeline pipeline;

    std::mutex mut;
    int num_worker_threads;

    // For when too many pipelines are active at once
    std::queue<ImageData> overflow_queue;

    // Shared aggregator results
    std::shared_ptr<CVResults> results;
};

#endif  // INCLUDE_CV_AGGREGATOR_HPP_
