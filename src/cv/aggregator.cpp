#include "cv/aggregator.hpp"
#include "utilities/locks.hpp"
#include "utilities/constants.hpp"
#include "utilities/logging.hpp"
#include "utilities/lockptr.hpp"

CVAggregator::CVAggregator(Pipeline&& p): pipeline{p} {
    this->num_worker_threads = 0;
    this->results = std::make_shared<CVResults>();
    // set each bottle to be initially unmatched
    this->results->matches[BottleDropIndex::A] = {};
    this->results->matches[BottleDropIndex::B] = {};
    this->results->matches[BottleDropIndex::C] = {};
    this->results->matches[BottleDropIndex::D] = {};
    this->results->matches[BottleDropIndex::E] = {};
}

CVAggregator::~CVAggregator() {}

LockPtr<CVResults> CVAggregator::getResults() {
    return LockPtr<CVResults>(this->results, &this->mut);
}

void CVAggregator::runPipeline(const ImageData& image) {
    Lock lock(this->mut);

    if (this->num_worker_threads > MAX_CV_PIPELINES) {
        LOG_F(WARNING, "Tried to spawn more than %ld CVAggregator workers at one time.",
            MAX_CV_PIPELINES);
        this->overflow_queue.push(image);
        LOG_F(WARNING, "CVAggregator overflow queue at size %ld", this->overflow_queue.size());
        return;
    }

    static int thread_num = 0;
    this->num_worker_threads++;
    std::thread worker(&CVAggregator::worker, this, image, ++thread_num);
    worker.detach();
}

void CVAggregator::worker(ImageData image, int thread_num) {
    loguru::set_thread_name(("cv worker " + std::to_string(thread_num)).c_str());
    LOG_F(INFO, "New CVAggregator worker spawned.");

    while (true) {
        auto results = this->pipeline.run(image);

        Lock lock(this->mut);
        for (auto curr_target : results.targets) {
            // this->detected_targets.size() will end up being the index of the
            // newly inserted target, once we insert it after the if/else
            size_t detected_target_index = this->results->detected_targets.size();

            std::optional<size_t> curr_match_idx = this->results->matches[curr_target.likely_bottle];
            if (!curr_match_idx.has_value()) {
                LOG_F(INFO, "Made first match between target %ld and bottle %d",
                    detected_target_index, curr_target.likely_bottle);

                this->results->matches[curr_target.likely_bottle] = detected_target_index;
            } else {
                auto curr_match = this->results->detected_targets[curr_match_idx.value()];

                if (curr_match.match_distance > curr_target.match_distance) {
                    LOG_F(INFO,
                        "Swapping match on bottle %d from target %ld -> %ld (distance %f -> %f)",
                        static_cast<int>(curr_match.likely_bottle),
                        this->results->matches[curr_match.likely_bottle].value_or(0),
                        detected_target_index,
                        curr_match.match_distance,
                        curr_target.match_distance);

                    this->results->matches[curr_match.likely_bottle] = detected_target_index;
                }
            }

            // do this last so we can still meaningfully access local var curr_target
            // before this line
            this->results->detected_targets.push_back(std::move(curr_target));
        }

        if (this->overflow_queue.empty()) {
            break;
        }

        image = std::move(this->overflow_queue.front());
        this->overflow_queue.pop();
    }

    LOG_F(INFO, "CVAggregator worker terminating.");
    this->num_worker_threads--;
}
