#include "cv/owlv2.hpp"

#include <atomic>
#include <cmath>
#include <optional>
#include <set>

using json = nlohmann::json;

namespace {
    // CLIP image preprocessing normalization constants
    // https://github.com/openai/CLIP/issues/20
    static const float MEAN[3] = {0.48145466f, 0.4578275f, 0.40821073f};
    static const float STD_[3] = {0.26862954f, 0.26130258f, 0.27577711f};

    inline float sigmoid(float x) { return 1.f / (1.f + std::exp(-x)); }
}  // namespace

OWLv2::OWLv2(const std::string &modelPath, int inputSize, float scoreThreshold,
             float nmsIouThreshold, int maxDetections, const std::string &tokensPath)
    : env_(ORT_LOGGING_LEVEL_WARNING, "owlv2-inference"),
      session_(nullptr),
      modelPath_(modelPath),
      tokensPath_(tokensPath.empty() ? std::string("/workspaces/obcpp/models/labels_tokenized.json")
                                     : tokensPath),
      inputSize_(inputSize),
      scoreThreshold_(scoreThreshold),
      nmsIouThreshold_(nmsIouThreshold),
      maxDetections_(maxDetections) {
    sessionOptions_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    try {
        session_ = new Ort::Session(env_, modelPath_.c_str(), sessionOptions_);
    } catch (const Ort::Exception &e) {
        std::cerr << "Error loading OWLv2 model: " << e.what() << std::endl;
        session_ = nullptr;
    }
}

OWLv2::~OWLv2() {
    if (session_) {
        delete session_;
        session_ = nullptr;
    }
}

cv::Mat OWLv2::letterboxRGB(const cv::Mat &rgb, int targetLong, int &outH, int &outW, float &scale,
                            int &padTop, int &padLeft) {
    int h = rgb.rows, w = rgb.cols;
    float r = static_cast<float>(targetLong) / static_cast<float>(std::max(h, w));
    int nh = static_cast<int>(std::round(h * r));
    int nw = static_cast<int>(std::round(w * r));
    outH = targetLong;
    outW = targetLong;
    scale = r;

    cv::Mat resized;
    cv::resize(rgb, resized, cv::Size(nw, nh), 0, 0, cv::INTER_LINEAR);

    cv::Mat padded(outH, outW, CV_8UC3, cv::Scalar(0, 0, 0));
    padTop = (outH - nh) / 2;
    padLeft = (outW - nw) / 2;
    resized.copyTo(padded(cv::Rect(padLeft, padTop, nw, nh)));
    return padded;
}

std::vector<float> OWLv2::hwcToChwNormalize(const cv::Mat &rgbPadded) const {
    const int H = rgbPadded.rows, W = rgbPadded.cols;
    std::vector<float> out(3 * H * W);
    for (int y = 0; y < H; ++y) {
        const uint8_t *row = rgbPadded.ptr<uint8_t>(y);
        for (int x = 0; x < W; ++x) {
            float r = row[3 * x + 0] / 255.0f;
            float g = row[3 * x + 1] / 255.0f;
            float b = row[3 * x + 2] / 255.0f;
            r = (r - MEAN[0]) / STD_[0];
            g = (g - MEAN[1]) / STD_[1];
            b = (b - MEAN[2]) / STD_[2];
            out[0 * H * W + y * W + x] = r;
            out[1 * H * W + y * W + x] = g;
            out[2 * H * W + y * W + x] = b;
        }
    }
    return out;
}

json OWLv2::loadTokensJson(const std::string &path) const {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("cannot open tokens json: " + path);
    json j;
    f >> j;
    return j;
}

