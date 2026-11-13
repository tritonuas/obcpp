#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "cv/utilities.hpp"

namespace fs = std::filesystem;

struct BoundingBox {
    int x;
    int y;
    int width;
    int height;
};

struct BenchmarkResult {
    std::string name;
    cv::Mat image;
    std::vector<uchar> encoded;
    double elapsedMs;
};

cv::Mat encodeAndDecode(const cv::Mat& image, int quality,
                        std::vector<uchar>& encoded) {
    const std::vector<int> compressionParams{cv::IMWRITE_JPEG_QUALITY, quality};
    if (!cv::imencode(".jpg", image, encoded, compressionParams)) {
        throw std::runtime_error("Unable to encode an image as JPEG");
    }

    cv::Mat decoded = cv::imdecode(encoded, cv::IMREAD_COLOR);
    if (decoded.empty()) {
        throw std::runtime_error("Unable to decode a JPEG image");
    }
    return decoded;
}

BenchmarkResult runCurrentCompression(const cv::Mat& source, int quality) {
    const auto start = std::chrono::steady_clock::now();
    std::optional<cv::Mat> compressed = compressImg(source, quality);
    if (!compressed.has_value()) {
        throw std::runtime_error("compressImg failed");
    }

    std::vector<uchar> encoded;
    if (!cv::imencode(".jpg", compressed.value(), encoded,
                      {cv::IMWRITE_JPEG_QUALITY, quality})) {
        throw std::runtime_error("Unable to encode the compressImg result");
    }
    cv::Mat decoded = cv::imdecode(encoded, cv::IMREAD_COLOR);
    if (decoded.empty()) {
        throw std::runtime_error("Unable to decode the compressImg result");
    }
    const auto end = std::chrono::steady_clock::now();
    const double elapsedMs =
        std::chrono::duration<double, std::milli>(end - start).count();
    return {"current_compressImg", std::move(decoded), std::move(encoded),
            elapsedMs};
}

BenchmarkResult runJpegCompression(const cv::Mat& source, int quality,
                                   const std::string& name) {
    const auto start = std::chrono::steady_clock::now();
    std::vector<uchar> encoded;
    cv::Mat decoded = encodeAndDecode(source, quality, encoded);
    const auto end = std::chrono::steady_clock::now();
    const double elapsedMs =
        std::chrono::duration<double, std::milli>(end - start).count();
    return {name, std::move(decoded), std::move(encoded), elapsedMs};
}

BenchmarkResult runResizedJpegCompression(const cv::Mat& source, int quality,
                                         double scale,
                                         const std::string& name) {
    const auto start = std::chrono::steady_clock::now();
    cv::Mat resized;
    cv::resize(source, resized, cv::Size(), scale, scale, cv::INTER_AREA);

    std::vector<uchar> encoded;
    cv::Mat decoded = encodeAndDecode(resized, quality, encoded);
    const auto end = std::chrono::steady_clock::now();
    const double elapsedMs =
        std::chrono::duration<double, std::milli>(end - start).count();
    return {name, std::move(decoded), std::move(encoded), elapsedMs};
}

cv::Rect validateBoundingBox(const BoundingBox& box, const cv::Mat& image) {
    if (box.x < 0 || box.y < 0 || box.width <= 0 || box.height <= 0 ||
        box.x > image.cols - box.width || box.y > image.rows - box.height) {
        throw std::invalid_argument(
            "Bounding box must be positive and fit within the input image");
    }
    return {box.x, box.y, box.width, box.height};
}

