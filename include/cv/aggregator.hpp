#ifndef INCLUDE_CV_AGGREGATOR_HPP_
#define INCLUDE_CV_AGGREGATOR_HPP_

#include <atomic>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "cv/pipeline.hpp"
#include "cv/utilities.hpp"
#include "protos/obc.pb.h"
#include "utilities/constants.hpp"
#include "utilities/lockptr.hpp"

struct AggregatedRun {
    int run_id;
    cv::Mat annotatedImage;
    std::vector<Bbox> bboxes;
    std::vector<GPSCoord> coords;
};

struct CVResults {
    std::vector<AggregatedRun> runs;
};

struct MatchedResults {
    std::unordered_map<AirdropType, AirdropTarget> matched_airdrop;
};

class CVAggregator {
 public:
    explicit CVAggregator(Pipeline&& pipeline);
    ~CVAggregator();

    // Adds the image to the work queue (non-blocking)
    void runPipeline(const ImageData& image);

    LockPtr<CVResults> getResults();
    LockPtr<MatchedResults> getMatchedResults();
    std::vector<AggregatedRun> popAllRuns();
    LockPtr<std::map<int, IdentifiedTarget>> getCVRecord();
    void updateRecords(std::vector<IdentifiedTarget>& new_values);

 private:
    // The worker loop that runs inside each thread
    void workerLoop(int thread_id);

    Pipeline pipeline;

    // Thread Pool Synchronization
    std::vector<std::thread> workers;   // Fixed pool of threads
    std::queue<ImageData> tasks;        // The work queue
    std::mutex queue_mutex;             // Protects the queue
    std::condition_variable condition;  // Wakes up workers
    bool stop_processing;               // Flag to signal shutdown

    // Results storage
    std::mutex cv_record_mut;
    std::shared_ptr<std::map<int, IdentifiedTarget>> cv_record;

    std::mutex results_mut;  // Renamed from 'mut' for clarity
    std::shared_ptr<CVResults> results;
    std::shared_ptr<MatchedResults> matched_results;
};

#endif  // INCLUDE_CV_AGGREGATOR_HPP_