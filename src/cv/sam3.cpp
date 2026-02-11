#include "cv/sam3.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>

#include "utilities/logging.hpp"
#include <nlohmann/json.hpp>

// Sigmoid helper: logits -> probabilities
inline float sigmoid(float x) { return 1.0f / (1.0f + std::exp(-x)); }

SAM3::SAM3(const std::string& encoderPath, const std::string& decoderPath,
           const std::string& tokenizerPath, double min_confidence, double nms_iou) {
    try {
        min_confidence_ = min_confidence;
        nms_iou_ = nms_iou;
        // 1. Initialize Environment
        env_ = std::make_shared<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "SAM3_Detector");

        // 2. Session Options
        sessionOptions_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        sessionOptions_.SetIntraOpNumThreads(4);

        // 3. Load Encoder Model
        encoderSession_ =
            std::make_unique<Ort::Session>(*env_, encoderPath.c_str(), sessionOptions_);
        LOG_F(INFO, "Successfully loaded SAM3 Encoder from: %s", encoderPath.c_str());

        // 4. Load Decoder Model
        decoderSession_ =
            std::make_unique<Ort::Session>(*env_, decoderPath.c_str(), sessionOptions_);
        LOG_F(INFO, "Successfully loaded SAM3 Decoder from: %s", decoderPath.c_str());

        // 5. Load Tokenizer
        loadTokenizer(tokenizerPath);

    } catch (const std::exception& e) {
        LOG_F(ERROR, "Error loading SAM3 model: %s", e.what());
    }
}

float computeIoU(const Detection& a, const Detection& b) {
    float x1 = std::max(a.x1, b.x1);
    float y1 = std::max(a.y1, b.y1);
    float x2 = std::min(a.x2, b.x2);
    float y2 = std::min(a.y2, b.y2);

    float interArea = std::max(0.0f, x2 - x1) * std::max(0.0f, y2 - y1);
    float areaA = (a.x2 - a.x1) * (a.y2 - a.y1);
    float areaB = (b.x2 - b.x1) * (b.y2 - b.y1);

    return interArea / (areaA + areaB - interArea + 1e-6);
}

std::vector<Detection> SAM3::detect(cv::Mat& image, const std::vector<std::string>& prompts) {
    std::vector<Detection> results;
    if (!encoderSession_ || !decoderSession_) {
        LOG_F(WARNING, "Encoder or decoder session not initialized");
        return results;
    }

    // --- 1. PREPROCESS ---
    // Resizes to 1008x1008 (Squashing)
    std::vector<float> pixelValues = preprocess(image);
    std::vector<int64_t> inputShape = {1, 3, 1008, 1008};

    auto memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

    // --- 2. ENCODE IMAGE (once for all prompts) ---
    const char* encoderInputNames[] = {"pixel_values"};
    const char* encoderOutputNames[] = {"feat0", "feat1", "feat2", "feat3",
                                        "pos0",  "pos1",  "pos2",  "pos3"};

    std::vector<Ort::Value> encoderInputTensors;
    encoderInputTensors.push_back(Ort::Value::CreateTensor<float>(
        memoryInfo, pixelValues.data(), pixelValues.size(), inputShape.data(), inputShape.size()));

    std::vector<Ort::Value> encoderOutputs;
    try {
        encoderOutputs = encoderSession_->Run(Ort::RunOptions{nullptr}, encoderInputNames,
                                              encoderInputTensors.data(), 1, encoderOutputNames, 8);
    } catch (const Ort::Exception& e) {
        LOG_F(ERROR, "ONNX Encoder Inference Error: %s", e.what());
        return results;
    }

    LOG_F(INFO, "Encoder produced %zu output tensors", encoderOutputs.size());

    // --- 3. DECODE PER PROMPT ---
    // Decoder only uses: feat0, feat1, feat2, pos2, input_ids (5 inputs)
    const char* decoderInputNames[] = {"feat0", "feat1", "feat2", "pos2", "input_ids"};
    const char* decoderOutputNames[] = {"boxes", "masks", "scores", "presence_score"};
    const int decoderInputIndices[] = {0, 1, 2, 6};

    for (const auto& promptText : prompts) {
        std::vector<int64_t> inputIds = tokenize(promptText);
        std::vector<int64_t> tokenShape = {1, static_cast<int64_t>(inputIds.size())};

        // Build decoder input tensors: 4 encoder outputs + 1 token tensor
        std::vector<Ort::Value> decoderInputTensors;

        // Re-feed the required encoder output tensors
        for (int idx : decoderInputIndices) {
            auto tensorInfo = encoderOutputs[idx].GetTensorTypeAndShapeInfo();
            auto shape = tensorInfo.GetShape();
            size_t elemCount = 1;
            for (auto d : shape) elemCount *= d;

            float* data = encoderOutputs[idx].GetTensorMutableData<float>();
            decoderInputTensors.push_back(Ort::Value::CreateTensor<float>(
                memoryInfo, data, elemCount, shape.data(), shape.size()));
        }

        // Add the token input
        decoderInputTensors.push_back(Ort::Value::CreateTensor<int64_t>(
            memoryInfo, inputIds.data(), inputIds.size(), tokenShape.data(), tokenShape.size()));

        try {
            auto outputs =
                decoderSession_->Run(Ort::RunOptions{nullptr}, decoderInputNames,
                                     decoderInputTensors.data(), 5, decoderOutputNames, 4);

            // --- 4. PARSE OUTPUTS ---
            auto boxesInfo = outputs[0].GetTensorTypeAndShapeInfo();
            int numQueries = boxesInfo.GetShape()[1];

            float* rawBoxes = outputs[0].GetTensorMutableData<float>();
            float* rawScores = outputs[2].GetTensorMutableData<float>();

            std::vector<Detection> rawDetections;

            for (int i = 0; i < numQueries; i++) {
                float score = sigmoid(rawScores[i]);
                float finalConf = score;

                if (finalConf > min_confidence_) {
                    float x1_norm = rawBoxes[i * 4 + 0];
                    float y1_norm = rawBoxes[i * 4 + 1];
                    float x2_norm = rawBoxes[i * 4 + 2];
                    float y2_norm = rawBoxes[i * 4 + 3];

                    Detection det;
                    det.confidence = finalConf;
                    det.class_id = 0;
                    det.label = promptText;

                    det.x1 = x1_norm * image.cols;
                    det.y1 = y1_norm * image.rows;
                    det.x2 = x2_norm * image.cols;
                    det.y2 = y2_norm * image.rows;

                    det.x1 = std::max(0.0f, std::min((float)image.cols, det.x1));
                    det.y1 = std::max(0.0f, std::min((float)image.rows, det.y1));
                    det.x2 = std::max(0.0f, std::min((float)image.cols, det.x2));
                    det.y2 = std::max(0.0f, std::min((float)image.rows, det.y2));

                    if (det.x2 > det.x1 && det.y2 > det.y1) {
                        rawDetections.push_back(det);
                    }
                }
            }

            // --- 5. NON-MAXIMUM SUPPRESSION (NMS) ---
            std::sort(
                rawDetections.begin(), rawDetections.end(),
                [](const Detection& a, const Detection& b) { return a.confidence > b.confidence; });

            for (const auto& det : rawDetections) {
                bool overlap = false;
                for (const auto& keep : results) {
                    if (computeIoU(det, keep) >= nms_iou_) {
                        overlap = true;
                        break;
                    }
                }
                if (!overlap) {
                    results.push_back(det);
                }
            }

            LOG_F(INFO, "Detected %zu objects for prompt '%s'", results.size(), promptText.c_str());

        } catch (const Ort::Exception& e) {
            LOG_F(ERROR, "ONNX Decoder Inference Error: %s", e.what());
        }
    }

    // Draw detections onto the input image for visualization
    for (const auto& det : results) {
        cv::rectangle(image, cv::Point(static_cast<int>(det.x1), static_cast<int>(det.y1)),
                      cv::Point(static_cast<int>(det.x2), static_cast<int>(det.y2)),
                      cv::Scalar(0, 255, 0), 2);
        char text[64];
        std::snprintf(text, sizeof(text), "%s %.2f", det.label.c_str(), det.confidence);
        cv::putText(image, text,
                    cv::Point(static_cast<int>(det.x1), std::max(0, static_cast<int>(det.y1) - 5)),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 2);
    }

    return results;
}

