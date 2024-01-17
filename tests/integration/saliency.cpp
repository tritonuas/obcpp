#include <torch/script.h> // One-stop header.
#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>

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
    std::vector<torch::jit::IValue> toReturn;
    toReturn.push_back(tensor_image);
    return toReturn;
}

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    std::cerr << "usage: example-app <path-to-exported-script-module>\n";
    return -1;
  }

  // convert image to tensor
  std::string msg = "sample image";
  auto currentPath = argv[1];
  auto img = cv::imread(currentPath);
  // show_image(img, msg);

  auto tensor = ToTensor(img);

  // convert the tensor into float and scale it 
  tensor = tensor.toType(c10::kFloat).div(255);
  // swap axis 
  tensor = transpose(tensor, { (2),(0),(1) });
  //add batch dim (an inplace operation just like in pytorch)
  tensor.unsqueeze_(0);

  std::vector<torch::jit::IValue> input_to_net = ToInput(tensor);
  // auto input_to_net = ToInput(tensor);

  torch::jit::script::Module module;
  try {
    // Deserialize the ScriptModule from a file using torch::jit::load().
    module = torch::jit::load("/workspaces/obcpp/models/traced_resnet_model.pt"); // cannot accept .pth?
    // at::Tensor output = module.forward(input_to_net).toTensor(); 
    // std::cout << output.sizes() << std::endl;
    // std::cout << "output: " << output[0] << std::endl;
  }
  catch (const c10::Error& e) {
    std::cerr << "error loading the model\n";
    std::cerr << e.msg() << std::endl; 
    // REGARDING ERROR: see comments
    // https://discuss.pytorch.org/t/how-to-convert-an-opencv-image-into-libtorch-tensor/90818/4
    return -1;
  }

  // std::cout << input_to_net.size() << std::endl;
  // making a prediction on the image using our model
  at::Tensor output = module.forward(input_to_net).toTensor(); 
  std::cout << output.sizes() << std::endl;
  // std::cout << "output: " << output[0] << std::endl;

  std::cout << "ok\n";
  // torch::Tensor tensor = torch::randn({3, 5000, 3000});
  // std::vector<torch::jit::IValue> tensors;
  
  // tensors.push_back(input_to_net);
  // at::Tensor output = module.forward(input_to_net).toTensor(); 
  // std::cout << output.slice(/*dim=*/1, /*start=*/0, /*end=*/5) << '\n';
}