cv::Rect scaleBoundingBox(const cv::Rect& box, const cv::Size& sourceSize,
                          const cv::Size& resultSize) {
    const double scaleX =
        static_cast<double>(resultSize.width) / sourceSize.width;
    const double scaleY =
        static_cast<double>(resultSize.height) / sourceSize.height;
    const int x1 = static_cast<int>(std::floor(box.x * scaleX));
    const int y1 = static_cast<int>(std::floor(box.y * scaleY));
    const int x2 =
        static_cast<int>(std::ceil((box.x + box.width) * scaleX));
    const int y2 =
        static_cast<int>(std::ceil((box.y + box.height) * scaleY));
    const int clippedX2 = std::min(x2, resultSize.width);
    const int clippedY2 = std::min(y2, resultSize.height);
    return {x1, y1, clippedX2 - x1, clippedY2 - y1};
}

void writeFile(const fs::path& path, const std::vector<uchar>& contents) {
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("Unable to open output file: " + path.string());
    }
    output.write(reinterpret_cast<const char*>(contents.data()),
                 static_cast<std::streamsize>(contents.size()));
    if (!output) {
        throw std::runtime_error("Unable to write output file: " + path.string());
    }
}

void writePreview(const fs::path& path, const cv::Mat& image) {
    if (!cv::imwrite(path.string(), image)) {
        throw std::runtime_error("Unable to write preview image: " + path.string());
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2 && argc != 3 && argc != 7) {
        std::cerr << "Usage: " << argv[0]
                  << " <input-image> [output-dir]"
                     " [bbox-x bbox-y bbox-width bbox-height]\n";
        return 2;
    }

    try {
        const fs::path inputPath = argv[1];
        const fs::path outputDir =
            argc >= 3 ? fs::path(argv[2]) : fs::path("compression_benchmark_output");
        cv::Mat source = cv::imread(inputPath.string(), cv::IMREAD_COLOR);
        if (source.empty()) {
            throw std::runtime_error("Unable to read input image: " +
                                     inputPath.string());
        }

        BoundingBox box;
        if (argc == 7) {
            box = {std::stoi(argv[3]), std::stoi(argv[4]), std::stoi(argv[5]),
                   std::stoi(argv[6])};
        } else {
            box.width = std::min(128, source.cols);
            box.height = std::min(128, source.rows);
            box.x = (source.cols - box.width) / 2;
            box.y = (source.rows - box.height) / 2;
            std::cout << "No bounding box supplied; examining the centered "
                      << box.width << "x" << box.height << " region.\n";
        }
        const cv::Rect region = validateBoundingBox(box, source);

        fs::create_directories(outputDir);
        writePreview(outputDir / "original_bbox.png", source(region));

        constexpr int kQuality = 60;
        std::vector<BenchmarkResult> results;
        results.push_back(runCurrentCompression(source, kQuality));
        results.push_back(
            runJpegCompression(source, kQuality, "jpeg_q60"));
        results.push_back(
            runJpegCompression(source, 80, "jpeg_q80"));
        results.push_back(runResizedJpegCompression(
            source, kQuality, 0.75, "jpeg_75pct_q60"));

        std::cout << "Image: " << source.cols << "x" << source.rows
                  << "\nResults (timing includes image processing and JPEG "
                     "encode/decode):\n"
                  << std::left << std::setw(24) << "algorithm" << std::right
                  << std::setw(12) << "time (ms)" << std::setw(14) << "bytes"
                  << std::setw(14) << "dimensions" << '\n';

        for (const BenchmarkResult& result : results) {
            const std::string baseName = result.name;
            writeFile(outputDir / (baseName + ".jpg"), result.encoded);
            const cv::Rect resultRegion =
                scaleBoundingBox(region, source.size(), result.image.size());
            writePreview(outputDir / (baseName + "_bbox.png"),
                         result.image(resultRegion));

            std::cout << std::left << std::setw(24) << result.name
                      << std::right << std::setw(12) << std::fixed
                      << std::setprecision(2) << result.elapsedMs
                      << std::setw(14) << result.encoded.size()
                      << std::setw(7) << result.image.cols << "x"
                      << result.image.rows << '\n';
        }
        std::cout << "Full JPEGs and bounding-box previews saved in "
                  << outputDir << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Compression benchmark failed: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
