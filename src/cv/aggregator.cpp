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

void CVAggregator::worker(ImageData image, int thread_num) {
    loguru::set_thread_name(("cv worker " + std::to_string(thread_num)).c_str());
    LOG_F(INFO, "New CVAggregator worker #%d spawned.", thread_num);

    while (true) {
        // Run the pipeline
        auto pipeline_results = this->pipeline.run(image);

        // Accumulate the detected targets
        {
            Lock lock(this->mut);
            this->results->detected_targets.insert(this->results->detected_targets.end(),
                                                   pipeline_results.targets.begin(),
                                                   pipeline_results.targets.end());
        }

        // If no more queued images, we're done
        {
            Lock lock(this->mut);
            if (this->overflow_queue.empty()) {
                break;
            }
            // Otherwise, pop the next image and loop again
            image = std::move(this->overflow_queue.front());
            this->overflow_queue.pop();
        }
    }

    // Mark ourselves as finished
    {
        Lock lock(this->mut);
        LOG_F(INFO, "CVAggregator worker #%d terminating. Active threads: %d -> %d", thread_num,
              this->num_worker_threads, this->num_worker_threads - 1);
        --this->num_worker_threads;
    }
}
