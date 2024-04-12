#include "cv/saliency.hpp"
#include <torch/script.h> // One-stop header.
#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include "cv/saliency.hpp"
#include <torch/cuda.h>
#include <torch/mps.h>
#include "utilities/logging.hpp"

// constructor definition
Saliency::Saliency () {
    this->modelPath = "";
} 

// constructor definition
Saliency::Saliency (std::string modelPath) {
    this->modelPath = modelPath;
} 

std::vector<CroppedTarget> Saliency::salience(cv::Mat image) { 
    
    at::Tensor tensor = Saliency::ToTensor(image, false, false, 0);

    // convert the tensor into float and scale it 
    tensor = tensor.toType(c10::kFloat).div(255);
    
    // swap axis 
    tensor = Saliency::transpose(tensor, { (2),(0),(1) });

    auto input_to_net = ToInput(tensor);

    c10::Device device = torch::cuda::is_available() ? torch::kCUDA : torch::kCPU;
    torch::jit::script::Module module;
    try {
        // Deserialize the ScriptModule from a file using torch::jit::load().
        module = torch::jit::load(modelPath, device);
        module.eval();
    }
    catch (const c10::Error& e) {
        std::cerr << "error loading the model\n";
        std::cerr << e.msg() << std::endl; 
        return {};
    }

    // note: add flag so that forward () runs using CUDA on the jetson
        
    auto output = module.forward(input_to_net); // vector of IValues which contains c10::List to IValue output
    c10::ivalue::Tuple& tuple = output.toTupleRef();
    auto detections = tuple.elements()[1];      // index 1: detections
    std::vector<CroppedTarget> targets;
    auto listDetections = detections.toList();

    for (c10::IValue results: listDetections) {
        c10::Dict<c10::IValue, c10::IValue> dict = results.toGenericDict();     // dict whose fields are boxes, labels, scores. 
                                                        // see: https://pytorch.org/vision/main/models/generated/torchvision.models.detection.fasterrcnn_resnet50_fpn.html#torchvision.models.detection.fasterrcnn_resnet50_fpn
        c10::IValue boxes = dict.at("boxes");           // https://discuss.pytorch.org/t/genericdict-c/178178/3
        c10::IValue labels = dict.at("labels");          
        at::Tensor boxTensor = boxes.toTensor();
        at::Tensor labelsTensor = labels.toTensor();
        // std::cout << boxTensor << std::endl;
        // std::cout << labelsTensor << std::endl;
        
        for (int i = 0; i < boxTensor.size(0); i++) { 
            CroppedTarget targ; 
            Bbox box;
            box.x1 = boxTensor[i][0].item().toInt();
            box.y1 = boxTensor[i][1].item().toInt();
            box.x2 = boxTensor[i][2].item().toInt();
            box.y2 = boxTensor[i][3].item().toInt();

            // label interpretation:
            // 1 if regular target
            // 2 if mannikin
            if (labelsTensor[0].item().toInt() == 2) {
                targ.isMannikin = 1;
            } else if (labelsTensor[0].item().toInt() == 1) {
                targ.isMannikin = 0;
            } else {
                LOG_F(ERROR, "Saliency::salience found invalid score");
            }

            cv::Rect roi; // setup region of interest (cropped target)
            roi.x = box.x1;
            roi.y = box.y1;
            roi.width = box.x2 - box.x1;
            roi.height = box.y2 - box.y1;
            cv::Mat cropdImg = image(roi);

            targ.bbox = box;        
            targ.croppedImage = cropdImg;
               
            targets.push_back(targ);
        }
    }

    

    LOG_F(INFO, "salience ok");
    return targets; 
}


at::Tensor Saliency::ToTensor(cv::Mat img, bool show_output = false, bool unsqueeze=false, int unsqueeze_dim = 0)
{
    // std::cout << "image shape: " << img.size() << std::endl;
    at::Tensor tensor_image = torch::from_blob(img.data, { img.rows, img.cols, 3 }, at::kByte);

    if (unsqueeze)
    {
        tensor_image.unsqueeze_(unsqueeze_dim);
        // std::cout << "tensors new shape: " << tensor_image.sizes() << std::endl;
    }
    
    if (show_output)
    {
        std::cout << tensor_image.slice(2, 0, 1) << std::endl;
    }
    // std::cout << "tenor shape: " << tensor_image.sizes() << std::endl;
    return tensor_image;
}

at::Tensor Saliency::transpose(at::Tensor tensor, c10::IntArrayRef dims = { 0, 3, 1, 2 })
{
    // std::cout << "############### transpose ############" << std::endl;
    // std::cout << "shape before : " << tensor.sizes() << std::endl;
    tensor = tensor.permute(dims);
    // std::cout << "shape after : " << tensor.sizes() << std::endl;
    // std::cout << "######################################" << std::endl;
    return tensor;
}

std::vector<torch::jit::IValue> Saliency::ToInput(at::Tensor tensor_image)
{
    // Create a vector of inputs.
    std::vector<torch::jit::IValue> toReturn; //c10 List is also IValue
    c10::List<at::Tensor> tensorList;
    tensorList.push_back(tensor_image);       // add Tensor to c10::List
    toReturn.push_back(tensorList);           // add c10::List to vector of IValues
    return toReturn;
}

