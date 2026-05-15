#ifndef INCLUDE_CV_AGGREGATOR_HPP_
#define INCLUDE_CV_AGGREGATOR_HPP_

#include <atomic>
#include <cmath>
#include <filesystem>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
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
    // Integration-test constructor
    explicit CVAggregator(Pipeline&& p);

    // Mission constructor
    CVAggregator(Pipeline&& p, const std::string& image_dir, int sample_every_n_images,
                 int image_listener_poll_interval_ms, int image_listener_settle_time_ms);
    ~CVAggregator();

    // Spawn a thread to run the pipeline on the given imageData
    void runPipeline(const ImageData& image);

    // Watch a camera save directory and submit every nth saved image to the pipeline
    void startListening(const std::string& image_dir, int sample_every_n_images,
                        int image_listener_poll_interval_ms, int image_listener_settle_time_ms);

    // Stop watching the camera save directory
    void stopListening();

    // Stop accepting work, discard queued images, and wait for active workers to finish
    void terminate();

    // Lockable pointer to retrieve aggregator results
    LockPtr<CVResults> getResults();

    // Lockable pointer to retrieve matched results (after manual match)
    LockPtr<MatchedResults> getMatchedResults();

    // For the endpoint to reset the current list of structs
    std::vector<AggregatedRun> popAllRuns();

    // gets the record of all cv results
    LockPtr<std::map<int, IdentifiedTarget>> getCVRecord();

    void updateRecords(std::vector<IdentifiedTarget>& new_values);

 private:
    std::mutex cv_record_mut;

    std::shared_ptr<std::map<int, IdentifiedTarget>> cv_record;

    void worker(ImageData image, int thread_num);
    void listenForImages(std::filesystem::path image_dir, int sample_every_n_images,
                         int image_listener_poll_interval_ms, int image_listener_settle_time_ms);
    std::optional<ImageData> loadImageData(const std::filesystem::path& image_path) const;
    std::optional<ImageTelemetry> loadTelemetry(
        const std::filesystem::path& telemetry_path) const;

    Pipeline pipeline;

    std::mutex mut;
    std::atomic<int> num_worker_threads;
    std::atomic<bool> accepting_images;
    std::atomic<bool> listening_images;
    std::vector<std::thread> worker_threads;
    std::thread listener_thread;

    // For when too many pipelines are active at once
    std::queue<ImageData> overflow_queue;

    // Shared aggregator results
    std::shared_ptr<CVResults> results;

    // Shared matched results
    std::shared_ptr<MatchedResults> matched_results;
};

#endif  // INCLUDE_CV_AGGREGATOR_HPP_
