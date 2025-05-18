#include "ticks/mission_prep.hpp"

#include <httplib.h>

#include <memory>
#include <string>

#include "core/mission_state.hpp"
#include "ticks/ids.hpp"
#include "ticks/path_gen.hpp"
#include "utilities/datatypes.hpp"
#include "utilities/logging.hpp"

MissionPrepTick::MissionPrepTick(std::shared_ptr<MissionState> state)
    : Tick(state, TickID::MissionPrep) {}

std::chrono::milliseconds MissionPrepTick::getWait() const { return MISSION_PREP_TICK_WAIT; }

Tick* MissionPrepTick::tick() {
    if (this->state->mission_params.getCachedMission().has_value()) {
        LOG_F(INFO, "Valid mission configuration detected");

        std::array<Airdrop, NUM_AIRDROPS> airdrops_to_drop;

        // Get the airdrop bottles from the mission parameters
        // and copy into an std::array of the same size. Annoying conversion because
        // mission parameters and cv aggregator expect them in different data
        // structures. Could fix by making them both take a vector / both take an
        // array of size NUM_AIRDROP_BOTTLES
        std::copy_n(state->mission_params.getAirdrops().begin(), NUM_AIRDROPS,
                    airdrops_to_drop.begin());

        std::string yolo_model_dir = this->state->config.cv.yolo_model_dir;

        LOG_F(INFO, "Instantiating CV Aggregator with the following models:");
        LOG_F(INFO, "Yolo Model: %s", yolo_model_dir.c_str());

        // Make a CVAggregator instance and set it in the state
        this->state->setCV(
            std::make_shared<CVAggregator>(Pipeline(PipelineParams(yolo_model_dir))));

        return new PathGenTick(this->state);
    } else {
        return nullptr;
    }
}

std::vector<std::pair<cv::Mat, AirdropIndex>> MissionPrepTick::generateReferenceImages(
    std::array<Airdrop, NUM_AIRDROPS> competitionObjectives) {
    std::vector<std::pair<cv::Mat, AirdropIndex>> ref_imgs;

    // Default is Kaz cuz we don't have Undefined anymore
    int curr_airdrop_idx = AirdropIndex::Kaz;
    for (const auto& airdrop : competitionObjectives) {
        curr_airdrop_idx++;

        // don't generate reference images for mannikin since matching model doesn't
        // match mannikins (handled by saliency)
        // if (bottle.ismannikin()) {
        //     continue;
        // }

        httplib::Client client(this->state->config.cv.not_stolen_addr,
                               this->state->config.cv.not_stolen_port);
        auto res = client.Get(this->getNotStolenRoute(airdrop));
        // connection failed
        if (!res) {
            LOG_F(ERROR, "Failed to send request to not-stolen at %s:%u. Reason: %s",
                  this->state->config.cv.not_stolen_addr.c_str(),
                  this->state->config.cv.not_stolen_port, httplib::to_string(res.error()).c_str());
            return {};
        }

        if (res->status != 200) {
            LOG_F(ERROR, "Got invalid response from not-stolen: %s", res->body.c_str());
            continue;
        }
        std::vector<uint8_t> vectordata(res->body.begin(), res->body.end());
        cv::Mat data_mat(vectordata, true);
        cv::Mat ref_img(cv::imdecode(data_mat, 1));  // put 0 if you want greyscale

        ref_imgs.push_back({ref_img, (AirdropIndex)curr_airdrop_idx});
    }
    return ref_imgs;
}

std::string MissionPrepTick::getNotStolenRoute(const Airdrop& target) {
    return std::string("/generate");
}
