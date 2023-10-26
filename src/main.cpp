#include <iostream>

#include "core/states.hpp"

#include <opencv2/opencv.hpp>
#include <torch/torch.h>
#include <iostream>

using namespace cv;

int main() {
    std::cout << "hellasdfasdfTarget torch_cpu not found.o" << std::endl;

    PreparationState state;

    state.tick();

  torch::Tensor tensor = torch::rand({2, 3});
  std::cout << tensor << std::endl;

      //create a gui window:
    // namedWindow("Output",1);
    
    //initialize a 120X350 matrix of black pixels:
    Mat output = Mat::zeros( 120, 350, CV_8UC3 );
    std::cout << output << std::endl;
    
   
    return 0;
}