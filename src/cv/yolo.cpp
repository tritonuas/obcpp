#include "cv/yolo.hpp"

// For simplicity, we are not doing advanced error handling
// Make sure to catch and handle exceptions in production code

YOLO::YOLO(const std::string& modelPath, float confThreshold, int inputWidth, int inputHeight)
    : env_(ORT_LOGGING_LEVEL_WARNING, "yolo-inference"),
      session_(nullptr),
      confThreshold_(confThreshold),
      inputWidth_(inputWidth),
      inputHeight_(inputHeight) {
    // Initialize session options if needed
    sessionOptions_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

    // // Create the session
    // session_ = new Ort::Session(env_, modelPath.c_str(), sessionOptions_);

    try {
        session_ = new Ort::Session(env_, modelPath.c_str(), sessionOptions_);
        std::cout << "Model loaded successfully.\n";
    } catch (const Ort::Exception& e) {
        std::cerr << "Error loading model: " << e.what() << std::endl;
    }

    // Create an allocator
    Ort::AllocatorWithDefaultOptions allocator;

    // Get input names
    size_t numInputNodes = session_->GetInputCount();
    inputNames_.resize(numInputNodes);
    for (size_t i = 0; i < numInputNodes; i++) {
        auto name = session_->GetInputNameAllocated(i, allocator);
        inputNames_[i] = std::string(name.get());
    }

    // Get output names
    size_t numOutputNodes = session_->GetOutputCount();
    outputNames_.resize(numOutputNodes);
    for (size_t i = 0; i < numOutputNodes; i++) {
        auto name =
            session_->GetOutputNameAllocated(i, allocator);  // Correct function for output nodes
        outputNames_[i] = std::string(name.get());
    }
}

YOLO::~YOLO() {
    if (session_) {
        delete session_;
        session_ = nullptr;
    }
}

std::vector<float> YOLO::preprocess(const cv::Mat& image) {
    // Compute letterbox scale
    float r =
        std::min((float)inputWidth_ / (float)image.cols, (float)inputHeight_ / (float)image.rows);
    int unpadW = std::round(image.cols * r);
    int unpadH = std::round(image.rows * r);
    int dw = inputWidth_ - unpadW;   // total horizontal padding
    int dh = inputHeight_ - unpadH;  // total vertical padding

    // We'll store these so we can "un-letterbox" later:
    scale_ = r;
    padLeft_ = dw / 2;  // per-side left padding
    padTop_ = dh / 2;   // per-side top padding

    // Then call letterbox(...). This returns a 640×640 image, but we
    // know how it was scaled & padded:
    cv::Mat resized = letterbox(image, inputWidth_, inputHeight_, cv::Scalar(114, 114, 114));

    // [Optional] Convert BGR->RGB if your model is truly trained on RGB:
    cv::cvtColor(resized, resized, cv::COLOR_BGR2RGB);

    // Convert to float [0..1]
    resized.convertTo(resized, CV_32F, 1.0f / 255.0f);

    // HWC -> CHW
    std::vector<float> inputData(inputWidth_ * inputHeight_ * 3);
    size_t index = 0;
    for (int c = 0; c < 3; c++) {
        for (int h = 0; h < inputHeight_; h++) {
            for (int w = 0; w < inputWidth_; w++) {
                inputData[index++] = resized.at<cv::Vec3f>(h, w)[c];
            }
        }
    }

    return inputData;
}

