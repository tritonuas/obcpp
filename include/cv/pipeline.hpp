#ifndef INCLUDE_CV_PIPELINE_HPP_
#define INCLUDE_CV_PIPELINE_HPP_

#include <cmath>
#include <opencv2/opencv.hpp>
#include <string>
#include <utility>
#include <vector>

#include "camera/interface.hpp"
#include "cv/localization.hpp"
#include "cv/utilities.hpp"
#include "cv/yolo.hpp"
#include "protos/obc.pb.h"

// Processed image holds all predictions made concerning a given image.
struct PipelineResults {
    PipelineResults(ImageData imageData, std::vector<DetectedTarget> targets)
        : imageData{imageData}, targets{targets} {}

    ImageData imageData;
    std::vector<DetectedTarget> targets;
};

struct PipelineParams {
    // Adjust as needed; you can rename `saliencyModelPath` to `yoloModelPath` for clarity
    PipelineParams(std::string yoloModelPath) : yoloModelPath{yoloModelPath} {}

    // If you no longer need reference images or anything else,
    // you can remove them entirely or keep them as placeholders.
    std::string yoloModelPath;
};

// Pipeline handles YOLO + localization
class Pipeline {
 public:
    explicit Pipeline(const PipelineParams& p);
    PipelineResults run(const ImageData& imageData);

 private:
    std::unique_ptr<YOLO> yoloDetector;
    ECEFLocalization ecefLocalizer;
    GSDLocalization gsdLocalizer;
};

#endif  // INCLUDE_CV_PIPELINE_HPP_
