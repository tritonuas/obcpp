#include <torch/script.h> // One-stop header.
#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include </workspaces/obcpp/include/cv/saliency.hpp>

/*
auto ToTensor(cv::Mat img, bool show_output = false, bool unsqueeze=false, int unsqueeze_dim = 0)
{
    std::cout << "image shape: " << img.size() << std::endl;
    at::Tensor tensor_image = torch::from_blob(img.data, { img.rows, img.cols, 3 }, at::kByte);

    if (unsqueeze)
    {
        tensor_image.unsqueeze_(unsqueeze_dim);
        std::cout << "tensors new shape: " << tensor_image.sizes() << std::endl;
    }
    
    if (show_output)
    {
        std::cout << tensor_image.slice(2, 0, 1) << std::endl;
    }
    std::cout << "tenor shape: " << tensor_image.sizes() << std::endl;
    return tensor_image;
}

auto transpose(at::Tensor tensor, c10::IntArrayRef dims = { 0, 3, 1, 2 })
{
    std::cout << "############### transpose ############" << std::endl;
    std::cout << "shape before : " << tensor.sizes() << std::endl;
    tensor = tensor.permute(dims);
    std::cout << "shape after : " << tensor.sizes() << std::endl;
    std::cout << "######################################" << std::endl;
    return tensor;
}

std::vector<torch::jit::IValue> ToInput(at::Tensor tensor_image)
{
    // Create a vector of inputs.
    std::vector<torch::jit::IValue> toReturn; //c10 List is also IValue
    c10::List<at::Tensor> tensorList;
    tensorList.push_back(tensor_image);       // add Tensor to c10::List
    toReturn.push_back(tensorList);           // add c10::List to vector of IValues
    return toReturn;
}
*/

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    std::cerr << "usage: example-app <path-to-exported-script-module>\n";
    return -1;
  }

  // convert image to tensor
  std::string msg = "sample image";
  auto currentPath = argv[1];
  cv::Mat img = cv::imread(currentPath);
  Saliency sal;
  sal.salience(img);

  // show_image(img, msg);

  // pass img into salience
  /*
  auto tensor = ToTensor(img);

  // convert the tensor into float and scale it 
  tensor = tensor.toType(c10::kFloat).div(255);
  
  // swap axis 
  tensor = transpose(tensor, { (2),(0),(1) });
  
  //add batch dim (an inplace operation just like in pytorch)
  // tensor.unsqueeze_(0);

  auto input_to_net = ToInput(tensor);

  torch::jit::script::Module module;
  try {
    // Deserialize the ScriptModule from a file using torch::jit::load().
    module = torch::jit::load("/workspaces/obcpp/models/default.pth"); // cannot accept .pth?
    module.eval();
  }
  catch (const c10::Error& e) {
    std::cerr << "error loading the model\n";
    std::cerr << e.msg() << std::endl; 
    return -1;
  }

  // making a prediction on the image using our model
  // at::Tensor output = module.forward(input_to_net).toTensor(); 
  auto output = module.forward(input_to_net); // vector of IValues which contains c10::List to IValue output
  c10::ivalue::Tuple& tuple = output.toTupleRef();
  auto detections = tuple.elements()[1].toList();
  std::cout << detections << "\n";

  // Is output IValue a tensor? No, it is a tuple.
  // links from 1/31:
  // https://pytorch.org/cppdocs/api/structtorch_1_1jit_1_1_module.html#exhale-struct-structtorch-1-1jit-1-1-module
  // https://pytorch.org/cppdocs/api/structc10_1_1_i_value.html
  // https://github.com/pytorch/pytorch/blob/main/aten/src/ATen/core/ivalue.h
  // link from 2/8
  // https://dev-discuss.pytorch.org/t/working-with-c10-ivalue-efficiently/585  
  // std::cout << output.isTensor() << "\n";
  std::cout << "ok\n";
  */
  
}