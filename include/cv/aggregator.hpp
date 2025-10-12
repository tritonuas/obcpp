#ifndef INCLUDE_CV_AGGREGATOR_HPP_
#define INCLUDE_CV_AGGREGATOR_HPP_

#include <atomic>
#include <cmath>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

#include "cv/pipeline.hpp"
#include "cv/utilities.hpp"
#include "protos/obc.pb.h"
#include "utilities/constants.hpp"
#include "utilities/lockptr.hpp"

struct AggregatedRun {
    int run_id;                    // Unique integer ID for this pipeline run
    cv::Mat annotatedImage;        // The "big image" with bounding boxes drawn
    std::vector<Bbox> bboxes;      // All bounding boxes in that image
    std::vector<GPSCoord> coords;  // Matching lat-longs for each bounding box
};

struct CVResults {
    std::vector<AggregatedRun> runs;  // Each pipeline invocation => 1 run
};

struct MatchedResults {
    std::unordered_map<AirdropType, AirdropTarget> matched_airdrop;
};

class CVAggregator {
 public:
    explicit CVAggregator(Pipeline&& p);
    ~CVAggregator();

    // Spawn a thread to run the pipeline on the given imageData
    void runPipeline(const ImageData& image);

    // Lockable pointer to retrieve aggregator results
    LockPtr<CVResults> getResults();

    // Lockable pointer to retrieve matched results (after manual match)
    LockPtr<MatchedResults> getMatchedResults();

    // For the endpoint to reset the current list of structs
    std::vector<AggregatedRun> popAllRuns();

 private:
    void worker(ImageData image, int thread_num);

    Pipeline pipeline;

    std::mutex mut;
    int num_worker_threads;

    // For when too many pipelines are active at once
    std::queue<ImageData> overflow_queue;

    // Shared aggregator results
    std::shared_ptr<CVResults> results;

    // Shared matched results
    std::shared_ptr<MatchedResults> matched_results;
};

#endif  // INCLUDE_CV_AGGREGATOR_HPP_
