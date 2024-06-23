#include "cv/saliency.hpp"
#include <torch/script.h>   // One-stop header.
#include <torch/cuda.h>
#include <torch/mps.h>
#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>
#include "utilities/logging.hpp"


#define REGULAR_TARGET 1
#define MANNIKIN 2

// constructor definition
Saliency::Saliency(std::string modelPath) {
    this->modelPath = modelPath;

    // use cuda if it is available
    c10::Device device = torch::cuda::is_available() ? torch::kCUDA : torch::kCPU;
    torch::jit::script::Module module;
    try {
        // Deserialize the ScriptModule from a file using torch::jit::load().
        this->module = torch::jit::load(modelPath, device);
        this->module.eval();
    }
    catch (const c10::Error& e) {
        LOG_F(ERROR, "error loading the model %s", e.msg().c_str());
    }
}

std::vector<CroppedTarget> Saliency::salience(cv::Mat image) {
    at::Tensor tensor = Saliency::ToTensor(image, false, false, 0);
    // preparing our tensor
    // convert the tensor into float and scale it
    tensor = tensor.toType(c10::kFloat).div(255);
    // swap axis
    tensor = Saliency::transpose(tensor, { (2), (0), (1) });

    // eventually add device as member of Saliency
    c10::Device device = torch::cuda::is_available() ? torch::kCUDA : torch::kCPU;
    auto tensor_cuda = tensor.to(device);

    auto input_to_net = ToInput(tensor_cuda);

    /* 
     * forward() runs an inference on the input image using the provided model
     * and returns predictions as List[Dict[Tensor]], where the fields of Dict
     * that we want are : a) boxes (FloatTensor[N, 4]): the predicted boxes, and
     * b) scores (Tensor[N]): the scores of each detection.
     */

    // output is a tuple of (losses, detections)
    auto output = module.forward(input_to_net);
    c10::ivalue::Tuple& tuple = output.toTupleRef();
    // detections is our List[Dict[Tensor]], and index 1 contains the detections in the image
    auto detections = tuple.elements()[1];
    // since we only passed in one image, listDetections will only have one element
    auto listDetections = detections.toList();

    std::vector<CroppedTarget> targets;
    targets = extractTargets(listDetections, image);

    LOG_F(INFO, "saliency found %ld targets", targets.size());
    for (auto const& target : targets) {
        LOG_F(INFO, "\ttarget at bbox: (%d, %d, %d, %d). ismannikin: %d",
            target.bbox.x1,
            target.bbox.y1,
            target.bbox.x2,
            target.bbox.y2,
            target.isMannikin);
    }
    return targets;
}

// show_output flag can be used to print tensor_image.slice(2, 0, 1)
at::Tensor Saliency::ToTensor(cv::Mat img, bool show_output = false, bool unsqueeze = false,
                              int unsqueeze_dim = 0) {
    at::Tensor tensor_image = torch::from_blob(img.data, { img.rows, img.cols, 3 }, at::kByte);

    if (unsqueeze) {
        tensor_image.unsqueeze_(unsqueeze_dim);
    }

    return tensor_image;
}

at::Tensor Saliency::transpose(at::Tensor tensor, c10::IntArrayRef dims = { 0, 3, 1, 2 }) {
    tensor = tensor.permute(dims);
    return tensor;
}

std::vector<torch::jit::IValue> Saliency::ToInput(at::Tensor tensor_image) {
    // Create a vector of inputs.
    std::vector<torch::jit::IValue> toReturn;   // c10 List is also IValue
    c10::List<at::Tensor> tensorList;
    tensorList.push_back(tensor_image);         // add Tensor to c10::List
    toReturn.push_back(tensorList);             // add c10::List to vector of IValues
    return toReturn;
}

std::vector<CroppedTarget> Saliency::extractTargets(c10::List<c10::IValue> listDetections,
                                                    cv::Mat image) {
    std::vector<CroppedTarget> targetList;

    // looping through the number of images (we only have one image)
    for (c10::IValue results : listDetections) {
        /* 
         * Each c10::IValue in results is a dict whose fields are boxes, labels, scores.
         * we only need to extract boxes (bounding boxes) and labels (mannikin or not?).
         */
        c10::Dict<c10::IValue, c10::IValue> dict = results.toGenericDict();
        c10::IValue boxes = dict.at("boxes");
        c10::IValue labels = dict.at("labels");
        at::Tensor boxTensor = boxes.toTensor();
        at::Tensor labelsTensor = labels.toTensor();

        // looping through the number of bounding boxes in the image identified by the model
        for (int i = 0; i < boxTensor.size(0); i++) {
            /*
             * format of box: [x1, y1, x2, y2], with 0 <= x1 < x2 <= W and 0 <= y1 < y2 <= H
             */
            CroppedTarget target;
            Bbox box;
            box.x1 = boxTensor[i][0].item().toInt();
            box.y1 = boxTensor[i][1].item().toInt();
            box.x2 = boxTensor[i][2].item().toInt();
            box.y2 = boxTensor[i][3].item().toInt();
            /*
             * label interpretation:
             * 1 if regular target
             * 2 if mannikin
             */
            if (labelsTensor[0].item().toInt() == MANNIKIN) {
                target.isMannikin = 1;
            } else if (labelsTensor[0].item().toInt() == REGULAR_TARGET) {
                target.isMannikin = 0;
            } else {
                LOG_F(ERROR, "Saliency::salience found invalid score");
            }

            cv::Rect roi;   // setup region of interest (cropped target)
            roi.x = box.x1;
            roi.y = box.y1;
            roi.width = box.x2 - box.x1;
            roi.height = box.y2 - box.y1;
            cv::Mat cropdImg = image(roi);

            target.bbox = box;
            target.croppedImage = cropdImg;

            targetList.push_back(target);
        }
    }
    return targetList;
}