bool OWLv2::resolveTokens(const std::string &explicitPath, std::vector<int64_t> &ids,
                          std::vector<int64_t> &mask, int &Q, int &L) const {
    std::string path = explicitPath;
    if (path.empty()) return false;
    json toks;
    try {
        toks = loadTokensJson(path);
    } catch (...) {
        return false;
    }

    auto input_ids = toks["input_ids"];            // expected [G,Q,L]
    auto attention_mask = toks["attention_mask"];  // [G,Q,L]

    int G = static_cast<int>(input_ids.size());
    if (G <= 0) return false;
    L = static_cast<int>(input_ids[0][0].size());
    int totalQ = 0;
    for (int g = 0; g < G; ++g) {
        if (static_cast<int>(input_ids[g].size()) == 0) return false;
        if (static_cast<int>(input_ids[g][0].size()) != L) return false;
        totalQ += static_cast<int>(input_ids[g].size());
    }

    ids.resize(static_cast<size_t>(totalQ) * L);
    mask.resize(static_cast<size_t>(totalQ) * L);
    int qoff = 0;
    for (int g = 0; g < G; ++g) {
        int Qg = static_cast<int>(input_ids[g].size());
        for (int q = 0; q < Qg; ++q) {
            for (int l = 0; l < L; ++l) {
                ids[static_cast<size_t>(qoff + q) * L + l] = input_ids[g][q][l].get<int>();
                mask[static_cast<size_t>(qoff + q) * L + l] = attention_mask[g][q][l].get<int>();
            }
        }
        qoff += Qg;
    }
    Q = totalQ;
    return true;
}

std::vector<int> OWLv2::nms(const std::vector<cv::Rect2f> &boxes, const std::vector<float> &scores,
                            float iouThresh, int maxKeep) const {
    std::vector<int> order(scores.size());
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int a, int b) { return scores[a] > scores[b]; });

    auto inter_area = [](const cv::Rect2f &a, const cv::Rect2f &b) {
        float x1 = std::max(a.x, b.x);
        float y1 = std::max(a.y, b.y);
        float x2 = std::min(a.x + a.width, b.x + b.width);
        float y2 = std::min(a.y + a.height, b.y + b.height);
        float w = std::max(0.f, x2 - x1);
        float h = std::max(0.f, y2 - y1);
        return w * h;
    };
    auto area_of = [](const cv::Rect2f &r) {
        return std::max(0.f, r.width) * std::max(0.f, r.height);
    };

    std::vector<int> keep;
    keep.reserve(order.size());
    for (int idx : order) {
        bool suppressed = false;
        for (int k : keep) {
            float inter = inter_area(boxes[idx], boxes[k]);
            float uni = area_of(boxes[idx]) + area_of(boxes[k]) - inter;
            float iou = (uni > 0.f) ? (inter / uni) : 0.f;
            if (iou > iouThresh) {
                suppressed = true;
                break;
            }
        }
        if (!suppressed) {
            keep.push_back(idx);
            if (static_cast<int>(keep.size()) >= maxKeep) break;
        }
    }
    return keep;
}

