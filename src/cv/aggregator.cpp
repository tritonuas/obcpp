#include "cv/aggregator.hpp"
#include "utilities/locks.hpp"
#include "utilities/constants.hpp"
#include "utilities/logging.hpp"

CVAggregator::CVAggregator(Pipeline p): pipeline{p} {
    this->num_worker_threads = 0;
    for (size_t& index : this->matches) {
        index = -1; // mark each bottle as not having a match yet
    }
}

CVAggregator::~CVAggregator() {

}

void CVAggregator::runPipeline(const ImageData& image) {
    Lock lock(this->mut);

    if (this->num_worker_threads > MAX_CV_PIPELINES) {
        LOG_F(WARNING, "Tried to spawn more than %d CV pipelines at one time.", MAX_CV_PIPELINES);
        this->overflow_queue.push(image);
        return;
    }

    this->num_worker_threads++;
    std::thread worker(CVAggregator::worker, this, image);
    worker.detach();
}

void CVAggregator::worker(ImageData image) {
    while (true) {
        auto results = this->pipeline.run(image);
        
        Lock lock(this->mut);
        for (auto curr_target : results.targets) {
            auto curr_match = this->bottleToMatchedTarget(curr_target.likely_bottle);

            // this->detected_targets.size() will end up being the index of the
            // newly inserted target, once we insert it after this if/else
            if (!curr_match.has_value()) {
                // this bottle hasn't been matched yet
                this->matches[static_cast<int>(curr_target.likely_bottle) - 1] =
                    this->detected_targets.size();
            } else if (curr_match->match_distance > curr_target.match_distance) {
                // this bottle has been matched, but we just found a better match so we
                // should overwrite it
                this->matches[static_cast<int>(curr_match->likely_bottle) - 1] =
                    this->detected_targets.size();
            }

            // do this last so we can still meaningfully access local var curr_target
            // before this line
            this->detected_targets.push_back(std::move(curr_target));
        }

        if (this->overflow_queue.empty()) {
            break;
        } else {
            image = std::move(this->overflow_queue.front());
            this->overflow_queue.pop();
        }
    }

    this->num_worker_threads--;
}

inline std::optional<const DetectedTarget&> CVAggregator::bottleToMatchedTarget(BottleDropIndex index) {
    size_t index = this->matches[static_cast<int>(index) - 1];
    if (index < 0) {
        return {};
    }

    return this->detected_targets[index];
}