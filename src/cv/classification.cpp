#include "cv/classification.hpp"

Classification::Classification(const std::string &shapeModelPath, \
const std::string &characterModelPath, const std::string &colorModelPath) {
    auto device = torch::kCPU;
    if (torch::cuda::is_available()) {
        device = torch::kCUDA;
    }

    try {
        this->shapeModel = torch::jit::load(shapeModelPath);
        this->shapeModel.to(device);
    }
    catch (const c10::Error& e) {
        std::cerr << "ERROR: could not load the shape model\n";
        throw;
    }

    try {
        this->characterModel = torch::jit::load(characterModelPath);
        this->characterModel.to(device);
    }
    catch (const c10::Error& e) {
        std::cerr << "ERROR: could not load the character model\n";
        throw;
    }

    this->index_to_shape = {
        { 1, "CIRCLE"},
        { 2, "SEMICIRCLE"},
        { 3, "QUARTER_CIRCLE"},
        { 4, "TRIANGLE"},
        { 5, "SQUARE"},
        { 6, "RECTANGLE"},
        { 7, "TRAPEZOID"},
        { 8, "PENTAGON"},
        { 9, "HEXAGON"},
        { 10, "HEPTAGON"},
        { 11, "OCTAGON"},
        { 12, "STAR"},
        { 13, "CROSS"}
    };

    /* We are not using color model for now */
    // try {
    //     this->colorModel = torch::jit::load(colorModelPath);
    //     this->colorModel.to(device);
    // }
    // catch (const c10::Error& e) {
    //     std::cerr << "ERROR: could not load the color model\n";
    //     throw;
    // }
}

std::string Classification::classifyShape(cv::Mat croppedImage, cv::Mat shapeMask) {
    // Prepare cropped image tensor
    at::Tensor img_tensor = ToTensor(croppedImage, false, false, 0);
    img_tensor = img_tensor.toType(c10::kFloat).div(255);
    img_tensor = transpose(img_tensor, { (2), (0), (1) });

    // Prepare mask tensor
    at::Tensor mask_tensor = torch::from_blob(shapeMask.data, \
    { shapeMask.rows, shapeMask.cols}, at::kFloat);
    mask_tensor = mask_tensor.expand({3, mask_tensor.sizes()[0], mask_tensor.sizes()[1]});

    // multiply mask tensor with cropped image tensor
    mask_tensor = mask_tensor*img_tensor;
    mask_tensor.unsqueeze_(0);

    at::Tensor output = (this->shapeModel.forward(ToInput(mask_tensor))).toTensor();
    auto index = torch::argmax(output).item<int>();
    return this->index_to_shape[index + 1];
}

std::string Classification::classifyCharacter(cv::Mat croppedImage, cv::Mat characterMask) {
    // Prepare cropped image tensor
    at::Tensor img_tensor = ToTensor(croppedImage, false, false, 0);
    img_tensor = img_tensor.toType(c10::kFloat).div(255);
    img_tensor = transpose(img_tensor, { (2), (0), (1) });

    // Prepare mask tensor
    at::Tensor mask_tensor = torch::from_blob(characterMask.data, \
    { characterMask.rows, characterMask.cols}, at::kFloat);
    mask_tensor = mask_tensor.expand({3, mask_tensor.sizes()[0], mask_tensor.sizes()[1]});

    // multiply mask tensor with cropped image tensor
    mask_tensor = mask_tensor*img_tensor;
    mask_tensor.unsqueeze_(0);

    at::Tensor output = (this->characterModel.forward(ToInput(mask_tensor))).toTensor();
    auto index = torch::argmax(output).item<int>();
    if (index < 26) {
        return std::string(1, 'A' + index);
    } else {
        return std::to_string(index - 26);
    }
}

std::string Classification::classifyColor(cv::Mat croppedImage, cv::Mat mask) {
    return nullptr;
}

ClassificationResults Classification::classify(cv::Mat croppedImage, cv::Mat shapeMask, \
cv::Mat characterMask) {
    ClassificationResults results;
    results.character = this->classifyCharacter(croppedImage, characterMask);
    results.shape = this->classifyShape(croppedImage, shapeMask);
    return results;
}