std::vector<Detection> OWLv2::detect(const cv::Mat &bgrImage) {
    std::vector<Detection> empty;
    if (!session_) return empty;
    if (bgrImage.empty()) return empty;

    cv::Mat rgb;
    cv::cvtColor(bgrImage, rgb, cv::COLOR_BGR2RGB);
    int procH, procW, padTop, padLeft;
    float scale;
    cv::Mat padded = letterboxRGB(rgb, inputSize_, procH, procW, scale, padTop, padLeft);
    lastScale_ = scale;
    lastPadTop_ = padTop;
    lastPadLeft_ = padLeft;

    std::vector<float> pixel_values = hwcToChwNormalize(padded);

    // Resolve tokens
    std::vector<int64_t> ids_buf, mask_buf;
    int Q = 0, L = 0;
    if (!resolveTokens(tokensPath_, ids_buf, mask_buf, Q, L)) {
        // No tokens provided; gracefully return no detections to keep pipeline behavior safe
        return empty;
    }

    // Prepare inputs
    Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::array<int64_t, 4> pix_shape = {1, 3, procH, procW};
    Ort::Value pix = Ort::Value::CreateTensor<float>(mi, pixel_values.data(), pixel_values.size(),
                                                     pix_shape.data(), pix_shape.size());
    std::array<int64_t, 2> tok_shape = {static_cast<int64_t>(Q), static_cast<int64_t>(L)};
    Ort::Value ids = Ort::Value::CreateTensor<int64_t>(mi, ids_buf.data(), ids_buf.size(),
                                                       tok_shape.data(), tok_shape.size());
    Ort::Value msk = Ort::Value::CreateTensor<int64_t>(mi, mask_buf.data(), mask_buf.size(),
                                                       tok_shape.data(), tok_shape.size());

    const char *inames[3] = {"pixel_values", "input_ids", "attention_mask"};

    // Discover outputs
    Ort::AllocatorWithDefaultOptions allocator;
    std::set<std::string> out_name_set;
    std::vector<std::string> out_names_all;
    const size_t out_count = session_->GetOutputCount();
    out_names_all.reserve(out_count);
    for (size_t i = 0; i < out_count; ++i) {
        auto name_alloc = session_->GetOutputNameAllocated(i, allocator);
        std::string name = name_alloc.get();
        out_name_set.insert(name);
        out_names_all.push_back(std::move(name));
    }
    auto pick_first_present =
        [&](const std::vector<std::string> &cands) -> std::optional<std::string> {
        for (const auto &c : cands)
            if (out_name_set.count(c)) return c;
        return std::nullopt;
    };
    std::string logits_name =
        pick_first_present({"logits", "pred_logits", "class_logits"}).value_or("");
    std::string boxes_name =
        pick_first_present({"pred_boxes", "boxes", "box_predictions"}).value_or("");
    std::optional<std::string> objectness_name =
        pick_first_present({"objectness_logits", "pred_obj_logits", "objectness", "obj_logits"});
    if (logits_name.empty() || boxes_name.empty()) {
        return empty;
    }

    std::vector<const char *> out_names_to_fetch;
    out_names_to_fetch.push_back(logits_name.c_str());
    out_names_to_fetch.push_back(boxes_name.c_str());
    const bool has_objectness = objectness_name.has_value();
    if (has_objectness) out_names_to_fetch.push_back(objectness_name->c_str());

    std::array<Ort::Value, 3> inputs = {std::move(pix), std::move(ids), std::move(msk)};
    auto outputs = session_->Run(Ort::RunOptions{nullptr}, inames, inputs.data(), 3,
                                 out_names_to_fetch.data(), out_names_to_fetch.size());

    auto logits_info = outputs[0].GetTensorTypeAndShapeInfo();
    auto boxes_info = outputs[1].GetTensorTypeAndShapeInfo();
    std::optional<Ort::TensorTypeAndShapeInfo> obj_info_opt;
    if (has_objectness) obj_info_opt = outputs[2].GetTensorTypeAndShapeInfo();

    auto ls = logits_info.GetShape();
    int C_from_logits, Q_from_logits;
    if (ls.size() == 3) {
        Q_from_logits = static_cast<int>(ls[1]);
        C_from_logits = static_cast<int>(ls[2]);
    } else {
        Q_from_logits = static_cast<int>(ls[0]);
        C_from_logits = static_cast<int>(ls[1]);
    }
    int C = C_from_logits;
    int Q_use = Q_from_logits;

    float *logits = outputs[0].GetTensorMutableData<float>();
    float *boxes = outputs[1].GetTensorMutableData<float>();

    std::vector<float> ones_objectness;
    float *objlog = nullptr;
    bool valid_obj = false;
    if (has_objectness) {
        auto s = obj_info_opt->GetShape();
        if ((s.size() == 1 && static_cast<int>(s[0]) == Q_use) ||
            (s.size() == 2 && static_cast<int>(s[0]) == Q_use && static_cast<int>(s[1]) == 1)) {
            valid_obj = true;
        }
    }
    if (has_objectness && valid_obj) {
        objlog = outputs[2].GetTensorMutableData<float>();
    } else {
        ones_objectness.assign(Q_use, 1.0f);
        objlog = ones_objectness.data();
    }

    // Postprocess into Detection list
    std::vector<cv::Rect2f> boxes_vec;
    boxes_vec.reserve(Q_use);
    std::vector<float> scores_vec;
    scores_vec.reserve(Q_use);

    for (int q = 0; q < Q_use; ++q) {
        const float *cls = logits + q * C;
        int best_c = 0;
        float best_p = 0.f;
        for (int c = 0; c < C; ++c) {
            float p = sigmoid(cls[c]);
            if (p > best_p) {
                best_p = p;
                best_c = c;
            }
        }
        float obj = sigmoid(objlog[q]);
        float score = obj * best_p;
        if (score < scoreThreshold_) continue;

        const float *bx = boxes + q * 4;
        float cx = bx[0], cy = bx[1], bw = bx[2], bh = bx[3];

        float px = (cx - bw / 2.f) * procW;
        float py = (cy - bh / 2.f) * procH;
        float px2 = (cx + bw / 2.f) * procW;
        float py2 = (cy + bh / 2.f) * procH;

        // unpad
        px -= padLeft;
        px2 -= padLeft;
        py -= padTop;
        py2 -= padTop;

        // unscale to original
        px /= scale;
        py /= scale;
        px2 /= scale;
        py2 /= scale;

        // clip
        px = std::max(0.f, std::min(px, static_cast<float>(bgrImage.cols - 1)));
        py = std::max(0.f, std::min(py, static_cast<float>(bgrImage.rows - 1)));
        px2 = std::max(0.f, std::min(px2, static_cast<float>(bgrImage.cols - 1)));
        py2 = std::max(0.f, std::min(py2, static_cast<float>(bgrImage.rows - 1)));

        boxes_vec.emplace_back(cv::Rect2f(cv::Point2f(px, py), cv::Point2f(px2, py2)));
        scores_vec.emplace_back(score);
    }

    // NMS
    std::vector<int> keep = nms(boxes_vec, scores_vec, nmsIouThreshold_, maxDetections_);

    std::vector<Detection> dets;
    dets.reserve(keep.size());
    for (int k : keep) {
        Detection d{};
        d.x1 = boxes_vec[k].x;
        d.y1 = boxes_vec[k].y;
        d.x2 = boxes_vec[k].x + boxes_vec[k].width;
        d.y2 = boxes_vec[k].y + boxes_vec[k].height;
        d.confidence = scores_vec[k];
        d.class_id = 0;  // class mapping not used downstream; keep 0
        dets.push_back(d);
    }
    return dets;
}

