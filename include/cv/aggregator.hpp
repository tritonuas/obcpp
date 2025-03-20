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

// NEW: small struct to store the big image, bounding box, and coords
struct AggregatedItem {
    cv::Mat bigImage;  // Annotated image from the pipeline
    Bbox bbox;         // The detection bounding box
    GPSCoord coord;    // Localized GPS coordinate
};

struct CVResults {
    // We now store a list of AggregatedItems instead of just DetectedTarget
    std::vector<AggregatedItem> items;
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
