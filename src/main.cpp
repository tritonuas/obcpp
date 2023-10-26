#include <iostream>
#include "core/states.hpp"

#include <opencv2/opencv.hpp>
#include <torch/torch.h>
#include <iostream>

using namespace cv;

int main()
{
  std::cout << "hellasdfasdfTarget torch_cpu not found.o" << std::endl;

  PreparationState state;

  state.tick();

  torch::Tensor tensor = torch::rand({2, 3});
  std::cout << tensor << std::endl;

  Mat output = Mat::zeros(120, 350, CV_8UC3);

  // write text on the matrix:
  putText(output,
          "Hello World :)",
          Point(15, 70),
          FONT_HERSHEY_PLAIN,
          3,
          Scalar(0, 255, 0),
          4);

  imwrite("Output.png", output);

  return 0;
}