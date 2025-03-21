#include "cv/aggregator.hpp"

#include "utilities/constants.hpp"
#include "utilities/lockptr.hpp"
#include "utilities/locks.hpp"
#include "utilities/logging.hpp"

CVAggregator::CVAggregator(Pipeline&& p) : pipeline(std::move(p)) {
    this->num_worker_threads = 0;
    this->results = std::make_shared<CVResults>();
}

CVAggregator::~CVAggregator() {}

LockPtr<CVResults> CVAggregator::getResults() {
    return LockPtr<CVResults>(this->results, &this->mut);
}

void CVAggregator::runPipeline(const ImageData& image) {
    Lock lock(this->mut);

    if (this->num_worker_threads >= MAX_CV_PIPELINES) {
        // If we have too many running workers, just queue the new image
        LOG_F(WARNING, "Too many CVAggregator workers (%d). Pushing to overflow queue...",
              this->num_worker_threads);
        this->overflow_queue.push(image);
        LOG_F(WARNING, "Overflow queue size is now %ld", this->overflow_queue.size());
        return;
    }

    static int thread_counter = 0;
    ++this->num_worker_threads;
    std::thread worker_thread(&CVAggregator::worker, this, image, ++thread_counter);
    worker_thread.detach();  // We don’t need to join in the caller
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
        Lock lock(this->mut);
        LOG_F(INFO, "CVAggregator worker #%d terminating. Active threads: %d -> %d", thread_num,
              this->num_worker_threads, this->num_worker_threads - 1);
        --this->num_worker_threads;
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