std::vector<Detection> YOLO::detect(const cv::Mat& image) {
    // Preprocess the image
    std::vector<float> inputTensorValues = preprocess(image);

    // Create input tensor object from data values
    std::vector<int64_t> inputShape = {1, 3, static_cast<int64_t>(inputHeight_),
                                       static_cast<int64_t>(inputWidth_)};
    size_t inputTensorSize = 1 * 3 * inputHeight_ * inputWidth_;
    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value inputTensor =
        Ort::Value::CreateTensor<float>(memoryInfo, inputTensorValues.data(), inputTensorSize,
                                        inputShape.data(), inputShape.size());

    // Convert std::vector<std::string> to std::vector<const char*>
    std::vector<const char*> inputNamesCStr;
    for (const auto& name : inputNames_) {
        inputNamesCStr.push_back(name.c_str());
    }

    std::vector<const char*> outputNamesCStr;
    for (const auto& name : outputNames_) {
        outputNamesCStr.push_back(name.c_str());
    }

    // Run inference using the C-style string arrays
    auto outputTensors =
        session_->Run(Ort::RunOptions{nullptr}, inputNamesCStr.data(), &inputTensor, 1,
                      outputNamesCStr.data(), outputNamesCStr.size());

    // Process the output as before...
    float* outputData = outputTensors[0].GetTensorMutableData<float>();

    // The rest of your detection code...
    std::vector<Detection> detections;
    size_t numDetections = 300;
    size_t elementsPerDetection = 6;

    auto shapeInfo = outputTensors[0].GetTensorTypeAndShapeInfo();
    auto shape = shapeInfo.GetShape();
    std::cout << "Output shape = [ ";
    for (auto s : shape) std::cout << s << " ";
    std::cout << "]" << std::endl;

    // float scaleWidth = static_cast<float>(image.cols) / static_cast<float>(inputWidth_);
    // float scaleHeight = static_cast<float>(image.rows) / static_cast<float>(inputHeight_);

    for (size_t i = 0; i < numDetections; i++) {
        size_t offset = i * elementsPerDetection;
        // "Un-letterbox" each detection
        //    1) subtract padLeft_ / padTop_
        //    2) divide by scale_
        // This maps from 640×640 letterboxed coords back to original image coords.

        float x1 = outputData[offset + 0];
        float y1 = outputData[offset + 1];
        float x2 = outputData[offset + 2];
        float y2 = outputData[offset + 3];
        float conf = outputData[offset + 4];
        float cls = outputData[offset + 5];

        if (conf >= confThreshold_) {
            // Remove the letterbox offset
            x1 -= padLeft_;
            y1 -= padTop_;
            x2 -= padLeft_;
            y2 -= padTop_;

            // Scale back up to original image
            x1 /= scale_;
            y1 /= scale_;
            x2 /= scale_;
            y2 /= scale_;

            // Optionally clamp coords to image boundaries:
            x1 = std::max(0.f, std::min(x1, (float)image.cols - 1));
            y1 = std::max(0.f, std::min(y1, (float)image.rows - 1));
            x2 = std::max(0.f, std::min(x2, (float)image.cols - 1));
            y2 = std::max(0.f, std::min(y2, (float)image.rows - 1));

            Detection det;
            det.x1 = x1;
            det.y1 = y1;
            det.x2 = x2;
            det.y2 = y2;
            det.confidence = conf;
            det.class_id = static_cast<int>(cls);
            detections.push_back(det);
        }
    }

    return detections;
}

cv::Mat YOLO::letterbox(const cv::Mat& src, int newWidth, int newHeight, const cv::Scalar& color) {
    // Scale = min of width-ratio or height-ratio
    float r = std::min((float)newWidth / (float)src.cols, (float)newHeight / (float)src.rows);

    // Compute padded (unscaled) width/height
    int unpadW = std::round(src.cols * r);
    int unpadH = std::round(src.rows * r);

    // Resize to the new scaled dimensions
    cv::Mat dst;
    cv::resize(src, dst, cv::Size(unpadW, unpadH));

    // Compute padding to match requested newWidth/newHeight
    int dw = newWidth - unpadW;   // total padding width
    int dh = newHeight - unpadH;  // total padding height

    // Divide padding into 2 sides (left/right, top/bottom)
    int top = std::round(dh / 2.0f);
    int bottom = dh - top;
    int left = std::round(dw / 2.0f);
    int right = dw - left;

    // Apply border
    cv::copyMakeBorder(dst, dst, top, bottom, left, right, cv::BORDER_CONSTANT, color);

    return dst;
}
