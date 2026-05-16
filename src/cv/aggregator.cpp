#include "cv/aggregator.hpp"

#include <exception>

#include "utilities/constants.hpp"
#include "utilities/lockptr.hpp"
#include "utilities/locks.hpp"
#include "utilities/logging.hpp"

CVAggregator::CVAggregator(Pipeline&& p) : pipeline(std::move(p)) {
    this->num_worker_threads.store(0);
    this->accepting_images.store(true);
    this->results = std::make_shared<CVResults>();
    this->matched_results = std::make_shared<MatchedResults>();
    this->cv_record = std::make_shared<std::map<int, IdentifiedTarget>>();
    AirdropTarget dummy;  // Create one dummy template

    // Configure the coordinate part of the dummy
    GPSCoord* coord_in_dummy = dummy.mutable_coordinate();  // Let dummy own its coordinate
    coord_in_dummy->set_altitude(0.0);
    coord_in_dummy->set_latitude(0.0);
    coord_in_dummy->set_longitude(0.0);

    // Now assign copies, each with its specific index
    dummy.set_index(AirdropType::Water);
    this->matched_results->matched_airdrop[AirdropType::Water] = dummy;

    dummy.set_index(AirdropType::Beacon);
    this->matched_results->matched_airdrop[AirdropType::Beacon] = dummy;
}

CVAggregator::~CVAggregator() { this->terminate(); }

LockPtr<CVResults> CVAggregator::getResults() {
    return LockPtr<CVResults>(this->results, &this->mut);
}

LockPtr<MatchedResults> CVAggregator::getMatchedResults() {
    return LockPtr<MatchedResults>(this->matched_results, &this->mut);
}

LockPtr<std::map<int, IdentifiedTarget>> CVAggregator::getCVRecord() {
    return LockPtr<std::map<int, IdentifiedTarget>>(this->cv_record, &this->cv_record_mut);
}
void CVAggregator::updateRecords(std::vector<IdentifiedTarget>& new_values) {
    LockPtr<std::map<int, IdentifiedTarget>> records = this->getCVRecord();
    for (IdentifiedTarget id : new_values) {
        if (records.data->contains(id.run_id())) {
            records.data->at(id.run_id()).CopyFrom(id);
        } else {
            LOG_F(WARNING, "Tried to modify with an ID not inside CVRecord. ID: (%d)", id.run_id());
        }
    }
}
void CVAggregator::runPipeline(const ImageData& image) {
    Lock lock(this->mut);

    if (!this->accepting_images.load()) {
        LOG_F(WARNING, "CVAggregator is not accepting new images. Dropping pipeline run.");
        return;
    }

    const int active_workers = this->num_worker_threads.load();
    if (active_workers >= MAX_CV_PIPELINES) {
        // If we have too many running workers, just queue the new image
        LOG_F(WARNING, "Too many CVAggregator workers (%d). Pushing to overflow queue...",
              active_workers);
        this->overflow_queue.push(image);
        LOG_F(WARNING, "Overflow queue size is now %ld", this->overflow_queue.size());
        return;
    }

    static int thread_counter = 0;
    this->num_worker_threads.fetch_add(1);
    try {
        this->worker_threads.emplace_back(&CVAggregator::worker, this, image, ++thread_counter);
    } catch (const std::exception& err) {
        this->num_worker_threads.fetch_sub(1);
        LOG_F(ERROR, "Failed to spawn CVAggregator worker: %s", err.what());
    }
}

void CVAggregator::terminate() {
    std::vector<std::thread> threads;
    {
        Lock lock(this->mut);
        this->accepting_images.store(false);
        const std::size_t dropped_images = this->overflow_queue.size();
        std::queue<ImageData> empty_queue;
        this->overflow_queue.swap(empty_queue);

        if (this->worker_threads.empty()) {
            return;
        }

        LOG_F(INFO, "Dropped %zu queued images. Waiting for %zu CVAggregator worker threads.",
              dropped_images, this->worker_threads.size());
        threads.swap(this->worker_threads);
    }

    for (std::thread& worker_thread : threads) {
        if (worker_thread.joinable()) {
            worker_thread.join();
        }
    }
}

static std::atomic<int> global_run_id{0};

void CVAggregator::worker(ImageData image, int thread_num) {
    loguru::set_thread_name(("cv worker " + std::to_string(thread_num)).c_str());
    LOG_F(INFO, "New CVAggregator worker #%d spawned.", thread_num);

    while (true) {
        // 1) Run the pipeline
        auto pipeline_results = this->pipeline.run(image);

        // 2) Build ONE run for all detections in that pipeline output
        AggregatedRun run;
        run.run_id = global_run_id.fetch_add(1);
        run.annotatedImage = pipeline_results.imageData.DATA.clone();
        run.bboxes.reserve(pipeline_results.targets.size());
        run.coords.reserve(pipeline_results.targets.size());

        for (auto& det : pipeline_results.targets) {
            run.bboxes.push_back(det.bbox);
            run.coords.push_back(det.coord);
        }

        {
            Lock lock(this->mut);
            this->results->runs.push_back(std::move(run));
        }

        // 3) If no more queued images, break
        {
            Lock lock(this->mut);
            if (this->overflow_queue.empty()) {
                break;
            }
            image = std::move(this->overflow_queue.front());
            this->overflow_queue.pop();
        }
    }

    // 4) Mark ourselves as finished
    {
        const int active_workers = this->num_worker_threads.fetch_sub(1);
        LOG_F(INFO, "CVAggregator worker #%d terminating. Active threads: %d -> %d", thread_num,
              active_workers, active_workers - 1);
    }
}

// Empty list (for endpoint)
std::vector<AggregatedRun> CVAggregator::popAllRuns() {
    Lock lock(this->mut);
    // Move out everything
    std::vector<AggregatedRun> out = std::move(this->results->runs);
    // Now results->runs is empty
    this->results->runs.clear();
    return out;
}
