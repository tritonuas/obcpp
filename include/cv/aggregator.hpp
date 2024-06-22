#ifndef INCLUDE_CV_AGGREGATOR_HPP_
#define INCLUDE_CV_AGGREGATOR_HPP_

#include <mutex>
#include <memory>
#include <vector>
#include <future>
#include <queue>
#include <functional>
#include <unordered_map>

#include "cv/utilities.hpp"
#include "cv/pipeline.hpp"
#include "utilities/constants.hpp"
#include "utilities/lockptr.hpp"

struct CVResults {
    std::vector<DetectedTarget> detected_targets;
    // mapping from bottle -> index into detected_targets 
    // (optional is none if we don't have a match yet)
    std::unordered_map<BottleDropIndex, std::optional<size_t>> matches;
};



class CVAggregator {
 public:
    explicit CVAggregator(Pipeline&& p);

    ~CVAggregator();

    void runPipeline(const ImageData& image);

    LockPtr<CVResults> getResults();

 private:
    void worker(ImageData image, int thread_num);

    Pipeline pipeline;

    std::mutex mut;
    int num_worker_threads;

    // For when too many pipelines are active at the same time
    std::queue<ImageData> overflow_queue;

    std::shared_ptr<CVResults> results;
};

#endif  // INCLUDE_CV_AGGREGATOR_HPP_
