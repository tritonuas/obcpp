#include "cv/aggregator.hpp"
#include "utilities/locks.hpp"
#include "utilities/constants.hpp"
#include "utilities/logging.hpp"

CVAggregator::CVAggregator() {
    this->num_worker_threads = 0;
    for (size_t& index : this->matches) {
        index = -1;
    }
}

CVAggregator::~CVAggregator() {

}

void CVAggregator::runPipeline(ImageData image) {
    Lock lock(this->mut);

    if (this->num_worker_threads > MAX_CV_PIPELINES) {
        LOG_F(WARNING, "Tried to spawn more than %d CV pipelines at one time.", MAX_CV_PIPELINES);
        this->overflow_queue.push(image);
        return;
    }

    std::thread
}