std::vector<float> SAM3::preprocess(const cv::Mat& img) {
    cv::Mat rgb_img;
    // 1. Convert BGR -> RGB
    cv::cvtColor(img, rgb_img, cv::COLOR_BGR2RGB);

    cv::Mat resized;
    // 2. Simple Resize (Squash) to 1008x1008
    // We do NOT preserve aspect ratio here, matching "what we had before"
    cv::resize(rgb_img, resized, cv::Size(1008, 1008));

    // 3. Convert to float32 [0, 1]
    resized.convertTo(resized, CV_32FC3, 1.0f / 255.0f);

    // 4. Normalize (ImageNet mean/std)
    cv::Scalar mean(0.485, 0.456, 0.406);
    cv::Scalar std(0.229, 0.224, 0.225);
    cv::subtract(resized, mean, resized);
    cv::divide(resized, std, resized);

    // 5. HWC -> CHW Layout
    std::vector<float> output;
    output.reserve(3 * 1008 * 1008);
    std::vector<cv::Mat> channels(3);
    cv::split(resized, channels);
    for (auto& c : channels) {
        output.insert(output.end(), (float*)c.datastart, (float*)c.dataend);
    }
    return output;
}

// ... [The Tokenizer functions remain unchanged] ...
void SAM3::loadTokenizer(const std::string& path) {
    try {
        std::ifstream f(path);
        if (!f.is_open()) {
            LOG_F(ERROR, "Cannot open tokenizer: %s", path.c_str());
            return;
        }
        nlohmann::json j = nlohmann::json::parse(f);

        auto vocabMap = j.contains("model") ? j["model"]["vocab"] : j;
        for (auto& el : vocabMap.items()) {
            vocab_[el.key()] = el.value();
        }
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Tokenizer load error: %s", e.what());
    }
}

std::vector<int64_t> SAM3::tokenize(const std::string& text) {
    std::vector<int64_t> ids;
    ids.reserve(contextLength_);
    ids.push_back(startToken_);

    std::istringstream iss(text);
    std::string word;
    while (iss >> word) {
        if (ids.size() + 1 >= contextLength_) break;
        std::string withSuffix = word + "</w>";
        if (vocab_.count(withSuffix)) {
            ids.push_back(vocab_[withSuffix]);
        } else if (vocab_.count(word)) {
            ids.push_back(vocab_[word]);
        }
    }
    ids.push_back(endToken_);
    while (ids.size() < (size_t)contextLength_) {
        ids.push_back(padToken_);
    }
    return ids;
}