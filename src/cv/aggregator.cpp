#include "cv/aggregator.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <exception>
#include <fstream>
#include <set>
#include <string>
#include <utility>

#include "nlohmann/json.hpp"
#include "utilities/constants.hpp"
#include "utilities/lockptr.hpp"
#include "utilities/locks.hpp"
#include "utilities/logging.hpp"

namespace {

bool isSupportedImageFile(const std::filesystem::path& path) {
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return ext == ".jpg" || ext == ".jpeg" || ext == ".png";
}

std::vector<std::filesystem::path> listImageFiles(const std::filesystem::path& image_dir) {
    std::vector<std::filesystem::path> image_paths;

    for (const auto& entry : std::filesystem::directory_iterator(image_dir)) {
        if (entry.is_regular_file() && isSupportedImageFile(entry.path())) {
            image_paths.push_back(entry.path());
        }
    }

    return image_paths;
}

}  // namespace

CVAggregator::CVAggregator(
    Pipeline&& p, const std::string& image_dir, int sample_every_n_images,
    int image_listener_poll_interval_ms, int image_listener_settle_time_ms)
    : pipeline(std::move(p)) {
    this->num_worker_threads.store(0);
    this->accepting_images.store(true);
    this->listening_images.store(false);
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

    this->startListening(image_dir, sample_every_n_images, image_listener_poll_interval_ms,
                         image_listener_settle_time_ms);
}

CVAggregator::~CVAggregator() { this->terminate(); }

void CVAggregator::startListening(
    const std::string& image_dir, int sample_every_n_images,
    int image_listener_poll_interval_ms, int image_listener_settle_time_ms) {
    if (sample_every_n_images <= 0) {
        LOG_F(ERROR, "cv.sample_every_n_images must be greater than 0.");
    }

    if (image_listener_poll_interval_ms <= 0) {
        LOG_F(ERROR, "cv.image_listener_poll_interval_ms must be greater than 0.");
    }

    if (image_listener_settle_time_ms <= 0) {
        LOG_F(ERROR, "cv.image_listener_settle_time_ms must be greater than 0.");
    }

    std::filesystem::path image_path = image_dir;
    if (!std::filesystem::is_directory(image_path)) {
        LOG_F(ERROR, "CV image directory does not exist: %s", image_path.string().c_str());
        return;
    }

    if (this->listening_images.exchange(true)) {
        LOG_F(WARNING, "CVAggregator is already listening for camera images.");
        return;
    }

    this->listener_thread = std::thread(&CVAggregator::listenForImages, this, image_path,
                                        sample_every_n_images,
                                        image_listener_poll_interval_ms,
                                        image_listener_settle_time_ms);
}

void CVAggregator::stopListening() {
    if (!this->listening_images.exchange(false)) {
        return;
    }

    if (this->listener_thread.joinable()) {
        this->listener_thread.join();
    }
}

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
    this->stopListening();

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

std::optional<ImageTelemetry> CVAggregator::loadTelemetry(
    const std::filesystem::path& telemetry_path) const {
    std::ifstream telemetry_file(telemetry_path);
    if (!telemetry_file.is_open()) {
        return std::nullopt;
    }

    try {
        nlohmann::json telemetry_json = nlohmann::json::parse(telemetry_file, nullptr, true, true);
        return ImageTelemetry{
            .latitude_deg = telemetry_json.at("latitude_deg").get<double>(),
            .longitude_deg = telemetry_json.at("longitude_deg").get<double>(),
            .altitude_agl_m = telemetry_json.at("altitude_agl_m").get<double>(),
            .airspeed_m_s = telemetry_json.at("airspeed_m_s").get<double>(),
            .heading_deg = telemetry_json.at("heading_deg").get<double>(),
            .yaw_deg = telemetry_json.at("yaw_deg").get<double>(),
            .pitch_deg = telemetry_json.at("pitch_deg").get<double>(),
            .roll_deg = telemetry_json.at("roll_deg").get<double>(),
        };
    } catch (const std::exception& err) {
        LOG_F(WARNING, "Failed to load image telemetry from %s: %s",
              telemetry_path.string().c_str(), err.what());
        return std::nullopt;
    }
}

std::optional<ImageData> CVAggregator::loadImageData(
    const std::filesystem::path& image_path) const {
    cv::Mat image = cv::imread(image_path.string(), cv::IMREAD_COLOR);
    if (image.empty()) {
        LOG_F(WARNING, "Failed to load camera image %s", image_path.string().c_str());
        return std::nullopt;
    }

    std::filesystem::path telemetry_path = image_path;
    telemetry_path.replace_extension(".json");
    std::optional<ImageTelemetry> telemetry = this->loadTelemetry(telemetry_path);
    if (!telemetry.has_value()) {
        LOG_F(WARNING, "Skipping camera image %s because telemetry could not be loaded from %s.",
              image_path.string().c_str(), telemetry_path.string().c_str());
        return std::nullopt;
    }

    return ImageData{
        .DATA = std::move(image),
        .TIMESTAMP = 0,
        .TELEMETRY = telemetry,
    };
}

void CVAggregator::listenForImages(
    std::filesystem::path image_dir, int sample_every_n_images,
    int image_listener_poll_interval_ms, int image_listener_settle_time_ms) {
    loguru::set_thread_name("cv listener");
    const auto poll_interval = std::chrono::milliseconds(image_listener_poll_interval_ms);
    const auto settle_time = std::chrono::milliseconds(image_listener_settle_time_ms);

    std::set<std::filesystem::path> seen_images;
    for (const auto& image_path : listImageFiles(image_dir)) {
        seen_images.insert(image_path);
    }

    std::map<std::filesystem::path, std::chrono::steady_clock::time_point> pending_images;
    std::size_t image_count = 0;

    LOG_F(INFO,
          "CVAggregator listening for images in %s. Sampling every %d image(s), polling every %d "
          "ms, settling for %d ms.",
          image_dir.string().c_str(), sample_every_n_images, image_listener_poll_interval_ms,
          image_listener_settle_time_ms);

    while (this->listening_images.load()) {
        const auto now = std::chrono::steady_clock::now();

        for (const auto& image_path : listImageFiles(image_dir)) {
            if (seen_images.contains(image_path) || pending_images.contains(image_path)) {
                continue;
            }
            pending_images.insert({image_path, now});
        }

        for (auto iter = pending_images.begin(); iter != pending_images.end();) {
            if (!this->listening_images.load()) {
                break;
            }

            const auto& image_path = iter->first;
            if (now - iter->second < settle_time) {
                ++iter;
                continue;
            }

            std::optional<ImageData> image = this->loadImageData(image_path);
            if (!image.has_value()) {
                seen_images.insert(image_path);
                iter = pending_images.erase(iter);
                continue;
            }

            seen_images.insert(image_path);
            iter = pending_images.erase(iter);

            ++image_count;
            if (image_count % static_cast<std::size_t>(sample_every_n_images) != 0) {
                continue;
            }

            this->runPipeline(image.value());
        }

        std::this_thread::sleep_for(poll_interval);
    }

    LOG_F(INFO, "CVAggregator image listener stopped.");
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
