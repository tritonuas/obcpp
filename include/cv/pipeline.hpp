#ifndef INCLUDE_CV_PIPELINE_HPP_
#define INCLUDE_CV_PIPELINE_HPP_

#include <cmath>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <opencv2/opencv.hpp>

#include "camera/interface.hpp"
#include "cv/localization.hpp"
#include "cv/owlv2.hpp"
#include "cv/preprocess.hpp"
#include "cv/utilities.hpp"
#include "protos/obc.pb.h"

// Processed image holds all predictions made concerning a given image.
struct PipelineResults {
    PipelineResults(ImageData imageData, std::vector<DetectedTarget> targets)
        : imageData(std::move(imageData)), targets(std::move(targets)) {}

    // This holds the final annotated image in imageData.DATA
    ImageData imageData;

    // This is the list of YOLO + localized detections
    std::vector<DetectedTarget> targets;
};

struct PipelineParams {
    // Added outputPath parameter with default empty string
    explicit PipelineParams(std::string yoloModelPath, std::string outputPath = "",
                            bool do_preprocess = true)
        : yoloModelPath{std::move(yoloModelPath)},
          outputPath(std::move(outputPath)),
          do_preprocess(do_preprocess) {}

    std::string yoloModelPath;
    bool do_preprocess;
    std::string outputPath;
};

// Pipeline handles YOLO + localization (and now optional preprocessing and output saving)
class Pipeline {
 public:
    explicit Pipeline(const PipelineParams& p);
    PipelineResults run(const ImageData& imageData);

 private:
    std::unique_ptr<OWLv2> yoloDetector;
    // ECEFLocalization ecefLocalizer;
    GSDLocalization gsdLocalizer;
    bool do_preprocess;       // Flag to enable/disable preprocessing
    Preprocess preprocessor;  // Preprocess utility instance
    std::string outputPath;   // New member to hold output image path
};

#endif  // INCLUDE_CV_PIPELINE_HPP_
