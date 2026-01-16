#ifndef INCLUDE_CV_PIPELINE_HPP_
#define INCLUDE_CV_PIPELINE_HPP_

#include <cmath>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "camera/interface.hpp"
#include "cv/localization.hpp"
#include "cv/preprocess.hpp"
#include "cv/sam3.hpp"
#include "cv/utilities.hpp"
#include "protos/obc.pb.h"
#include <opencv2/opencv.hpp>

// Processed image holds all predictions made concerning a given image.
struct PipelineResults {
    PipelineResults(ImageData imageData, std::vector<DetectedTarget> targets)
        : imageData(std::move(imageData)), targets(std::move(targets)) {}

    // This holds the final annotated image in imageData.DATA
    ImageData imageData;

    // This is the list of SAM3 + localized detections
    std::vector<DetectedTarget> targets;
};

struct PipelineParams {
    // SAM3 model and tokenizer paths are optional; when absent, no CV models will be loaded.
    explicit PipelineParams(std::optional<std::string> modelPath,
                            std::optional<std::string> tokenizerPath,
                            std::vector<std::string> prompts = {"person"},
                            std::string outputPath = "", bool do_preprocess = true,
                            double min_confidence = 0.20, double nms_iou = 0.2)
        : modelPath{std::move(modelPath)},
          tokenizerPath{std::move(tokenizerPath)},
          prompts{std::move(prompts)},
          outputPath(std::move(outputPath)),
          do_preprocess(do_preprocess),
          min_confidence(min_confidence),
          nms_iou(nms_iou) {}

    std::optional<std::string> modelPath;
    std::optional<std::string> tokenizerPath;
    std::vector<std::string> prompts;
    std::string outputPath;
    bool do_preprocess;
    double min_confidence;  // Minimum confidence threshold for detections
    double nms_iou;         // IoU threshold for Non-Max Suppression
};

// Forward declare Ort::Env
namespace Ort {
class Env;
}

// Pipeline handles SAM3 + localization (and now optional preprocessing and output saving)
class Pipeline {
 public:
    explicit Pipeline(const PipelineParams& p);
    PipelineResults run(const ImageData& imageData);

 private:
    std::unique_ptr<SAM3> sam3Detector;
    // ECEFLocalization ecefLocalizer;
    GSDLocalization gsdLocalizer;
    bool do_preprocess;                 // Flag to enable/disable preprocessing
    Preprocess preprocessor;            // Preprocess utility instance
    std::string outputPath;             // New member to hold output image path
    std::vector<std::string> prompts_;  // Prompts used for SAM3 detection
    double min_confidence_;             // Minimum confidence threshold
    double nms_iou_;                    // NMS IoU threshold
};

#endif  // INCLUDE_CV_PIPELINE_HPP_
