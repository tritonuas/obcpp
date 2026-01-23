#include "cv/aggregator.hpp"

#include "utilities/constants.hpp"
#include "utilities/lockptr.hpp"
#include "utilities/locks.hpp"
#include "utilities/logging.hpp"

// Global atomic for unique run IDs
static std::atomic<int> global_run_id{0};

CVAggregator::CVAggregator(Pipeline&& pipeline) : pipeline(std::move(pipeline)) {
    this->results = std::make_shared<CVResults>();
    this->matched_results = std::make_shared<MatchedResults>();
    this->cv_record = std::make_shared<std::map<int, IdentifiedTarget>>();
    this->stop_processing = false;

    // Initialize dummy targets
    AirdropTarget dummy;
    GPSCoord* coord_in_dummy = dummy.mutable_coordinate();
    coord_in_dummy->set_altitude(0.0);
    coord_in_dummy->set_latitude(0.0);
    coord_in_dummy->set_longitude(0.0);

    dummy.set_index(AirdropType::Water);
    this->matched_results->matched_airdrop[AirdropType::Water] = dummy;
    dummy.set_index(AirdropType::Beacon);
    this->matched_results->matched_airdrop[AirdropType::Beacon] = dummy;

    // Launch fixed number of workers immediately
    for (int i = 0; i < MAX_CV_PIPELINES; ++i) {
        workers.emplace_back(&CVAggregator::workerLoop, this, i);
    }
    LOG_F(INFO, "CVAggregator initialized with %d worker threads.", MAX_CV_PIPELINES);
}

CVAggregator::~CVAggregator() {
    // Clean shutdown prevents "Use-After-Free" crash
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop_processing = true;
    }
    condition.notify_all();  // Wake up all threads

    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();  // Wait for them to finish using the pipeline
        }
    }
    LOG_F(INFO, "CVAggregator shutdown complete.");
}

void CVAggregator::runPipeline(const ImageData& image) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        // Optional: Drop frames if queue is too full to prevent lag
        if (tasks.size() > 20) {
            LOG_F(WARNING, "CV Queue full, dropping image.");
            return;
        }
        tasks.push(image);
    }
    condition.notify_one();  // Wake up one available worker
}

void CVAggregator::workerLoop(int thread_id) {
    loguru::set_thread_name(("cv_worker_" + std::to_string(thread_id)).c_str());
    LOG_F(INFO, "Worker %d started", thread_id);

    while (true) {
        ImageData current_image;

        // Wait for work or shutdown signal
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [this] { return this->stop_processing || !this->tasks.empty(); });

            if (this->stop_processing && this->tasks.empty()) {
                break;  // Exit the loop
            }

            current_image = std::move(this->tasks.front());
            this->tasks.pop();
        }

        // --- Run Pipeline (Thread Safe Part) ---
        // Note: We access 'pipeline' concurrently.
        // Ensure Pipeline::run doesn't modify internal state that isn't thread-local.
        auto pipeline_results = this->pipeline.run(current_image);

        AggregatedRun run;
        run.run_id = global_run_id.fetch_add(1);
        run.annotatedImage = pipeline_results.imageData.DATA.clone();
        run.bboxes.reserve(pipeline_results.targets.size());
        run.coords.reserve(pipeline_results.targets.size());

        for (auto& det : pipeline_results.targets) {
            run.bboxes.push_back(det.bbox);
            run.coords.push_back(det.coord);
        }

        // --- Store Results ---
        {
            Lock lock(this->results_mut);
            this->results->runs.push_back(std::move(run));
        }
    }
    LOG_F(INFO, "Worker %d exiting", thread_id);
}

LockPtr<CVResults> CVAggregator::getResults() {
    return LockPtr<CVResults>(this->results, &this->results_mut);
}

LockPtr<MatchedResults> CVAggregator::getMatchedResults() {
    return LockPtr<MatchedResults>(this->matched_results, &this->results_mut);
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

std::vector<AggregatedRun> CVAggregator::popAllRuns() {
    Lock lock(this->results_mut);
    std::vector<AggregatedRun> out = std::move(this->results->runs);
    this->results->runs.clear();
    return out;
}