#include "cv/segmentation.hpp"

#include <loguru.hpp>

Segmentation::Segmentation(const std::string &modelPath) {
            try {
                this->module = torch::jit::load(modelPath);
            }
            catch (const c10::Error& e) {
                LOG_F(ERROR, "could not load the model, check if model file is present at %s. If the file is present, try to verify that it's contents are valid model weights. Sometimes when pulling from google drive, an error html page will download with the same filename if the model fails to download. \n", modelPath.c_str());
                throw;
            }
        }

SegmentationResults Segmentation::segment(const CroppedTarget &target) {
    auto device = torch::kCPU;
    auto fcn = this->module;
    fcn.to(device);

    auto target_tensor = ToTensor(target.croppedImage, false, false, 0);
    target_tensor = target_tensor.clamp_max(c10::Scalar(50));
    target_tensor = target_tensor.toType(c10::kFloat).div(255);
    target_tensor = transpose(target_tensor, { (2), (0), (1) });
    target_tensor.unsqueeze_(0);

    auto input_to_net = ToInput(target_tensor);
    at::Tensor output = (fcn.forward(input_to_net).toGenericDict().begin()->value()).toTensor();
    torch::Tensor cpu_tensor = output.to(device);
    cpu_tensor = cpu_tensor[0];
    cpu_tensor = torch::softmax(cpu_tensor, 0);

    auto character_mask = cpu_tensor[1];
    auto shape_mask = cpu_tensor[0];
    cv::Mat character_mask_mat(character_mask.sizes()[0], character_mask.sizes()[1],
        CV_32FC1, character_mask.data_ptr<float>());
    cv::Mat shape_mask_mat(shape_mask.sizes()[0], shape_mask.sizes()[1],
        CV_32FC1, shape_mask.data_ptr<float>());

    SegmentationResults result;
    result.shapeMask = character_mask_mat;
    result.characterMask = shape_mask_mat;
    return result;
}



std::string get_image_type(const cv::Mat& img, bool more_info = true) {
    std::string r;
    int type = img.type();
    uchar depth = type & CV_MAT_DEPTH_MASK;
    uchar chans = 1 + (type >> CV_CN_SHIFT);

    switch (depth) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
    }

    r += "C";
    r += (chans + '0');

    if (more_info) {
        LOG_F(INFO, "depth: %d channels: %d\n", img.depth(), img.channels());
    }
    return r;
}

void show_image(cv::Mat& img, std::string title) {
    std::string image_type = get_image_type(img);
    cv::namedWindow(title + " type:" + image_type, cv::WINDOW_NORMAL);
    cv::imshow(title, img);
    cv::waitKey(0);
}

at::Tensor transpose(at::Tensor tensor, c10::IntArrayRef dims = { 0, 3, 1, 2 }) {
    tensor = tensor.permute(dims);
    return tensor;
}

at::Tensor ToTensor(cv::Mat img, bool show_output = false, bool unsqueeze = false,
int unsqueeze_dim = 0) {
    at::Tensor tensor_image = torch::from_blob(img.data, { img.rows, img.cols, 3 }, at::kByte);

    if (unsqueeze) {
        tensor_image.unsqueeze_(unsqueeze_dim);
    }

    return tensor_image;
}

std::vector<torch::jit::IValue> ToInput(at::Tensor tensor_image) {
    // Create a vector of inputs.
    return std::vector<torch::jit::IValue>{tensor_image};
}

cv::Mat ToCvImage(at::Tensor tensor) {
    int width = tensor.sizes()[0];
    int height = tensor.sizes()[1];
    try {
        cv::Mat output_mat(cv::Size{ height, width }, CV_8UC3, tensor.data_ptr<uchar>());

        show_image(output_mat, "converted image from tensor");
        return output_mat.clone();
    }
    catch (const c10::Error& e) {
        LOG_S(ERROR) << "error loading the model: : " << e.msg() << "\n";
    }
    return cv::Mat(height, width, CV_8UC3);
}
