#include "ticks/mission_prep.hpp"
#include <httplib.h>

#include <memory>
#include <string>

#include "utilities/logging.hpp"
#include "core/mission_state.hpp"
#include "ticks/path_gen.hpp"
#include "ticks/ids.hpp"
#include "utilities/datatypes.hpp"

MissionPrepTick::MissionPrepTick(std::shared_ptr<MissionState> state)
    :Tick(state, TickID::MissionPrep) {}

std::chrono::milliseconds MissionPrepTick::getWait() const {
    return MISSION_PREP_TICK_WAIT;
}

Tick* MissionPrepTick::tick() {
    if (this->state->mission_params.getCachedMission().has_value()) {
        LOG_F(INFO, "Valid mission configuration detected");

        std::array<Bottle, NUM_AIRDROP_BOTTLES> bottles_to_drop;

        // Get the airdrop bottles from the mission parameters
        // and copy into an std::array of the same size. Annoying conversion because
        // mission parameters and cv aggregator expect them in different data
        // structures. Could fix by making them both take a vector / both take an
        // array of size NUM_AIRDROP_BOTTLES
        std::copy_n(state->mission_params.getAirdropBottles().begin(),
            NUM_AIRDROP_BOTTLES, bottles_to_drop.begin());

        std::string matching_model_dir = this->state->config.cv.matching_model_dir;
        std::string segmentation_model_dir = this->state->config.cv.segmentation_model_dir;
        std::string saliency_model_dir = this->state->config.cv.saliency_model_dir;

        LOG_F(INFO, "Instantiating CV Aggregator with the following models:");
        LOG_F(INFO, "Matching Model: %s", matching_model_dir.c_str());
        LOG_F(INFO, "Segmentation Model: %s", segmentation_model_dir.c_str());
        LOG_F(INFO, "Saliency Model: %s", saliency_model_dir.c_str());

        // Make a CVAggregator instance and set it in the state
        this->state->setCV(std::make_shared<CVAggregator>(Pipeline(
            PipelineParams(bottles_to_drop,
                this->generateReferenceImages(bottles_to_drop),
                matching_model_dir,
                segmentation_model_dir,
                saliency_model_dir))));

        return new PathGenTick(this->state);
    } else {
        return nullptr;
    }
}


std::vector<std::pair<cv::Mat, BottleDropIndex>> 
    MissionPrepTick::generateReferenceImages(std::array<Bottle, NUM_AIRDROP_BOTTLES> competitionObjectives) {

    std::vector<std::pair<cv::Mat, BottleDropIndex>> ref_imgs;

    int curr_bottle_idx = BottleDropIndex::A;
    for (const auto& bottle : competitionObjectives) {
        httplib::Client client(this->state->config.cv.not_stolen_addr, this->state->config.cv.not_stolen_port);
        auto res = client.Get(this->getNotStolenRoute(bottle));
        if (res->status != 200) {
            LOG_F(ERROR, "Got invalid response from not-stolen: %s", res->body.c_str());
            continue;
        }
        std::vector<uint8_t> vectordata(res->body.begin(),res->body.end());
        cv::Mat data_mat(vectordata, true);
        cv::Mat ref_img(cv::imdecode(data_mat,1)); //put 0 if you want greyscale

        ref_imgs.push_back({ref_img, (BottleDropIndex)curr_bottle_idx});
        curr_bottle_idx++;
    }
    return ref_imgs;
}

std::string MissionPrepTick::getNotStolenRoute(const Bottle& target) {
    std::string char_type = target.alphanumeric();
    std::string char_color = ODLCColorToString(target.alphanumericcolor());

    std::string shape_type = ODLCShapeToString(target.shape());
    std::string shape_color = ODLCColorToString(target.shapecolor());

    return std::string("/generate?shape_type=") + shape_type + 
        std::string("&shape_color=") + shape_color +
        std::string("&char_type=") + char_type +
        std::string("&char_color=") + char_color;
}