void OWLv2::drawAndPrintDetections(cv::Mat &image, const std::vector<Detection> &detections) {
    for (const auto &det : detections) {
        std::cout << "Detected conf: " << det.confidence << " box: [" << det.x1 << ", " << det.y1
                  << ", " << det.x2 << ", " << det.y2 << "]" << std::endl;
        cv::rectangle(image, cv::Point(static_cast<int>(det.x1), static_cast<int>(det.y1)),
                      cv::Point(static_cast<int>(det.x2), static_cast<int>(det.y2)),
                      cv::Scalar(0, 255, 0), 2);
        std::string label = "conf:" + std::to_string(det.confidence).substr(0, 4);
        cv::putText(image, label, cv::Point(static_cast<int>(det.x1), static_cast<int>(det.y1) - 5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
    }
}

void OWLv2::processAndSaveImage(const cv::Mat &image, const std::string &outputPath) {
    cv::Mat outputImage = image.clone();
    std::vector<Detection> detections = detect(outputImage);
    drawAndPrintDetections(outputImage, detections);

    static std::atomic<int> file_counter{0};
    int counter = file_counter.fetch_add(1);
    std::string uniqueOutputPath;
    size_t dotPos = outputPath.find_last_of('.');
    size_t sepPos = outputPath.find_last_of("/\\");
    if (dotPos != std::string::npos && (sepPos == std::string::npos || dotPos > sepPos)) {
        uniqueOutputPath = outputPath.substr(0, dotPos) + "_" + std::to_string(counter) +
                           outputPath.substr(dotPos);
    } else {
        uniqueOutputPath = outputPath + "_" + std::to_string(counter) + ".jpg";
    }
    if (!cv::imwrite(uniqueOutputPath, outputImage)) {
        std::cerr << "Failed to write output image to " << uniqueOutputPath << std::endl;
    } else {
        std::cout << "Output saved to " << uniqueOutputPath << std::endl;
    }
}
